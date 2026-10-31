#include "content/battle/BattleContentLoader.hpp"
#include "content/battle/BattleContentReader.hpp"
#include "content/battle/BattleContentValidation.hpp"
#include "content/JsonFileReader.hpp"

#include <stdexcept>

namespace soma::content
{
    bool BattleContentLoadResult::valid() const
    {
        return diagnostics.empty();
    }

    BattleContentLoadResult BattleContentLoader::load(const std::filesystem::path& contentRoot) const
    {
        BattleContentLoadResult result;
        try
        {
            const auto document = detail::readJsonFile(contentRoot / "battle" / "content.json");
            if (document.value("schema_version", 0) != 1 || document.value("content_kind", std::string{}) != "battle")
            {
                throw std::runtime_error("Expected battle content schema version 1");
            }
            detail::loadBattleDefinitions(document, result.content);
            detail::loadBattlePresentation(detail::readJsonFile(contentRoot / "battle" / "presentation.json"), result.content);
            detail::validateBattleContent(result.content, contentRoot);
        }
        catch (const std::exception& exception)
        {
            result.diagnostics.push_back({gameplay::DiagnosticSeverity::error, "battle-content", contentRoot, {}, exception.what()});
        }
        return result;
    }
} // namespace soma::content
