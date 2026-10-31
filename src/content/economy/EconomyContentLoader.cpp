#include "content/economy/EconomyContentLoader.hpp"

#include "content/economy/EconomyContentReader.hpp"

#include <stdexcept>

namespace soma::content::detail
{
    int integer(const Json& value, const char* name, int fallback)
    {
        return value.contains(name) ? value.at(name).get<int>() : fallback;
    }

    double number(const Json& value, const char* name, double fallback)
    {
        return value.contains(name) ? value.at(name).get<double>() : fallback;
    }

    void requireKnownProperties(const Json& value, const std::set<std::string>& allowed, const std::string& context)
    {
        for (const auto& [name, property] : value.items())
        {
            static_cast<void>(property);
            if (!allowed.contains(name))
            {
                throw std::runtime_error("Unknown " + context + " property: " + name);
            }
        }
    }

    Point readPoint(const Json& value)
    {
        if (!value.is_array() || value.size() != 2)
        {
            throw std::runtime_error("Expected a two-component point");
        }
        return {value.at(0).get<int>(), value.at(1).get<int>()};
    }

    Rectangle readRectangle(const Json& value)
    {
        if (!value.is_array() || value.size() != 4)
        {
            throw std::runtime_error("Expected a four-component rectangle");
        }
        return {value.at(0).get<int>(), value.at(1).get<int>(), value.at(2).get<int>(), value.at(3).get<int>()};
    }

} // namespace soma::content::detail

namespace soma::content
{
    bool EconomyContentLoadResult::valid() const
    {
        return diagnostics.empty();
    }

    EconomyContentLoadResult EconomyContentLoader::load(const std::filesystem::path& contentRoot) const
    {
        EconomyContentLoadResult result;
        try
        {
            const auto itemDocument = detail::readJsonFile(contentRoot / "economy" / "items.json");
            if (itemDocument.value("schema_version", 0) != 1 || itemDocument.value("content_kind", std::string{}) != "economy_items")
            {
                throw std::runtime_error("Expected economy item schema version 1");
            }
            detail::loadItems(itemDocument, result.content, contentRoot);
            const auto skillDocument = detail::readJsonFile(contentRoot / "economy" / "skills.json");
            if (skillDocument.value("schema_version", 0) != 1 || skillDocument.value("content_kind", std::string{}) != "economy_skills")
            {
                throw std::runtime_error("Expected economy skill schema version 1");
            }
            detail::loadSkills(skillDocument, result.content, contentRoot);
            const auto presentationDocument = detail::readJsonFile(contentRoot / "economy" / "presentation.json");
            if (presentationDocument.value("schema_version", 0) != 1 ||
                presentationDocument.value("content_kind", std::string{}) != "economy_presentation")
            {
                throw std::runtime_error("Expected economy presentation schema version 1");
            }
            detail::loadPresentation(presentationDocument, result.content, contentRoot);
        }
        catch (const std::exception& exception)
        {
            result.diagnostics.push_back({gameplay::DiagnosticSeverity::error, "economy-content", contentRoot, {}, exception.what()});
        }
        return result;
    }
} // namespace soma::content
