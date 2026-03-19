#ifdef _WIN32
    #include <curses.h>
#else
    #include <ncurses.h>
#endif

#include "commands/FindCommand.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "display/Renderer.hpp"
#include "models/Snippet.hpp"
#include "ui/InteractiveSearch.hpp"

namespace snip {

void FindCommand::registerTo(
    CLI::App&                   app,
    SnippetService&             service,
    std::shared_ptr<IClipboard> clipboard)
{
    auto* sub = app.add_subcommand(
        "find",
        "Interactively search snippets (TUI). "
        "Use -q for non-interactive / script-friendly output.");

    // -q/--query 是可选参数；未提供时为空字符串，触发 TUI 模式
    auto* query_opt = new std::string();

    sub->add_option("-q,--query", *query_opt,
                    "Search keyword. If provided, skip TUI and print results directly.")
       ->default_val("");

    sub->callback([&service, clipboard, query_opt]() {
        const std::string query = *query_opt;
        delete query_opt;

        // ── 脚本模式：-q 有值，跳过 TUI ──────────────────────────────────
        if (!query.empty()) {
            try {
                const std::vector<Snippet> results = service.search(query);

                if (results.empty()) {
                    Renderer::printWarning(
                        "No snippets matched \"" + query + "\".");
                } else {
                    Renderer::printSnippetList(results);
                }
            } catch (const std::exception& e) {
                Renderer::printError(
                    std::string("Search failed: ") + e.what());
            }
            return;
        }

        // ── 交互模式：启动 TUI ────────────────────────────────────────────
        try {
            std::vector<Snippet> corpus = service.listAll();

            if (corpus.empty()) {
                Renderer::printWarning(
                    "No snippets found. Add some with `snip add` first.");
                return;
            }

            InteractiveSearch tui(std::move(corpus), clipboard);
            tui.run();

            // run() 返回后终端已由 endwin() 恢复，可正常输出
            Renderer::printSuccess("Session ended.");

        } catch (const std::exception& e) {
            // endwin() 可能尚未被调用（initscr 失败时），
            // 此处确保终端状态恢复后再输出错误
            endwin();
            Renderer::printError(
                std::string("Interactive search failed: ") + e.what());
        }
    });
}

} // namespace snip
