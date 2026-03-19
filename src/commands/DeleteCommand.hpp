#pragma once

#include <CLI/CLI.hpp>
#include "services/SnippetService.hpp"

namespace snip {

class DeleteCommand {
public:
    DeleteCommand()  = delete;
    ~DeleteCommand() = delete;

    static void registerTo(CLI::App& app, SnippetService& service);
};

} // namespace snip
