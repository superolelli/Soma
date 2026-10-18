#pragma once

#include "content/EconomyContent.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <json.hpp>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>

namespace soma::content::detail
{
    using Json = nlohmann::json;

    [[nodiscard]] Json readJson(const std::filesystem::path& path);
    [[nodiscard]] int integer(const Json& value, const char* name, int fallback = 0);
    [[nodiscard]] double number(const Json& value, const char* name, double fallback = 0.0);
    void requireKnownProperties(const Json& value, const std::set<std::string>& allowed, const std::string& context);
    [[nodiscard]] Point readPoint(const Json& value);
    [[nodiscard]] Rectangle readRectangle(const Json& value);

    template <std::size_t Size, typename Reader>
    auto readArray(const Json& value, Reader reader)
    {
        using Element = decltype(reader(value.at(0)));
        if (!value.is_array() || value.size() != Size)
        {
            throw std::runtime_error("Presentation array has an unexpected size");
        }
        std::array<Element, Size> result;
        std::ranges::transform(value, result.begin(), reader);
        return result;
    }

    void loadItems(const Json& document, EconomyContent& content, const std::filesystem::path& contentRoot);
    void loadSkills(const Json& document, EconomyContent& content, const std::filesystem::path& contentRoot);
    void loadPresentation(const Json& document, EconomyContent& content, const std::filesystem::path& contentRoot);
} // namespace soma::content::detail
