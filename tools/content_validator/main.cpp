#include "content/battle/BattleContentLoader.hpp"
#include "content/economy/EconomyContentLoader.hpp"
#include "content/flow/GameFlowContentLoader.hpp"
#include "content/rules/RuleContentLoader.hpp"
#include "content/world/WorldContentLoader.hpp"

#include <filesystem>
#include <iostream>
#include <ranges>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: soma_content_validator <content-root>\n";
        return 2;
    }
    const auto contentRoot = std::filesystem::absolute(argv[1]);
    const auto battleResult = soma::content::BattleContentLoader{}.load(contentRoot);
    const auto economyResult = soma::content::EconomyContentLoader{}.load(contentRoot);
    const auto ruleResult = soma::content::RuleContentLoader{}.load(contentRoot);
    const auto flowResult = soma::content::GameFlowContentLoader{}.load(contentRoot);
    const auto worldResult = soma::content::WorldContentLoader{}.load(contentRoot, battleResult.content, economyResult.content);
    auto printDiagnostic = [](const auto& diagnostic) {
        std::cerr << (diagnostic.severity == soma::gameplay::DiagnosticSeverity::error ? "error" : "warning") << " [" << diagnostic.code << "] "
                  << diagnostic.file.generic_string();
        if (!diagnostic.field.empty())
        {
            std::cerr << ':' << diagnostic.field;
        }
        std::cerr << ": " << diagnostic.message << '\n';
    };
    for (const auto& diagnostic : battleResult.diagnostics)
    {
        printDiagnostic(diagnostic);
    }
    for (const auto& diagnostic : economyResult.diagnostics)
    {
        printDiagnostic(diagnostic);
    }
    for (const auto& diagnostic : ruleResult.diagnostics)
    {
        printDiagnostic(diagnostic);
    }
    for (const auto& diagnostic : flowResult.diagnostics)
    {
        printDiagnostic(diagnostic);
    }
    for (const auto& diagnostic : worldResult.diagnostics)
    {
        printDiagnostic(diagnostic);
    }
    auto crossContentValid = true;
    if (battleResult.valid() && economyResult.valid())
    {
        for (const auto& set : economyResult.content.abilitySets)
        {
            const auto combatant = battleResult.content.combatants.find(set.player);
            if (combatant == battleResult.content.combatants.end() || set.slot >= combatant->second.abilities.size())
            {
                std::cerr << "error [ability_set.combatant]: Missing player or logical ability slot for " << set.player.value() << '\n';
                crossContentValid = false;
                continue;
            }
            const auto& presentation = battleResult.content.presentation.at(set.player);
            for (const auto& version : set.versions)
            {
                if (!battleResult.content.abilities.contains(version.ability) || !presentation.abilityIcons.contains(version.ability))
                {
                    std::cerr << "error [ability_set.version]: Missing ability definition or battle icon for " << version.ability.value() << '\n';
                    crossContentValid = false;
                }
            }
        }
        for (const auto& [id, combatant] : battleResult.content.combatants)
        {
            if (combatant.team != soma::gameplay::Team::players) continue;
            const auto setCount = std::ranges::count(economyResult.content.abilitySets, id, &soma::content::AbilitySetDefinition::player);
            if (setCount != static_cast<decltype(setCount)>(combatant.abilities.size()))
            {
                std::cerr << "error [ability_set.count]: Player " << id.value() << " does not define one version set per ability slot\n";
                crossContentValid = false;
            }
        }
        for (const auto& [id, item] : economyResult.content.items)
        {
            const auto* equipment = std::get_if<soma::gameplay::EquipmentItem>(&item.properties);
            if (equipment != nullptr && equipment->battleEffect.has_value() && !battleResult.content.itemActivations.contains(id))
            {
                std::cerr << "error [item_activation.presentation]: Equipment battle effect has no presentation for " << id.value() << '\n';
                crossContentValid = false;
            }
        }
        for (const auto& [id, presentation] : battleResult.content.itemActivations)
        {
            static_cast<void>(presentation);
            const auto item = economyResult.content.items.find(id);
            const auto* equipment = item == economyResult.content.items.end()
                                        ? nullptr
                                        : std::get_if<soma::gameplay::EquipmentItem>(&item->second.properties);
            if (equipment == nullptr || !equipment->battleEffect.has_value())
            {
                std::cerr << "error [item_activation.effect]: Item activation has no matching equipment battle effect for " << id.value() << '\n';
                crossContentValid = false;
            }
        }
    }
    const auto battleDefinitions =
        battleResult.content.combatants.size() + battleResult.content.abilities.size() + battleResult.content.encounters.size();
    const auto economyDefinitions =
        economyResult.content.items.size() + economyResult.content.skills.size() + economyResult.content.abilitySets.size();
    const auto worldDefinitions =
        worldResult.content.levels.size() + worldResult.content.encounters.size() + worldResult.content.lootables.size();
    const auto definitionCount = battleDefinitions + economyDefinitions + worldDefinitions + ruleResult.content.fragments.size();
    std::cout << "Validated " << definitionCount << " definitions from " << contentRoot.generic_string() << '\n';
    return battleResult.valid() && economyResult.valid() && ruleResult.valid() && flowResult.valid() && worldResult.valid() && crossContentValid ? 0
                                                                                                                                              : 1;
}
