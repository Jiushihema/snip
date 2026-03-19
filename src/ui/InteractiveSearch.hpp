#pragma once

#include <string>
#include <vector>
#include <memory>

#include "models/Snippet.hpp"
#include "interfaces/IClipboard.hpp"

namespace snip {

/// 交互式模糊搜索 TUI
/// 基于 ncurses / PDCurses，在终端内提供实时过滤 + 键盘导航 + 一键复制
class InteractiveSearch {
public:
    /// @param corpus     初始代码片段数据集（全量）
    /// @param clipboard  剪贴板接口，用于 Enter 键触发复制
    InteractiveSearch(
        std::vector<Snippet>           corpus,
        std::shared_ptr<IClipboard>    clipboard
    );

    ~InteractiveSearch() = default;

    // 禁止拷贝与移动（curses 全局状态不可复制）
    InteractiveSearch(const InteractiveSearch&)            = delete;
    InteractiveSearch& operator=(const InteractiveSearch&) = delete;
    InteractiveSearch(InteractiveSearch&&)                 = delete;
    InteractiveSearch& operator=(InteractiveSearch&&)      = delete;

    /// 启动交互界面，阻塞直到用户按 ESC 或 Enter 退出
    /// 退出前自动调用 endwin() 恢复终端状态
    void run();

private:
    // ── 数据 ──────────────────────────────────────────────────────────────
    std::vector<Snippet>        corpus_;       ///< 原始全量数据集
    std::shared_ptr<IClipboard> clipboard_;    ///< 剪贴板依赖

    std::string          query_;               ///< 当前搜索输入字符串
    std::vector<Snippet> filtered_;            ///< 过滤后的结果列表
    int                  selectedIndex_ = 0;   ///< 当前高亮行（在 filtered_ 中的下标）
    int                  scrollOffset_  = 0;   ///< 列表区域的滚动偏移量

    // ── 私有方法 ──────────────────────────────────────────────────────────

    /// 根据 query_ 对 corpus_ 做大小写不敏感子串匹配，结果写入 filtered_
    /// 同时将 selectedIndex_ 和 scrollOffset_ 重置为 0
    void filterSnippets();

    /// 重绘整个 TUI 界面（搜索框 + 结果列表 + 底部提示栏）
    void render();

    /// 处理单次按键事件
    /// @param key  ncurses getch() 返回的键值
    /// @return     true 表示继续循环，false 表示请求退出
    bool handleInput(int key);

    // ── 渲染辅助 ──────────────────────────────────────────────────────────

    /// 在指定行绘制搜索框
    void drawSearchBar(int row, int cols);

    /// 在指定行区间绘制结果列表（含滚动与高亮）
    /// @param startRow   列表区域起始行
    /// @param endRow     列表区域结束行（不含）
    /// @param cols       终端列数
    void drawResultList(int startRow, int endRow, int cols);

    /// 在指定行绘制底部操作提示栏
    void drawStatusBar(int row, int cols);

    /// 在字符串 text 中高亮所有与 query_ 匹配的子串（A_BOLD）
    /// @param col   当前列偏移，绘制完成后更新
    void printWithHighlight(int row, int& col, const std::string& text,
                            const std::string& lowerQuery, int maxCols);

    /// 将字符串转为全小写（ASCII）
    static std::string toLower(const std::string& s);

    /// 将字符串截断到 maxLen 个字符（超出时末尾替换为 "…"）
    static std::string truncate(const std::string& s, int maxLen);
};

} // namespace snip
