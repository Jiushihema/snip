#pragma once

#include <CLI/CLI.hpp>
#include "services/SnippetService.hpp"

namespace snip {

class CopyCommand {
public:
    CopyCommand()  = delete;
    ~CopyCommand() = delete;

    static void registerTo(CLI::App& app, SnippetService& service);
};

} // namespace snip
