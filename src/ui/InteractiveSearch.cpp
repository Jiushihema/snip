#include "ui/InteractiveSearch.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include <stdexcept>

// ── 平台适配：PDCurses (Windows) vs ncurses (POSIX) ──────────────────────────
#ifdef _WIN32
#  include <curses.h>
#else
#  include <ncurses.h>
#endif

namespace snip {

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

InteractiveSearch::InteractiveSearch(
    std::vector<Snippet>        corpus,
    std::shared_ptr<IClipboard> clipboard)
    : corpus_(std::move(corpus))
    , clipboard_(std::move(clipboard))
{
    // 初始时显示全部片段
    filtered_ = corpus_;
}

// ─────────────────────────────────────────────────────────────────────────────
// 静态工具
// ─────────────────────────────────────────────────────────────────────────────

std::string InteractiveSearch::toLower(const std::string& s) {
    std::string r;
    r.reserve(s.size());
    for (const unsigned char c : s) {
        r.push_back(static_cast<char>(std::tolower(c)));
    }
    return r;
}

std::string InteractiveSearch::truncate(const std::string& s, int maxLen) {
    if (maxLen <= 0) return {};
    if (static_cast<int>(s.size()) <= maxLen) return s;
    if (maxLen <= 1) return ".";
    return s.substr(0, static_cast<std::size_t>(maxLen - 1)) + "\xe2\x80\xa6"; // "…"
}

// ─────────────────────────────────────────────────────────────────────────────
// filterSnippets
// ─────────────────────────────────────────────────────────────────────────────

void InteractiveSearch::filterSnippets() {
    filtered_.clear();

    if (query_.empty()) {
        filtered_ = corpus_;
    } else {
        const std::string lq = toLower(query_);
        for (const Snippet& s : corpus_) {
            if (toLower(s.title).find(lq)   != std::string::npos ||
                toLower(s.content).find(lq) != std::string::npos ||
                toLower(s.tags).find(lq)    != std::string::npos)
            {
                filtered_.push_back(s);
            }
        }
    }

    // 重置导航状态
    selectedIndex_ = 0;
    scrollOffset_  = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// 渲染辅助
// ─────────────────────────────────────────────────────────────────────────────

void InteractiveSearch::printWithHighlight(
    int row, int& col,
    const std::string& text,
    const std::string& lowerQuery,
    int maxCols)
{
    if (lowerQuery.empty()) {
        // 无关键词：直接输出
        const std::string t = truncate(text, maxCols - col);
        mvprintw(row, col, "%s", t.c_str());
        col += static_cast<int>(t.size());
        return;
    }

    const std::string lowerText = toLower(text);
    const int         qLen      = static_cast<int>(lowerQuery.size());
    int               pos       = 0;
    const int         tLen      = static_cast<int>(text.size());

    while (pos < tLen && col < maxCols) {
        const std::size_t found = lowerText.find(lowerQuery,
                                                  static_cast<std::size_t>(pos));

        if (found == std::string::npos) {
            // 剩余部分无匹配，普通输出
            const int remaining = std::min(tLen - pos, maxCols - col);
            mvprintw(row, col, "%.*s", remaining, text.c_str() + pos);
            col += remaining;
            break;
        }

        // 匹配前的普通文本
        const int plainLen = std::min(
            static_cast<int>(found) - pos,
            maxCols - col);
        if (plainLen > 0) {
            mvprintw(row, col, "%.*s", plainLen, text.c_str() + pos);
            col += plainLen;
        }

        // 高亮匹配段（A_BOLD）
        if (col < maxCols) {
            const int hlLen = std::min(qLen, maxCols - col);
            attron(A_BOLD);
            mvprintw(row, col, "%.*s", hlLen, text.c_str() + found);
            attroff(A_BOLD);
            col += hlLen;
        }

        pos = static_cast<int>(found) + qLen;
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void InteractiveSearch::drawSearchBar(int row, int cols) {
    // 清行
    move(row, 0);
    clrtoeol();

    // 提示符
    attron(A_BOLD);
    mvprintw(row, 0, "> ");
    attroff(A_BOLD);

    // 查询字符串（最多填满剩余宽度，留 1 列给光标）
    const int maxQueryDisplay = cols - 3;
    const std::string display =
        maxQueryDisplay > 0
            ? truncate(query_, maxQueryDisplay)
            : std::string{};

    mvprintw(row, 2, "%s", display.c_str());

    // 光标位置
    const int cursorCol = 2 + static_cast<int>(display.size());
    if (cursorCol < cols) {
        mvprintw(row, cursorCol, "_");
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void InteractiveSearch::drawResultList(int startRow, int endRow, int cols) {
    const int visibleLines = endRow - startRow;
    const int total        = static_cast<int>(filtered_.size());

    for (int i = 0; i < visibleLines; ++i) {
        const int row      = startRow + i;
        const int dataIdx  = scrollOffset_ + i;

        move(row, 0);
        clrtoeol();

        if (dataIdx >= total) continue; // 超出数据范围，留空行

        const Snippet& s      = filtered_[static_cast<std::size_t>(dataIdx)];
        const bool     isSelected = (dataIdx == selectedIndex_);
        const std::string lq  = toLower(query_);

        if (isSelected) attron(A_REVERSE);

        // ── 列布局（固定宽度）────────────────────────────────────────────
        // [ID:4] [space:1] [Title:28] [space:1] [Lang:10] [space:1] [Tags:rest]
        constexpr int W_ID   =  4;
        constexpr int W_TITL = 28;
        constexpr int W_LANG = 10;
        const     int W_TAGS = std::max(0, cols - W_ID - W_TITL - W_LANG - 4);

        int col = 0;

        // ID
        const std::string idStr = truncate(std::to_string(s.id), W_ID);
        mvprintw(row, col, "%-*s", W_ID, idStr.c_str());
        col += W_ID + 1;

        // Title（含关键词高亮，选中行反色时高亮效果叠加）
        if (!isSelected) {
            printWithHighlight(row, col,
                               truncate(s.title, W_TITL), lq, col + W_TITL);
            // 补齐空格
            while (col < W_ID + 1 + W_TITL) {
                mvprintw(row, col++, " ");
            }
        } else {
            // 选中行：反色下直接输出，不额外加粗（避免视觉混乱）
            mvprintw(row, col, "%-*s", W_TITL,
                     truncate(s.title, W_TITL).c_str());
            col += W_TITL;
        }
        col++; // 列间距

        // Language
        mvprintw(row, col, "%-*s", W_LANG,
                 truncate(s.language, W_LANG).c_str());
        col += W_LANG + 1;

        // Tags
        if (W_TAGS > 0) {
            mvprintw(row, col, "%-*s", W_TAGS,
                     truncate(s.tags, W_TAGS).c_str());
        }

        if (isSelected) attroff(A_REVERSE);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void InteractiveSearch::drawStatusBar(int row, int cols) {
    move(row, 0);
    clrtoeol();

    attron(A_REVERSE);

    const std::string hint =
        " ^Enter: copy & quit   ESC: quit   ↑↓: navigate   "
        + std::to_string(filtered_.size()) + " result(s) ";

    // 填满整行（反色背景）
    const std::string bar = truncate(hint, cols);
    mvprintw(row, 0, "%-*s", cols, bar.c_str());

    attroff(A_REVERSE);
}

// ─────────────────────────────────────────────────────────────────────────────
// render
// ─────────────────────────────────────────────────────────────────────────────

void InteractiveSearch::render() {
    int rows = 0, cols = 0;
    getmaxyx(stdscr, rows, cols);

    // 布局：第 0 行 = 搜索框，最后 1 行 = 状态栏，中间 = 结果列表
    const int searchRow    = 0;
    const int statusRow    = rows - 1;
    const int listStart    = 1;
    const int listEnd      = statusRow; // [listStart, listEnd)

    // 分隔线（搜索框下方）
    move(1, 0);
    clrtoeol();
    attron(A_DIM);
    for (int c = 0; c < cols; ++c) mvprintw(1, c, "\xe2\x94\x80"); // "─"
    attroff(A_DIM);

    drawSearchBar(searchRow, cols);
    drawResultList(listStart + 1, listEnd, cols); // +1 跳过分隔线行
    drawStatusBar(statusRow, cols);

    // 将光标停在搜索框输入位置（视觉友好）
    const int cursorCol = std::min(2 + static_cast<int>(query_.size()), cols - 1);
    move(searchRow, cursorCol);

    refresh();
}

// ─────────────────────────────────────────────────────────────────────────────
// handleInput  — 返回 false 表示退出事件循环
// ─────────────────────────────────────────────────────────────────────────────

bool InteractiveSearch::handleInput(int key) {
    int rows = 0, cols = 0;
    getmaxyx(stdscr, rows, cols);

    // 列表可见行数（减去搜索框行、分隔线行、状态栏行）
    const int visibleLines = std::max(0, rows - 3);

    switch (key) {

    // ── 退出 ──────────────────────────────────────────────────────────────
    case 27: // ESC
        return false;

    // ── 确认复制并退出 ────────────────────────────────────────────────────
    case '\n':
    case '\r':
    case KEY_ENTER:
        if (!filtered_.empty()) {
            const Snippet& s = filtered_[static_cast<std::size_t>(selectedIndex_)];
            try {
                clipboard_->copy(s.content);
            } catch (...) {
                // 剪贴板失败时静默退出，不在 TUI 内打印错误
                // （endwin 后由调用方处理）
            }
        }
        return false;

    // ── 向上移动 ──────────────────────────────────────────────────────────
    case KEY_UP:
        if (selectedIndex_ > 0) {
            --selectedIndex_;
            // 若选中项滚出顶部可视区域，向上滚动
            if (selectedIndex_ < scrollOffset_) {
                scrollOffset_ = selectedIndex_;
            }
        }
        break;

    // ── 向下移动 ──────────────────────────────────────────────────────────
    case KEY_DOWN: {
        const int maxIdx = static_cast<int>(filtered_.size()) - 1;
        if (selectedIndex_ < maxIdx) {
            ++selectedIndex_;
            // 若选中项滚出底部可视区域，向下滚动
            if (selectedIndex_ >= scrollOffset_ + visibleLines) {
                scrollOffset_ = selectedIndex_ - visibleLines + 1;
            }
        }
        break;
    }

    // ── Backspace 删除最后一个字符 ────────────────────────────────────────
    case KEY_BACKSPACE:
    case 127:  // DEL (某些终端)
    case '\b': // ^H
        if (!query_.empty()) {
            query_.pop_back();
            filterSnippets();
        }
        break;

    // ── 终端窗口大小变化 ──────────────────────────────────────────────────
    case KEY_RESIZE:
        // ncurses 会自动更新 LINES/COLS，下次 render() 时 getmaxyx 取新值
        // 确保 scrollOffset 不超出新的可视范围
        if (visibleLines > 0 &&
            scrollOffset_ > selectedIndex_ - visibleLines + 1)
        {
            scrollOffset_ = std::max(0, selectedIndex_ - visibleLines + 1);
        }
        break;

    // ── 普通可打印字符：追加到 query ──────────────────────────────────────
    default:
        if (key >= 32 && key <= 126) { // printable ASCII
            query_ += static_cast<char>(key);
            filterSnippets();
        }
        break;
    }

    return true; // 继续循环
}

// ─────────────────────────────────────────────────────────────────────────────
// run — 公开入口
// ─────────────────────────────────────────────────────────────────────────────

void InteractiveSearch::run() {
    // ── curses 初始化 ──────────────────────────────────────────────────────
    if (initscr() == nullptr) {
        throw std::runtime_error(
            "InteractiveSearch: failed to initialize ncurses (initscr returned NULL).");
    }

    noecho();                    // 不回显按键
    cbreak();                    // 逐字符输入，不等待换行
    keypad(stdscr, TRUE);        // 启用方向键、功能键等特殊键值
    curs_set(0);                 // 隐藏系统光标（由 render 手动定位）
    #ifndef _WIN32
        set_escdelay(25);
    #endif

    // 若终端支持颜色，初始化但本实现以属性（A_REVERSE/A_BOLD）为主
    if (has_colors()) {
        start_color();
        use_default_colors();
    }

    // ── 事件循环 ───────────────────────────────────────────────────────────
    render();

    bool running = true;
    while (running) {
        const int key = getch();
        running = handleInput(key);
        if (running) {
            render();
        }
    }

    // ── 清理终端状态 ───────────────────────────────────────────────────────
    endwin();
}

} // namespace snip
