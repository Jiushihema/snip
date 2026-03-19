#pragma once

#include <CLI/CLI.hpp>
#include "services/SnippetService.hpp"

namespace snip {

class ListCommand {
public:
    ListCommand()  = delete;
    ~ListCommand() = delete;

    static void registerTo(CLI::App& app, SnippetService& service);
};

} // namespace snip
