#include "infra/LocalSearch.hpp"

#include <algorithm>
#include <cctype>

namespace snip {

// ─────────────────────────────────────────────
// 私有工具函数
// ─────────────────────────────────────────────

std::string LocalSearch::toLower(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (const unsigned char c : s) {
        result.push_back(static_cast<char>(std::tolower(c)));
    }
    return result;
}

bool LocalSearch::containsIgnoreCase(const std::string& haystack,
                                     const std::string& needle) {
    if (needle.empty()) return true;
    // haystack 与 needle 均已在调用处转为小写，直接查找
    return haystack.find(needle) != std::string::npos;
}

// ─────────────────────────────────────────────
// ISearchEngine 接口实现
// ─────────────────────────────────────────────

std::vector<Snippet> LocalSearch::search(
    const std::string&          query,
    const std::vector<Snippet>& corpus)
{
    // query 为空时返回全部
    if (query.empty()) return corpus;

    const std::string lowerQuery = toLower(query);

    std::vector<Snippet> results;
    results.reserve(corpus.size());

    for (const Snippet& snippet : corpus) {
        const bool matched =
            containsIgnoreCase(toLower(snippet.title),   lowerQuery) ||
            containsIgnoreCase(toLower(snippet.content), lowerQuery) ||
            containsIgnoreCase(toLower(snippet.tags),    lowerQuery);

        if (matched) {
            results.push_back(snippet);
        }
    }

    return results;
}

} // namespace snip
