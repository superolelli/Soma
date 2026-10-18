#include "simulation/battle/EnemyPolicy.hpp"

#include <algorithm>
#include <ranges>

namespace soma::simulation
{
    namespace
    {
        gameplay::AbilityId enemyAbility(const std::string& name)
        {
            return gameplay::AbilityId{"ability.enemy." + name};
        }

        bool woundedByAtLeast(const CombatantState& combatant, int amount)
        {
            return combatant.health <= combatant.attributes.maximumHealth - amount;
        }
    } // namespace

    EnemyPolicy::EnemyPolicy(gameplay::IRandom& random)
        : random_(random)
    {
    }

    gameplay::Result<BattleActionCommand> EnemyPolicy::choose(const BattleSimulation& battle) const
    {
        if (battle.finished())
        {
            return gameplay::Result<BattleActionCommand>::failure("The battle is finished");
        }
        const auto* actor = battle.find(battle.currentActor());
        if (actor == nullptr || actor->team != gameplay::Team::enemies)
        {
            return gameplay::Result<BattleActionCommand>::failure("The current actor is not an enemy");
        }
        const auto ability = chooseAbility(battle, *actor);
        const auto target = chooseTarget(battle, *actor, ability);
        if (!target.has_value())
        {
            return gameplay::Result<BattleActionCommand>::failure("The enemy policy found no valid target");
        }
        return gameplay::Result<BattleActionCommand>::success({actor->id, ability, *target});
    }

    gameplay::AbilityId EnemyPolicy::chooseAbility(const BattleSimulation& battle, const CombatantState& actor) const
    {
        if (const auto ordinaryAbility = chooseOrdinaryEnemyAbility(battle, actor); ordinaryAbility.has_value())
        {
            return *ordinaryAbility;
        }
        if (const auto bossAbility = chooseBangBossAbility(battle, actor); bossAbility.has_value())
        {
            return *bossAbility;
        }
        return chooseKutschfahrtAbility(battle, actor);
    }

    std::optional<gameplay::AbilityId> EnemyPolicy::chooseOrdinaryEnemyAbility(const BattleSimulation& battle, const CombatantState& actor) const
    {
        const auto& id = battle.content().combatants.at(actor.definition).aiPolicy;
        const auto coin = [this] {
            return random_.between(0, 1) == 0;
        };

        if (id == "miss_shooter")
        {
            return actor.misses >= 2 || coin() ? enemyAbility("springfield") : enemyAbility("miss");
        }
        if (id == "fanning")
        {
            const auto canFan = std::ranges::any_of(battle.combatants(), [](const auto& target) {
                return target.team == gameplay::Team::players && target.alive() && target.slot > 0;
            });
            return coin() || !canFan ? enemyAbility("bang") : enemyAbility("fanning");
        }
        if (id == "sleep")
        {
            const auto awakePlayer = std::ranges::any_of(battle.combatants(), [](const auto& target) {
                return target.team == gameplay::Team::players && target.alive() && !target.sleeping;
            });
            return !awakePlayer || coin() ? enemyAbility("tomahawk") : enemyAbility("knock_out");
        }
        if (id == "deputy_support")
        {
            const auto otherAlly = std::ranges::any_of(battle.combatants(), [&actor](const auto& ally) {
                return ally.team == gameplay::Team::enemies && ally.alive() && ally.id != actor.id;
            });
            const auto unmarkedPlayer = std::ranges::any_of(battle.combatants(), [](const auto& player) {
                return player.team == gameplay::Team::players && player.alive() && player.bountyTurns == 0;
            });
            if (!otherAlly)
            {
                return enemyAbility("bang");
            }
            return unmarkedPlayer && coin() ? enemyAbility("bounty") : enemyAbility("deputy");
        }
        return std::nullopt;
    }

