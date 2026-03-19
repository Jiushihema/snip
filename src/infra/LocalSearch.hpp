#pragma once

#include <string>
#include <vector>
#include "interfaces/ISearchEngine.hpp"
#include "models/Snippet.hpp"

namespace snip {

/// ISearchEngine 的本地纯内存实现
/// 对 title / content / tags 做大小写不敏感子串匹配
class LocalSearch : public ISearchEngine {
public:
    LocalSearch()           = default;
    ~LocalSearch() override = default;

    std::vector<Snippet> search(
        const std::string&          query,
        const std::vector<Snippet>& corpus
    ) override;

    // 本地搜索不支持流式输出
    bool supportsStreaming() const override { return false; }

private:
    /// 将字符串转为全小写（仅处理 ASCII，足够 CLI 场景使用）
    static std::string toLower(const std::string& s);

    /// 判断 haystack 是否包含 needle（均已转小写）
    static bool containsIgnoreCase(const std::string& haystack,
                                   const std::string& needle);
};

} // namespace snip
