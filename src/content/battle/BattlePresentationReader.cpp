#include "content/battle/BattleContentReader.hpp"

#include <json.hpp>
#include <stdexcept>

namespace soma::content
{
    using json = nlohmann::json;

    void detail::loadBattlePresentation(const json& document, BattleContent& content)
    {
        for (const auto& [id, path] : document.at("assets").items())
        {
            content.assets.emplace(id, path.get<std::string>());
        }
        for (const auto& [id, path] : document.at("sounds").items())
        {
            content.sounds.emplace(id, path.get<std::string>());
        }
        for (const auto& [id, description] : document.at("stat_tooltips").items())
        {
            if (!content.statTooltips.emplace(id, description.get<std::string>()).second || content.statTooltips.at(id).empty())
            {
                throw std::runtime_error("Invalid or duplicate stat tooltip: " + id);
            }
        }
        content.layout.referenceWidth = document.at("reference_resolution").at("width");
        content.layout.referenceHeight = document.at("reference_resolution").at("height");
        const auto& layout = document.at("layout");
        content.layout.normalScale = layout.at("normal_scale");
        content.layout.abilityScale = layout.at("ability_scale");
        content.layout.groundY = layout.at("ground_y");
        content.layout.playerStartX = layout.at("player_start_x");
        content.layout.playerSpacing = layout.at("player_spacing");
        content.layout.enemyStartFromViewCenter = layout.at("enemy_start_x_from_view_center");
        content.layout.enemySpacing = layout.at("enemy_spacing");
        content.layout.abilityStageY = layout.at("ability_stage_y");
        content.layout.abilityAttackerXPlayer = layout.at("ability_attacker_x_player");
        content.layout.abilityTargetXPlayer = layout.at("ability_target_x_player");
        content.layout.abilityGroupPadding = layout.at("ability_group_padding");
        const auto& abilityPanel = layout.at("ability_panel");
        content.layout.abilityPanelX = abilityPanel.at("x");
        content.layout.abilityPanelY = abilityPanel.at("y");
        const auto& abilityButtons = layout.at("ability_buttons");
        content.layout.abilityButtonStartX = abilityButtons.at("start_x");
        content.layout.abilityButtonY = abilityButtons.at("y");
        content.layout.abilityButtonSpacing = abilityButtons.at("spacing");
        const auto& informationPanel = layout.at("combatant_information_panel");
        content.layout.informationPanelX = informationPanel.at("x");
        content.layout.informationPanelY = informationPanel.at("y");
        const auto& announcement = layout.at("enemy_announcement");
        content.layout.announcementX = announcement.at("x");
        content.layout.announcementY = announcement.at("y");
        content.layout.announcementTextX = announcement.at("text_x");
        content.layout.announcementTextY = announcement.at("text_y");
        const auto& battleResult = layout.at("battle_result");
        content.layout.battleResultX = battleResult.at("x");
        content.layout.battleResultY = battleResult.at("y");
        const auto& turnOrderNumbers = layout.at("turn_order_numbers");
        content.layout.turnOrderNumberOffsetY = turnOrderNumbers.at("offset_y");
        content.layout.turnOrderNumberCharacterSize = turnOrderNumbers.at("character_size");
        const auto& expectedAttack = layout.at("expected_attack");
        content.layout.expectedAttackCenterX = expectedAttack.at("center_x");
        content.layout.expectedAttackSingleY = expectedAttack.at("single_line_y");
        content.layout.expectedAttackMultipleY = expectedAttack.at("multiple_lines_y");
        content.layout.expectedAttackLineSpacing = expectedAttack.at("line_spacing");
        content.layout.expectedAttackTextSize = expectedAttack.at("character_size");
        const auto& timing = layout.at("timing");
        content.layout.announcementDuration = timing.at("announcement_seconds");
        content.layout.executionTimeout = timing.at("execution_timeout_seconds");
        content.layout.reactionDuration = timing.at("reaction_seconds");
        content.layout.resultDuration = timing.at("result_seconds");
        content.layout.wakeResultDuration = timing.at("wake_result_seconds");
        content.layout.defeatDuration = timing.at("defeat_seconds");
        const auto& itemActivation = layout.at("item_activation");
        content.layout.itemSpeechDuration = itemActivation.at("speech_duration_seconds");
        content.layout.itemSpeechMaximumWidth = itemActivation.at("maximum_text_width");
        content.layout.itemSpeechCharacterSize = itemActivation.at("character_size");
        content.layout.itemSpeechPaddingX = itemActivation.at("padding").at(0);
        content.layout.itemSpeechPaddingY = itemActivation.at("padding").at(1);
        content.layout.itemSpeechSpeakerOffsetX = itemActivation.at("speaker_offset").at(0);
        content.layout.itemSpeechSpeakerOffsetY = itemActivation.at("speaker_offset").at(1);
        content.layout.itemSpeechTextVerticalOffset = itemActivation.at("text_vertical_offset");

        for (const auto& value : document.at("item_activations"))
        {
            const gameplay::ItemId item{value.at("item").get<std::string>()};
            const ItemActivationPresentation presentation{value.at("speech").get<std::string>(),
                                                            value.at("notification").get<std::string>()};
            if (presentation.speech.empty() || presentation.notification.empty() ||
                !content.itemActivations.emplace(item, presentation).second)
            {
                throw std::runtime_error("Invalid or duplicate item activation presentation: " + item.value());
            }
        }

        for (const auto& value : document.at("combatants"))
        {
            CombatantPresentationDefinition definition;
            definition.id = gameplay::ContentDefinitionId{value.at("id").get<std::string>()};
            definition.preferredSlot = value.at("slot");
            definition.model = value.at("model").get<std::string>();
            definition.entity = value.at("entity").get<std::string>();
            definition.scaleAdjustment = value.at("scale_adjustment");
            if (value.contains("ability_icons"))
            {
                for (const auto& [ability, icon] : value.at("ability_icons").items())
                {
                    definition.abilityIcons.emplace(gameplay::AbilityId{ability}, icon.get<std::string>());
                }
            }
            content.presentation.emplace(definition.id, std::move(definition));
        }
    }
} // namespace soma::content