    std::optional<gameplay::AbilityId> EnemyPolicy::chooseBangBossAbility(const BattleSimulation& battle, const CombatantState& actor) const
    {
        const auto& id = battle.content().combatants.at(actor.definition).aiPolicy;
        const auto coin = [this] {
            return random_.between(0, 1) == 0;
        };

        if (id == "gravedigger")
        {
            if (battle.enemyDiedLastRound() && actor.health < actor.attributes.maximumHealth)
            {
                return enemyAbility("bury_the_dead");
            }
            return coin() ? enemyAbility("shovel_hit") : enemyAbility("gravedigger_gaze");
        }
        if (id == "apache_kid")
        {
            const auto allies = std::ranges::count_if(battle.combatants(), [](const auto& ally) {
                return ally.team == gameplay::Team::enemies && ally.alive();
            });
            if (allies <= 2)
            {
                return enemyAbility("indians");
            }
            return coin() ? enemyAbility("knife") : enemyAbility("no_pain");
        }
        if (id == "big_spencer")
        {
            const auto playerBuffed = std::ranges::any_of(battle.combatants(), [](const auto& target) {
                return target.team == gameplay::Team::players && target.alive() && !target.buffs.empty();
            });
            const auto dynamitePossible = std::ranges::none_of(battle.combatants(), [](const auto& target) {
                return target.dynamite;
            });
            if (playerBuffed)
            {
                return dynamitePossible && coin() ? enemyAbility("dynamite") : enemyAbility("brawl");
            }
            return dynamitePossible && coin() ? enemyAbility("dynamite") : enemyAbility("punch");
        }
        if (id == "tequila_joe")
        {
            const auto selfWounded = woundedByAtLeast(actor, 8);
            const auto allyWounded = std::ranges::any_of(battle.combatants(), [&actor](const auto& ally) {
                return ally.team == gameplay::Team::enemies && ally.alive() && ally.id != actor.id && woundedByAtLeast(ally, 3);
            });
            if (selfWounded && allyWounded)
            {
                const auto choice = random_.between(0, 2);
                return choice == 0 ? enemyAbility("jabbering") : choice == 1 ? enemyAbility("beer") : enemyAbility("spend_a_round");
            }
            if (selfWounded)
            {
                return coin() ? enemyAbility("jabbering") : enemyAbility("beer");
            }
            if (allyWounded)
            {
                return coin() ? enemyAbility("jabbering") : enemyAbility("spend_a_round");
            }
            return enemyAbility("jabbering");
        }
        if (id == "bill_noface")
        {
            const std::array abilities{enemyAbility("shotgun"), enemyAbility("panic"), enemyAbility("rattlesnake")};
            return abilities.at(static_cast<std::size_t>(random_.between(0, 2)));
        }
        return std::nullopt;
    }

    gameplay::AbilityId EnemyPolicy::chooseKutschfahrtAbility(const BattleSimulation& battle, const CombatantState& actor) const
    {
        const auto& id = battle.content().combatants.at(actor.definition).aiPolicy;
        const auto coin = [this] {
            return random_.between(0, 1) == 0;
        };

        if (id == "duelist")
        {
            return coin() ? enemyAbility("duelpistol") : enemyAbility("thug");
        }
        if (id == "spy")
        {
            return coin() ? enemyAbility("distraction") : enemyAbility("spy");
        }
        if (id == "doctor")
        {
            const auto allyWounded = std::ranges::any_of(battle.combatants(), [](const auto& ally) {
                return ally.team == gameplay::Team::enemies && ally.alive() && woundedByAtLeast(ally, 6);
            });
            return allyWounded && random_.between(0, 4) != 0 ? enemyAbility("doctor") : enemyAbility("staff");
        }
        if (id == "psychic")
        {
            const auto playerBuffed = std::ranges::any_of(battle.combatants(), [](const auto& target) {
                return target.team == gameplay::Team::players && target.alive() && !target.buffs.empty();
            });
            if (playerBuffed)
            {
                const auto choice = random_.between(0, 5);
                return choice < 4 ? enemyAbility("crystal_ball") : choice == 4 ? enemyAbility("throwing_knives") : enemyAbility("psychic");
            }
            return coin() ? enemyAbility("psychic") : enemyAbility("throwing_knives");
        }
        const std::array abilities{enemyAbility("whip"), enemyAbility("gloves"), enemyAbility("grandmaster")};
        return abilities.at(static_cast<std::size_t>(random_.between(0, 2)));
    }

    std::optional<gameplay::EntityId> EnemyPolicy::chooseTarget(const BattleSimulation& battle,
                                                                const CombatantState& actor,
                                                                const gameplay::AbilityId& ability) const
    {
        auto targets = battle.validPrimaryTargets(ability);
        if (targets.empty())
        {
            return std::nullopt;
        }
        if (ability == enemyAbility("doctor"))
        {
            const auto wounded = std::ranges::find_if(targets, [&battle](const auto& id) {
                return woundedByAtLeast(*battle.find(id), 6);
            });
            if (wounded != targets.end())
            {
                return *wounded;
            }
        }
        if (ability == enemyAbility("knock_out"))
        {
            std::erase_if(targets, [&battle](const auto& id) {
                return battle.find(id)->sleeping;
            });
        }
        if (ability == enemyAbility("bounty"))
        {
            std::erase_if(targets, [&battle](const auto& id) {
                return battle.find(id)->bountyTurns > 0;
            });
        }
        if (ability == enemyAbility("crystal_ball"))
        {
            std::erase_if(targets, [&battle](const auto& id) {
                return battle.find(id)->buffs.empty();
            });
        }
        if (targets.empty())
        {
            return actor.id;
        }
        for (const auto& target : targets)
        {
            if (battle.find(target)->markedTurns > 0 && random_.between(0, 9) < 8)
            {
                return target;
            }
        }
        return targets.at(static_cast<std::size_t>(random_.between(0, static_cast<int>(targets.size()) - 1)));
    }
} // namespace soma::simulation
