#pragma once

#include <string>

namespace snip {

/// 剪贴板操作纯虚接口
/// 隔离平台差异（macOS pbcopy/pbpaste、Linux xclip/xsel、Windows clip 等）
class IClipboard {
public:
    virtual ~IClipboard() = default;

    /// 将 text 写入系统剪贴板
    virtual void copy(const std::string& text) = 0;

    /// 从系统剪贴板读取内容并返回
    virtual std::string paste() = 0;
};

} // namespace snip
