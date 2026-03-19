#pragma once

#include <string>
#include "interfaces/IClipboard.hpp"

namespace snip {

/// IClipboard 的平台适配实现
/// 编译时通过预处理宏自动选择 Windows / macOS / Linux 后端
class Clipboard : public IClipboard {
public:
    Clipboard()           = default;
    ~Clipboard() override = default;

    // 禁止拷贝
    Clipboard(const Clipboard&)            = delete;
    Clipboard& operator=(const Clipboard&) = delete;

    /// 将 text 写入系统剪贴板
    void copy(const std::string& text) override;

    /// 从系统剪贴板读取内容
    std::string paste() override;
};

} // namespace snip
