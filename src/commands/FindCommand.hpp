#pragma once

#include <memory>

#include <CLI/CLI.hpp>

#include "interfaces/IClipboard.hpp"
#include "services/SnippetService.hpp"

namespace snip {

/// "find" 子命令
/// 无 -q 参数时启动交互式 TUI（InteractiveSearch）
/// 携带 -q 参数时跳过 TUI，直接输出过滤结果（脚本友好模式）
class FindCommand {
public:
    FindCommand()  = delete;
    ~FindCommand() = delete;

    /// 向 CLI11 app 注册 "find" 子命令
    /// @param app        CLI11 根应用对象
    /// @param service    业务逻辑层服务
    /// @param clipboard  剪贴板接口（透传给 InteractiveSearch）
    static void registerTo(
        CLI::App&                    app,
        SnippetService&              service,
        std::shared_ptr<IClipboard>  clipboard
    );
};

} // namespace snip
