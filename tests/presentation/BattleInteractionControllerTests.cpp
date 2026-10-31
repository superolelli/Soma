#include "presentation/sfml/battle/BattleInteractionController.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <source_location>
#include <utility>
#include <vector>

namespace
{
    const soma::gameplay::ContentDefinitionId firstPlayerDefinition{"player.first"};
    const soma::gameplay::ContentDefinitionId secondPlayerDefinition{"player.second"};
    const soma::gameplay::ContentDefinitionId enemyDefinition{"enemy.test"};
    const soma::gameplay::AbilityId firstAbility{"ability.first"};
    const soma::gameplay::AbilityId secondAbility{"ability.second"};
    const soma::gameplay::EntityId firstPlayer{"player.first.slot0"};
    const soma::gameplay::EntityId secondPlayer{"player.second.slot1"};
    const soma::gameplay::EntityId enemy{"enemy.test.slot4"};

    void check(bool condition, const std::source_location location = std::source_location::current())
    {
        if (!condition)
        {
            std::cerr << "Check failed at " << location.file_name() << ':' << location.line() << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    class MinimumRandom final : public soma::gameplay::IRandom
    {
      public:
        [[nodiscard]] std::uint64_t seed() const noexcept override { return 1; }
        int between(int minimumInclusive, int) override { return minimumInclusive; }
        bool chance(double probability) override { return probability > 0.0; }
    };

    class RecordingActionStarter final : public soma::presentation::IBattleActionStarter
    {
      public:
        void start(soma::simulation::BattleActionCommand command, bool announce) override
        {
            commands.push_back(std::move(command));
            announcements.push_back(announce);
        }
        void startPass(const soma::gameplay::EntityId& actor) override { passes.push_back(actor); }

        std::vector<soma::simulation::BattleActionCommand> commands;
        std::vector<bool> announcements;
        std::vector<soma::gameplay::EntityId> passes;
    };

    soma::content::BattleContent makeContent()
    {
        soma::content::BattleContent content;
        const auto addAbility = [&content](const soma::gameplay::AbilityId& id) {
            soma::content::AbilityDefinition ability;
            ability.id = id;
            ability.name = id.value();
            ability.targeting.relation = soma::content::TargetRelation::opposingTeam;
            content.abilities.emplace(id, std::move(ability));
        };
        addAbility(firstAbility);
        addAbility(secondAbility);

        const auto addCombatant = [&content](const soma::gameplay::ContentDefinitionId& id,
                                             soma::gameplay::Team team,
                                             int initiative,
                                             std::vector<soma::gameplay::AbilityId> abilities) {
            soma::content::CombatantDefinition combatant;
            combatant.id = id;
            combatant.name = id.value();
            combatant.team = team;
            combatant.attributes.maximumHealth = 10;
            combatant.attributes.initiative = initiative;
            combatant.abilities = std::move(abilities);
            content.combatants.emplace(id, std::move(combatant));
        };
        addCombatant(firstPlayerDefinition, soma::gameplay::Team::players, 30, {firstAbility});
        addCombatant(secondPlayerDefinition, soma::gameplay::Team::players, 20, {secondAbility});
        addCombatant(enemyDefinition, soma::gameplay::Team::enemies, 10, {});
        return content;
    }

    soma::presentation::CombatantView view(const soma::gameplay::ContentDefinitionId& definition, sf::FloatRect bounds)
    {
        return {nullptr, definition, {}, bounds, 0.6F, 0.0F, 10.0F, 10.0F, 10, 0.7F, 10, 0.0F, 0.0F, 0, 0.7F};
    }
} // namespace

int main()
{
    auto content = makeContent();
    MinimumRandom random;
    soma::simulation::BattleSimulation battle{
        content, {{firstPlayerDefinition, 0}, {secondPlayerDefinition, 1}, {enemyDefinition, 4}}, random};
    soma::simulation::EnemyPolicy enemyPolicy{random};
    RecordingActionStarter actionStarter;
    soma::presentation::BattleLaunchOptions launchOptions;
    soma::presentation::CombatantViews views;
    views.emplace(firstPlayer, view(firstPlayerDefinition, {{60.0F, 0.0F}, {10.0F, 10.0F}}));
    views.emplace(secondPlayer, view(secondPlayerDefinition, {{80.0F, 0.0F}, {10.0F, 10.0F}}));
    views.emplace(enemy, view(enemyDefinition, {{20.0F, 0.0F}, {10.0F, 10.0F}}));
    std::optional<soma::presentation::PresentedAction> activeAction;
    const auto hitRegions = [](const soma::simulation::CombatantState& actor) {
        const auto ability = actor.definition == firstPlayerDefinition ? firstAbility : secondAbility;
        return std::vector{soma::presentation::BattleInteractionController::AbilityHitRegion{
            {{0.0F, 0.0F}, {10.0F, 10.0F}}, ability}};
    };
    soma::presentation::BattleInteractionController interaction{content,
                                                                 battle,
                                                                 enemyPolicy,
                                                                 actionStarter,
                                                                 launchOptions,
                                                                 views,
                                                                 activeAction,
                                                                 {{40.0F, 0.0F}, {10.0F, 10.0F}},
                                                                 hitRegions};

    interaction.prepareTurn();
    interaction.handlePointerClick({5.0F, 5.0F});
    check(interaction.selectedAbility() == firstAbility);
    check(interaction.clickableAt({25.0F, 5.0F}));
    interaction.handlePointerClick({25.0F, 5.0F});
    check(actionStarter.commands.size() == 1);
    check(actionStarter.commands.front().actor == firstPlayer);
    check(actionStarter.commands.front().ability == firstAbility);
    check(actionStarter.commands.front().primaryTarget == enemy);
    check(!interaction.selectedAbility());

    interaction.handlePointerClick({45.0F, 5.0F});
    check(actionStarter.passes == std::vector{firstPlayer});

    activeAction = soma::presentation::PresentedAction{{firstPlayer, firstAbility, enemy}};
    interaction.handlePointerClick({5.0F, 5.0F});
    interaction.handlePointerClick({45.0F, 5.0F});
    check(!interaction.selectedAbility());
    check(actionStarter.commands.size() == 1);
    check(actionStarter.passes.size() == 1);
    check(!interaction.clickableAt({5.0F, 5.0F}));

    activeAction.reset();
    interaction.handlePointerClick({5.0F, 5.0F});
    check(interaction.selectedAbility() == firstAbility);
    check(battle.passTurn(firstPlayer).hasValue());
    interaction.prepareTurn();
    check(battle.currentActor() == secondPlayer);
    check(!interaction.selectedAbility());
}
