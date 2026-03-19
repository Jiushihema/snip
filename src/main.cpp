#include "display/Renderer.hpp"
#include <iostream>
#include <memory>

#include <string>
#include <filesystem>
#include <cstdlib>


#include <fstream>


// 第三方库
#include <CLI/CLI.hpp>


// 接口层
#include "interfaces/IStore.hpp"
#include "interfaces/ISearchEngine.hpp"
#include "interfaces/IClipboard.hpp"

// 基础设施层
#include "infra/SqliteStore.hpp"
#include "infra/LocalSearch.hpp"
#include "infra/LLMSearch.hpp"
#include "infra/Clipboard.hpp"

// 业务逻辑层
#include "services/SnippetService.hpp"

// 表现层
#include "display/Renderer.hpp"
#include "commands/AddCommand.hpp"
#include "commands/ListCommand.hpp"
#include "commands/SearchCommand.hpp"
#include "commands/CopyCommand.hpp"
#include "commands/DeleteCommand.hpp"
#include "commands/FindCommand.hpp"   // [新增]

namespace {

// ─────────────────────────────────────────────
// 路径工具
// ─────────────────────────────────────────────

/// 返回配置文件路径：~/.config/snip/config.toml
std::filesystem::path configPath() {
    const char* home = std::getenv("HOME");
#if defined(_WIN32) || defined(_WIN64)
    if (!home) home = std::getenv("USERPROFILE");
#endif
    if (!home) {
        throw std::runtime_error(
            "Cannot determine home directory: $HOME is not set.");
    }
    return std::filesystem::path(home) / ".config" / "snip" / "config.toml";
}

/// 返回数据库文件路径：~/.local/share/snip/snip.db
std::filesystem::path dbPath() {
    const char* home = std::getenv("HOME");
#if defined(_WIN32) || defined(_WIN64)
    if (!home) home = std::getenv("USERPROFILE");
#endif
    if (!home) {
        throw std::runtime_error(
            "Cannot determine home directory: $HOME is not set.");
    }
    return std::filesystem::path(home) / ".local" / "share" / "snip" / "snip.db";
}

// ─────────────────────────────────────────────
// 配置结构体（带默认值）
// ─────────────────────────────────────────────

struct Config {
    // [search]
    std::string engine   = "local";

    // [llm]
    std::string provider = "openai";
    std::string api_key;
    std::string model    = "gpt-4o";
    std::string endpoint = "https://api.openai.com/v1/chat/completions";
};

/// 从 TOML 文件加载配置；文件不存在时静默返回默认值
Config loadConfig(const std::filesystem::path& path) {
    Config cfg; // 使用默认值
    
    std::ifstream file(path);
    if (!file.is_open()) return cfg; // 文件不存在就用默认值
    
    std::string line;
    std::string section;
    while (std::getline(file, line)) {
        // 去掉首尾空格
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        // 跳过注释和空行
        if (line.empty() || line[0] == '#') continue;
        
        // 解析 section [xxx]
        if (line[0] == '[') {
            section = line.substr(1, line.find(']') - 1);
            continue;
        }
        
        // 解析 key = value
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        // 去掉首尾空格和引号
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        val.erase(0, val.find_first_not_of(" \t\""));
        val.erase(val.find_last_not_of(" \t\"") + 1);
        
        if (section == "search" && key == "engine")   cfg.engine   = val;
        if (section == "llm"    && key == "provider") cfg.provider = val;
        if (section == "llm"    && key == "api_key")  cfg.api_key  = val;
        if (section == "llm"    && key == "model")    cfg.model    = val;
        if (section == "llm"    && key == "endpoint") cfg.endpoint = val;
    }
    return cfg;
}


// ─────────────────────────────────────────────
// 搜索引擎工厂
// ─────────────────────────────────────────────

std::shared_ptr<snip::ISearchEngine> buildSearchEngine(const Config& cfg) {
    using snip::Renderer;
    if (cfg.engine == "llm") {
        if (cfg.api_key.empty()) {
            Renderer::printWarning(
                "LLM search selected but api_key is empty. "
                "Set it in config.toml or via $SNIP_API_KEY.");
        }
        return std::make_shared<snip::LLMSearch>(
            cfg.endpoint, cfg.api_key, cfg.model);
    }

    // 默认 / "local"
    if (cfg.engine != "local") {
        Renderer::printWarning(
            "Unknown search engine \"" + cfg.engine +
            "\", falling back to local search.");
    }
    return std::make_shared<snip::LocalSearch>();
}

} // anonymous namespace

// ─────────────────────────────────────────────
// main
// ─────────────────────────────────────────────

int main(int argc, char* argv[]) {
    try {
        // ── 1. 加载配置 ──────────────────────────────
        const auto cfgFile = configPath();
        const Config cfg   = loadConfig(cfgFile);

        // ── 2. 构建基础设施层（依赖注入） ────────────
        auto store = std::make_shared<snip::SqliteStore>("");

        auto searchEngine  = buildSearchEngine(cfg);
        auto clipboard     = std::make_shared<snip::Clipboard>();

        // ── 3. 构建业务逻辑层 ────────────────────────
        snip::SnippetService service(store, searchEngine, clipboard);

        // ── 4. 初始化 CLI ────────────────────────────
        CLI::App app{"snip — command-line code snippet manager"};
        app.require_subcommand(1);   // 必须提供一个子命令

        // ── 5. 注册所有子命令 ────────────────────────
        snip::AddCommand::registerTo(app, service);
        snip::ListCommand::registerTo(app, service);
        snip::SearchCommand::registerTo(app, service);
        snip::CopyCommand::registerTo(app, service);
        snip::DeleteCommand::registerTo(app, service);
        snip::FindCommand::registerTo(app, service, clipboard);  // [新增]


        // ── 6. 解析命令行参数 ────────────────────────
        CLI11_PARSE(app, argc, argv);

    } catch (const CLI::ParseError& e) {
        // CLI11 解析错误（--help 也会走此路径，exit code 为 0）
        // CLI11_PARSE 宏已处理此分支，此处作为保险兜底
        return 1;
    } catch (const std::exception& e) {
        snip::Renderer::printError(std::string("Fatal error: ") + e.what());
        return 1;
    } catch (...) {
        snip::Renderer::printError("Fatal error: unknown exception.");
        return 1;
    }

    return 0;
}
