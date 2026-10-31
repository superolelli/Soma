#include "simulation/battle/EnemyPolicy.hpp"
#include "simulation/battle/AbilityTargets.hpp"

#include <cstdlib>
#include <iostream>
#include <source_location>
#include <string>
#include <vector>

namespace
{
    const soma::gameplay::ContentDefinitionId actingPlayerDefinition{"player.actor"};
    const soma::gameplay::ContentDefinitionId secondPlayerDefinition{"player.second"};
    const soma::gameplay::EntityId actingPlayer{"player.actor.slot0"};
    const soma::gameplay::EntityId secondPlayer{"player.second.slot1"};

    void check(bool condition, const std::source_location location = std::source_location::current())
    {
        if (!condition)
        {
            std::cerr << "Check failed at " << location.file_name() << ':' << location.line() << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    class FixedRandom final : public soma::gameplay::IRandom
    {
      public:
        explicit FixedRandom(bool maximum)
            : maximum_(maximum)
        {
        }

        [[nodiscard]] std::uint64_t seed() const noexcept override { return maximum_ ? 2 : 1; }
        int between(int minimumInclusive, int maximumInclusive) override
        {
            return maximum_ ? maximumInclusive : minimumInclusive;
        }
        bool chance(double probability) override { return probability >= 0.5; }

      private:
        bool maximum_{};
    };

    soma::gameplay::AbilityId enemyAbility(std::string name)
    {
        return soma::gameplay::AbilityId{"ability.enemy." + std::move(name)};
    }

    void addAbility(soma::content::BattleContent& content,
                    const soma::gameplay::AbilityId& id,
                    soma::content::TargetRelation relation,
                    const soma::content::AbilityEffect& friendlyEffect = {},
                    const soma::content::AbilityEffect& hostileEffect = {})
    {
        soma::content::AbilityDefinition ability;
        ability.id = id;
        ability.name = id.value();
        ability.targeting.relation = relation;
        ability.friendlyEffect = friendlyEffect;
        ability.hostileEffect = hostileEffect;
        content.abilities.emplace(id, std::move(ability));
    }

    void addCombatant(soma::content::BattleContent& content,
                      const soma::gameplay::ContentDefinitionId& id,
                      soma::gameplay::Team team,
                      int initiative,
                      std::vector<soma::gameplay::AbilityId> abilities,
                      std::string aiPolicy = {})
    {
        soma::content::CombatantDefinition combatant;
        combatant.id = id;
        combatant.name = id.value();
        combatant.team = team;
        combatant.attributes.maximumHealth = 20;
        combatant.attributes.damageMinimum = 7;
        combatant.attributes.damageMaximum = 7;
        combatant.attributes.precision = 100;
        combatant.attributes.initiative = initiative;
        combatant.abilities = std::move(abilities);
        combatant.aiPolicy = std::move(aiPolicy);
        content.combatants.emplace(id, std::move(combatant));
    }

    soma::content::BattleContent baseContent(const soma::gameplay::AbilityId& preparation,
                                             const soma::content::AbilityEffect& friendlyEffect,
                                             const soma::gameplay::ContentDefinitionId& enemyDefinition,
                                             std::string enemyPolicy,
                                             std::vector<soma::gameplay::AbilityId> enemyAbilities,
                                             bool includeSecondEnemy = false)
    {
        soma::content::BattleContent content;
        addAbility(content, preparation, soma::content::TargetRelation::ownTeam, friendlyEffect);
        addCombatant(content, actingPlayerDefinition, soma::gameplay::Team::players, 30, {preparation});
        addCombatant(content, secondPlayerDefinition, soma::gameplay::Team::players, 10, {});
        addCombatant(content, enemyDefinition, soma::gameplay::Team::enemies, 20, enemyAbilities, std::move(enemyPolicy));
        if (includeSecondEnemy)
        {
            addCombatant(content, soma::gameplay::ContentDefinitionId{"enemy.ally"}, soma::gameplay::Team::enemies, 5, {});
        }
        return content;
    }

    soma::simulation::BattleActionCommand chooseAfterPreparation(soma::content::BattleContent content,
                                                                  FixedRandom& random,
                                                                  const soma::gameplay::AbilityId& preparation,
                                                                  const soma::gameplay::EntityId& preparationTarget,
                                                                  std::vector<soma::simulation::BattleSpawn> spawns)
    {
        soma::simulation::BattleSimulation battle{content, std::move(spawns), random};
        const auto preparationResult = battle.execute({actingPlayer, preparation, preparationTarget});
        check(preparationResult.hasValue());
        const auto command = soma::simulation::EnemyPolicy{random}.choose(battle);
        check(command.hasValue());
        return command.value();
    }

    void doctorHealsAWoundedAlly()
    {
        const soma::gameplay::AbilityId preparation{"ability.player.damage"};
        soma::content::AbilityEffect damage;
        damage.damageFactor = 1.0;
        const soma::gameplay::ContentDefinitionId doctorDefinition{"enemy.doctor"};
        auto content = baseContent(preparation, {}, doctorDefinition, "doctor", {enemyAbility("doctor"), enemyAbility("staff")}, true);
        content.abilities.at(preparation).targeting.relation = soma::content::TargetRelation::opposingTeam;
        content.abilities.at(preparation).friendlyEffect = {};
        content.abilities.at(preparation).hostileEffect = damage;
        addAbility(content, enemyAbility("doctor"), soma::content::TargetRelation::ownTeam);
        addAbility(content, enemyAbility("staff"), soma::content::TargetRelation::opposingTeam);
        FixedRandom random{true};

        const auto command = chooseAfterPreparation(std::move(content),
                                                    random,
                                                    preparation,
                                                    soma::gameplay::EntityId{"enemy.ally.slot5"},
                                                    {{actingPlayerDefinition, 0},
                                                     {secondPlayerDefinition, 1},
                                                     {doctorDefinition, 4},
                                                     {soma::gameplay::ContentDefinitionId{"enemy.ally"}, 5}});

        check(command.ability == enemyAbility("doctor"));
        check(command.primaryTarget == soma::gameplay::EntityId{"enemy.ally.slot5"});
    }

    void knockOutSkipsAnAlreadySleepingPlayer()
    {
        const soma::gameplay::AbilityId preparation{"ability.player.sleep_friend"};
        soma::content::AbilityEffect sleep;
        sleep.sleepProbability = 1.0;
        const soma::gameplay::ContentDefinitionId enemyDefinition{"enemy.sleep"};
        auto content = baseContent(preparation, sleep, enemyDefinition, "sleep", {enemyAbility("knock_out"), enemyAbility("tomahawk")});
        addAbility(content, enemyAbility("knock_out"), soma::content::TargetRelation::opposingTeam);
        addAbility(content, enemyAbility("tomahawk"), soma::content::TargetRelation::opposingTeam);
        FixedRandom random{true};

        const auto command = chooseAfterPreparation(std::move(content),
                                                    random,
                                                    preparation,
                                                    secondPlayer,
                                                    {{actingPlayerDefinition, 0}, {secondPlayerDefinition, 1}, {enemyDefinition, 4}});

        check(command.ability == enemyAbility("knock_out"));
        check(command.primaryTarget == actingPlayer);
    }

    void aimSelectsTheMarkedPlayer()
    {
        const soma::gameplay::AbilityId preparation{"ability.player.mark_friend"};
        soma::content::AbilityEffect mark;
        mark.markRounds = 2;
        const soma::gameplay::ContentDefinitionId enemyDefinition{"enemy.shooter"};
        auto content = baseContent(preparation, mark, enemyDefinition, "miss_shooter", {enemyAbility("aim"), enemyAbility("springfield"),
                                                                                       enemyAbility("miss")});
        addAbility(content, enemyAbility("aim"), soma::content::TargetRelation::opposingTeam);
        addAbility(content, enemyAbility("springfield"), soma::content::TargetRelation::opposingTeam);
        addAbility(content, enemyAbility("miss"), soma::content::TargetRelation::opposingTeam);
        FixedRandom random{false};

        const auto command = chooseAfterPreparation(std::move(content),
                                                    random,
                                                    preparation,
                                                    secondPlayer,
                                                    {{actingPlayerDefinition, 0}, {secondPlayerDefinition, 1}, {enemyDefinition, 4}});

        check(command.ability == enemyAbility("aim"));
        check(command.primaryTarget == secondPlayer);
    }

    void bountySkipsAPlayerWhoAlreadyHasOne()
    {
        const soma::gameplay::AbilityId preparation{"ability.player.bounty_friend"};
        soma::content::AbilityEffect bounty;
        bounty.bountyRounds = 2;
        const soma::gameplay::ContentDefinitionId enemyDefinition{"enemy.deputy"};
        auto content = baseContent(
            preparation, bounty, enemyDefinition, "deputy_support", {enemyAbility("bounty"), enemyAbility("deputy"), enemyAbility("bang")}, true);
        addAbility(content, enemyAbility("bounty"), soma::content::TargetRelation::opposingTeam);
        addAbility(content, enemyAbility("deputy"), soma::content::TargetRelation::ownTeam);
        addAbility(content, enemyAbility("bang"), soma::content::TargetRelation::opposingTeam);
        FixedRandom random{false};

        const auto command = chooseAfterPreparation(std::move(content),
                                                    random,
                                                    preparation,
                                                    secondPlayer,
                                                    {{actingPlayerDefinition, 0},
                                                     {secondPlayerDefinition, 1},
                                                     {enemyDefinition, 4},
                                                     {soma::gameplay::ContentDefinitionId{"enemy.ally"}, 5}});

        check(command.ability == enemyAbility("bounty"));
        check(command.primaryTarget == actingPlayer);
    }

    void crystalBallSelectsTheBuffedPlayer()
    {
        const soma::gameplay::AbilityId preparation{"ability.player.buff_self"};
        soma::content::AbilityEffect buff;
        buff.targetBuff.duration = 2;
        buff.targetBuff.attributes.damageMinimum = 1;
        const soma::gameplay::ContentDefinitionId enemyDefinition{"enemy.psychic"};
        auto content = baseContent(
            preparation, buff, enemyDefinition, "psychic", {enemyAbility("crystal_ball"), enemyAbility("throwing_knives"), enemyAbility("psychic")});
        addAbility(content, enemyAbility("crystal_ball"), soma::content::TargetRelation::opposingTeam);
        addAbility(content, enemyAbility("throwing_knives"), soma::content::TargetRelation::opposingTeam);
        addAbility(content, enemyAbility("psychic"), soma::content::TargetRelation::opposingTeam);
        FixedRandom random{false};

        const auto command = chooseAfterPreparation(std::move(content),
                                                    random,
                                                    preparation,
                                                    actingPlayer,
                                                    {{actingPlayerDefinition, 0}, {secondPlayerDefinition, 1}, {enemyDefinition, 4}});

        check(command.ability == enemyAbility("crystal_ball"));
        check(command.primaryTarget == actingPlayer);
    }

    void fanningSelectsACompleteTwoTargetArc()
    {
        const soma::gameplay::AbilityId preparation{"ability.player.wait"};
        const soma::gameplay::ContentDefinitionId enemyDefinition{"enemy.fanning"};
        auto content = baseContent(
            preparation, {}, enemyDefinition, "fanning", {enemyAbility("fanning"), enemyAbility("bang")});
        addAbility(content, enemyAbility("fanning"), soma::content::TargetRelation::opposingTeam);
        content.abilities.at(enemyAbility("fanning")).targeting.scope = soma::content::TargetScope::count;
        content.abilities.at(enemyAbility("fanning")).targeting.targetCount = 2;
        addAbility(content, enemyAbility("bang"), soma::content::TargetRelation::opposingTeam);
        FixedRandom random{true};

        soma::simulation::BattleSimulation battle{
            content,
            {{actingPlayerDefinition, 0}, {secondPlayerDefinition, 1}, {enemyDefinition, 4}},
            random};
        check(battle.execute({actingPlayer, preparation, actingPlayer}).hasValue());
        const auto command = soma::simulation::EnemyPolicy{random}.choose(battle);
        check(command.hasValue());
        check(command.value().ability == enemyAbility("fanning"));
        check(command.value().primaryTarget == actingPlayer);

        const auto& actor = *battle.find(command.value().actor);
        const auto& primary = *battle.find(command.value().primaryTarget);
        const auto targets = soma::simulation::abilityTargets(
            content.abilities.at(command.value().ability).targeting, actor, primary, battle.combatants());
        check(targets == std::vector{soma::gameplay::EntityId{actingPlayer}, soma::gameplay::EntityId{secondPlayer}});
    }
} // namespace

int main()
{
    doctorHealsAWoundedAlly();
    knockOutSkipsAnAlreadySleepingPlayer();
    aimSelectsTheMarkedPlayer();
    bountySkipsAPlayerWhoAlreadyHasOne();
    crystalBallSelectsTheBuffedPlayer();
    fanningSelectsACompleteTwoTargetArc();
}
