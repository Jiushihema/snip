#include "infra/Clipboard.hpp"

#include <stdexcept>
#include <string>
#include <cstdio>
#include <cstring>
#include <array>

// ─────────────────────────────────────────────
// Windows 实现
// ─────────────────────────────────────────────
#if defined(_WIN32) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace snip {

void Clipboard::copy(const std::string& text) {
    if (!::OpenClipboard(nullptr)) {
        throw std::runtime_error("Clipboard::copy — OpenClipboard() failed");
    }

    ::EmptyClipboard();

    // 分配全局内存（+1 for null terminator）
    const std::size_t bytes = text.size() + 1;
    HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!hMem) {
        ::CloseClipboard();
        throw std::runtime_error("Clipboard::copy — GlobalAlloc() failed");
    }

    void* ptr = ::GlobalLock(hMem);
    if (!ptr) {
        ::GlobalFree(hMem);
        ::CloseClipboard();
        throw std::runtime_error("Clipboard::copy — GlobalLock() failed");
    }
    std::memcpy(ptr, text.c_str(), bytes);
    ::GlobalUnlock(hMem);

    if (!::SetClipboardData(CF_TEXT, hMem)) {
        ::GlobalFree(hMem);
        ::CloseClipboard();
        throw std::runtime_error("Clipboard::copy — SetClipboardData() failed");
    }

    // SetClipboardData 成功后，系统接管 hMem 的所有权，不可再 GlobalFree
    ::CloseClipboard();
}

std::string Clipboard::paste() {
    if (!::OpenClipboard(nullptr)) {
        throw std::runtime_error("Clipboard::paste — OpenClipboard() failed");
    }

    HANDLE hData = ::GetClipboardData(CF_TEXT);
    if (!hData) {
        ::CloseClipboard();
        throw std::runtime_error("Clipboard::paste — GetClipboardData() failed");
    }

    const char* ptr = static_cast<const char*>(::GlobalLock(hData));
    if (!ptr) {
        ::CloseClipboard();
        throw std::runtime_error("Clipboard::paste — GlobalLock() failed");
    }

    std::string result(ptr);
    ::GlobalUnlock(hData);
    ::CloseClipboard();
    return result;
}

} // namespace snip

// ─────────────────────────────────────────────
// macOS 实现
// ─────────────────────────────────────────────
#elif defined(__APPLE__)

namespace snip {

void Clipboard::copy(const std::string& text) {
    FILE* pipe = ::popen("pbcopy", "w");
    if (!pipe) {
        throw std::runtime_error("Clipboard::copy — popen(pbcopy) failed");
    }
    ::fwrite(text.data(), 1, text.size(), pipe);
    const int ret = ::pclose(pipe);
    if (ret != 0) {
        throw std::runtime_error("Clipboard::copy — pbcopy exited with error");
    }
}

std::string Clipboard::paste() {
    FILE* pipe = ::popen("pbpaste", "r");
    if (!pipe) {
        throw std::runtime_error("Clipboard::paste — popen(pbpaste) failed");
    }

    std::string result;
    std::array<char, 4096> buf{};
    while (::fgets(buf.data(), static_cast<int>(buf.size()), pipe)) {
        result += buf.data();
    }
    ::pclose(pipe);
    return result;
}

} // namespace snip

// ─────────────────────────────────────────────
// Linux 实现
// ─────────────────────────────────────────────
#else

namespace snip {

void Clipboard::copy(const std::string& text) {
    // 优先尝试 xclip，不存在时回退到 xsel
    FILE* pipe = ::popen("xclip -selection clipboard 2>/dev/null || "
                         "xsel --clipboard --input", "w");
    if (!pipe) {
        throw std::runtime_error("Clipboard::copy — popen(xclip/xsel) failed. "
                                 "Please install xclip or xsel.");
    }
    ::fwrite(text.data(), 1, text.size(), pipe);
    const int ret = ::pclose(pipe);
    if (ret != 0) {
        throw std::runtime_error("Clipboard::copy — xclip/xsel exited with error");
    }
}

std::string Clipboard::paste() {
    FILE* pipe = ::popen("xclip -selection clipboard -o 2>/dev/null || "
                         "xsel --clipboard --output", "r");
    if (!pipe) {
        throw std::runtime_error("Clipboard::paste — popen(xclip/xsel) failed. "
                                 "Please install xclip or xsel.");
    }

    std::string result;
    std::array<char, 4096> buf{};
    while (::fgets(buf.data(), static_cast<int>(buf.size()), pipe)) {
        result += buf.data();
    }
    ::pclose(pipe);
    return result;
}

} // namespace snip

#endif
