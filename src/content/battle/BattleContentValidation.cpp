#include "content/battle/BattleContentValidation.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace soma::content
{
    void detail::validateBattleContent(const BattleContent& content, const std::filesystem::path& contentRoot)
    {
        if (content.layout.turnOrderNumberCharacterSize <= 0 || content.layout.announcementDuration <= 0.0 ||
            content.layout.executionTimeout <= 0.0 || content.layout.reactionDuration <= 0.0 || content.layout.resultDuration <= 0.0 ||
            content.layout.wakeResultDuration <= 0.0 || content.layout.defeatDuration <= 0.0)
        {
            throw std::runtime_error("Invalid battle timing or turn-order presentation layout");
        }
        if (content.layout.itemSpeechDuration <= 0.0 || content.layout.itemSpeechMaximumWidth <= 0 ||
            content.layout.itemSpeechCharacterSize <= 0 || content.layout.itemSpeechPaddingX < 0 || content.layout.itemSpeechPaddingY < 0)
        {
            throw std::runtime_error("Invalid item activation presentation layout");
        }
        for (const auto& [id, combatant] : content.combatants)
        {
            if (combatant.team == gameplay::Team::enemies && combatant.encounterCost <= 0)
            {
                throw std::runtime_error("Enemy has no positive encounter cost: " + id.value());
            }
            if (!content.presentation.contains(id))
            {
                throw std::runtime_error("Combatant has no presentation definition: " + id.value());
            }
            for (const auto& ability : combatant.abilities)
            {
                if (!content.abilities.contains(ability))
                {
                    throw std::runtime_error("Combatant references unknown ability: " + ability.value());
                }
            }
        }
        for (const auto& [id, ability] : content.abilities)
        {
            if (ability.summon.has_value() && !content.combatants.contains(ability.summon->combatant))
            {
                throw std::runtime_error("Ability references unknown summon: " + id.value());
            }
        }
        for (const auto& [id, encounter] : content.encounters)
        {
            std::vector<int> occupiedSlots;
            for (const auto& [combatant, slot] : encounter.combatants)
            {
                if (!content.combatants.contains(combatant))
                {
                    throw std::runtime_error("Encounter references unknown combatant: " + id.value());
                }
                if (slot < 0 || slot >= 8 || std::ranges::find(occupiedSlots, slot) != occupiedSlots.end())
                {
                    throw std::runtime_error("Encounter contains an invalid or duplicate slot: " + id.value());
                }
                occupiedSlots.push_back(slot);
            }
        }

        const auto repositoryRoot = contentRoot.parent_path().parent_path().parent_path();
        const auto validateAsset = [&repositoryRoot](const std::string& id, const std::filesystem::path& path) {
            if (!std::filesystem::exists(repositoryRoot / path))
            {
                throw std::runtime_error("Missing asset '" + id + "': " + path.string());
            }
        };
        for (const auto& [id, path] : content.assets)
        {
            validateAsset(id, path);
        }
        for (const auto& [id, path] : content.sounds)
        {
            validateAsset(id, path);
        }
        for (const auto& [id, presentation] : content.presentation)
        {
            validateAsset(id.value() + ".model", presentation.model);
            for (const auto& [ability, icon] : presentation.abilityIcons)
            {
                if (!content.abilities.contains(ability))
                    throw std::runtime_error("Presentation references an unknown ability icon: " + ability.value());
                validateAsset(id.value() + ".ability_icon." + ability.value(), icon);
            }
        }
        for (const auto& [id, encounter] : content.encounters)
        {
            validateAsset(id.value() + ".background", encounter.background);
            validateAsset(id.value() + ".music", encounter.music);
        }
    }
} // namespace soma::content
