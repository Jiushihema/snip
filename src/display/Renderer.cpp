#include "display/Renderer.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <termcolor/termcolor.hpp>

namespace snip {

// ─────────────────────────────────────────────
// 私有工具
// ─────────────────────────────────────────────

std::string Renderer::truncate(const std::string& s, std::size_t width) {
    if (s.size() <= width) return s;
    // 留一个字符放省略号
    return s.substr(0, width - 1) + "\xe2\x80\xa6"; // UTF-8 "…"
}

std::string Renderer::cell(const std::string& s, std::size_t width) {
    const std::string t = truncate(s, width);
    if (t.size() >= width) return t;
    return t + std::string(width - t.size(), ' ');
}

// ─────────────────────────────────────────────
// 表格输出
// ─────────────────────────────────────────────

void Renderer::printSnippetList(const std::vector<Snippet>& snippets) {
    if (snippets.empty()) {
        std::cout << termcolor::yellow
                  << "  (no snippets found)\n"
                  << termcolor::reset;
        return;
    }

    // 列宽定义
    constexpr std::size_t W_ID    =  4;
    constexpr std::size_t W_TITLE = 28;
    constexpr std::size_t W_LANG  = 12;
    constexpr std::size_t W_TAGS  = 22;
    constexpr std::size_t W_TIME  = 19;

    // 分隔线
    const std::string sep =
        std::string(W_ID + W_TITLE + W_LANG + W_TAGS + W_TIME + 14, '-');

    // 表头
    std::cout << termcolor::bold << termcolor::cyan
              << "  "
              << cell("ID",      W_ID)    << "  "
              << cell("Title",   W_TITLE) << "  "
              << cell("Lang",    W_LANG)  << "  "
              << cell("Tags",    W_TAGS)  << "  "
              << cell("Created", W_TIME)  << "\n"
              << termcolor::reset;

    std::cout << termcolor::dark << "  " << sep << "\n" << termcolor::reset;

    // 数据行
    for (const Snippet& s : snippets) {
        std::cout
            << "  "
            << termcolor::bold << termcolor::white
            << cell(std::to_string(s.id), W_ID)
            << termcolor::reset << "  "

            << termcolor::white
            << cell(s.title, W_TITLE)
            << termcolor::reset << "  "

            << termcolor::green
            << cell(s.language, W_LANG)
            << termcolor::reset << "  "

            << termcolor::cyan
            << cell(s.tags, W_TAGS)
            << termcolor::reset << "  "

            << termcolor::dark
            << cell(s.created_at, W_TIME)
            << termcolor::reset << "\n";
    }

    std::cout << "\n";
}

// ─────────────────────────────────────────────
// 单片段详情
// ─────────────────────────────────────────────

void Renderer::printSnippet(const Snippet& s) {
    const std::string divider(60, '-');


    std::cout << "\n"
              << termcolor::bold << termcolor::cyan
              << "  " << s.title << "\n"
              << termcolor::reset;

    std::cout << termcolor::dark << "  " << divider << "\n" << termcolor::reset;

    // 元信息行
    std::cout
        << "  "
        << termcolor::yellow << "ID: "       << termcolor::reset
        << s.id << "   "
        << termcolor::yellow << "Language: " << termcolor::reset
        << (s.language.empty() ? "(none)" : s.language) << "   "
        << termcolor::yellow << "Tags: "     << termcolor::reset
        << (s.tags.empty()    ? "(none)" : s.tags) << "\n"
        << "  "
        << termcolor::yellow << "Created: "  << termcolor::reset
        << s.created_at << "\n\n";

    // 代码块
    std::cout << termcolor::dark << "  " << divider << "\n" << termcolor::reset;

    // 逐行输出 content，每行缩进两格
    std::istringstream stream(s.content);
    std::string line;
    while (std::getline(stream, line)) {
        std::cout << "  " << line << "\n";
    }

    std::cout << termcolor::dark << "  " << divider << "\n" << termcolor::reset
              << "\n";
}

// ─────────────────────────────────────────────
// 状态消息
// ─────────────────────────────────────────────

void Renderer::printSuccess(const std::string& msg) {
    std::cout << termcolor::bold << termcolor::green
              << "✔ " << msg
              << termcolor::reset << "\n";
}

void Renderer::printError(const std::string& msg) {
    std::cerr << termcolor::bold << termcolor::red
              << "✘ " << msg
              << termcolor::reset << "\n";
}

void Renderer::printWarning(const std::string& msg) {
    std::cout << termcolor::bold << termcolor::yellow
              << "⚠ " << msg
              << termcolor::reset << "\n";
}

} // namespace snip
