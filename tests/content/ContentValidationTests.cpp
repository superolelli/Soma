#include "content/battle/BattleContentValidation.hpp"
#include "content/world/WorldContentValidation.hpp"

#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <source_location>
#include <string_view>
#include <vector>

namespace
{
    void check(bool condition, const std::source_location location = std::source_location::current())
    {
        if (!condition)
        {
            std::cerr << "Check failed at " << location.file_name() << ':' << location.line() << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    template <typename Operation>
    void checkRejected(Operation&& operation, std::string_view expectedDiagnostic)
    {
        try
        {
            operation();
        }
        catch (const std::exception& exception)
        {
            check(std::string_view{exception.what()}.find(expectedDiagnostic) != std::string_view::npos);
            return;
        }
        check(false);
    }

    soma::content::BattleContent validMinimalBattleContent()
    {
        soma::content::BattleContent content;
        const soma::gameplay::ContentDefinitionId combatantId{"player.test"};
        soma::content::CombatantDefinition combatant;
        combatant.id = combatantId;
        combatant.name = "Test";
        combatant.team = soma::gameplay::Team::players;
        combatant.attributes.maximumHealth = 10;
        content.combatants.emplace(combatantId, combatant);
        content.presentation.emplace(
            combatantId, soma::content::CombatantPresentationDefinition{combatantId, 0, {}, "test", 0.0, {}});
        const soma::gameplay::ContentDefinitionId encounterId{"encounter.test"};
        content.encounters.emplace(
            encounterId, soma::content::BattleContent::Encounter{encounterId, {{combatantId, 0}}, {}, {}});
        return content;
    }

    void battleValidationRejectsInvalidContracts()
    {
        using Mutation = std::function<void(soma::content::BattleContent&)>;
        struct RejectionCase
        {
            Mutation mutate;
            std::string_view diagnostic;
        };
        const std::vector<RejectionCase> cases{
            {[](auto& content) { content.layout.executionTimeout = 0.0; }, "Invalid battle timing"},
            {[](auto& content) {
                 content.combatants.begin()->second.abilities.push_back(soma::gameplay::AbilityId{"ability.missing"});
             },
             "unknown ability"},
            {[](auto& content) {
                 auto& combatants = content.encounters.begin()->second.combatants;
                 combatants.push_back(combatants.front());
             },
             "invalid or duplicate slot"}};

        for (const auto& rejection : cases)
        {
            auto content = validMinimalBattleContent();
            rejection.mutate(content);
            checkRejected(
                [&] { soma::content::detail::validateBattleContent(content, std::filesystem::current_path()); }, rejection.diagnostic);
        }
    }

    void worldValidationRejectsAnUnreachableEncounterBudget()
    {
        soma::content::WorldContent world;
        const soma::gameplay::EncounterId encounterId{"encounter.test"};
        soma::content::EncounterDefinition encounter;
        encounter.id = encounterId;
        encounter.budgetCost = 2;
        world.encounters.emplace(encounterId, encounter);

        soma::content::LevelDefinition level;
        level.id = soma::gameplay::LevelId{"level.test"};
        level.roomCount = 3;
        level.minimumEncounterBudget = 5;
        level.maximumEncounterBudget = 5;
        level.encounters.push_back(encounterId);

        checkRejected(
            [&] { soma::content::detail::validateEncounterBudgetReachability(level, world); }, "No encounter budget");

        level.minimumEncounterBudget = 4;
        soma::content::detail::validateEncounterBudgetReachability(level, world);
    }
} // namespace

int main()
{
    battleValidationRejectsInvalidContracts();
    worldValidationRejectsAnUnreachableEncounterBudget();
}
