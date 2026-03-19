#pragma once

#include <CLI/CLI.hpp>
#include "services/SnippetService.hpp"

namespace snip {

class SearchCommand {
public:
    SearchCommand()  = delete;
    ~SearchCommand() = delete;

    static void registerTo(CLI::App& app, SnippetService& service);
};

} // namespace snip
