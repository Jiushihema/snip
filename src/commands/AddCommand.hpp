#pragma once

#include <CLI/CLI.hpp>
#include "services/SnippetService.hpp"

namespace snip {

class AddCommand {
public:
    AddCommand()  = delete;
    ~AddCommand() = delete;

    /// 向 CLI11 app 注册 "add" 子命令
    static void registerTo(CLI::App& app, SnippetService& service);
};

} // namespace snip
