#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include "interfaces/ISearchEngine.hpp"
#include "models/Snippet.hpp"

namespace snip {

/// ISearchEngine 的 LLM 远程搜索桩类（stub）
/// 后期接入大模型 API 时在此类中完整实现
class LLMSearch : public ISearchEngine {
public:
    /// @param endpoint  API 服务地址，如 "https://api.openai.com/v1/chat/completions"
    /// @param api_key   鉴权密钥
    /// @param model     模型名称，如 "gpt-4o"
    LLMSearch(std::string endpoint,
              std::string api_key,
              std::string model)
        : endpoint_(std::move(endpoint))
        , api_key_(std::move(api_key))
        , model_(std::move(model))
    {}

    ~LLMSearch() override = default;

    // 禁止拷贝
    LLMSearch(const LLMSearch&)            = delete;
    LLMSearch& operator=(const LLMSearch&) = delete;

    std::vector<Snippet> search(
        const std::string&          query,
        const std::vector<Snippet>& corpus
    ) override
    {
        (void)query;
        (void)corpus;
        throw std::runtime_error("LLM search not yet implemented");
    }

    /// LLM 后端天然支持流式输出
    bool supportsStreaming() const override { return true; }

private:
    std::string endpoint_;
    std::string api_key_;
    std::string model_;
};

} // namespace snip
