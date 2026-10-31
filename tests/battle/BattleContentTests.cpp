#include "content/battle/BattleContentLoader.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <source_location>

namespace
{
    void check(bool condition, const std::source_location location = std::source_location::current())
    {
        if (!condition)
        {
            std::cerr << "Check failed at line " << location.line() << '\n';
            std::exit(1);
        }
    }
} // namespace

int main(int argc, char** argv)
{
    check(argc == 2);
    const auto result = soma::content::BattleContentLoader{}.load(std::filesystem::absolute(argv[1]));
    for (const auto& diagnostic : result.diagnostics)
    {
        std::cerr << diagnostic.message << '\n';
    }
    check(result.valid());
    check(result.content.bossHealthAndDamageMultiplier == 1.5);
    check(result.content.statTooltips.size() == 11);
    check(result.content.statTooltips.contains("armour"));
    check(result.content.statTooltips.contains("decay_resistance"));
    check(result.content.combatants.size() == 18);
    check(!result.content.abilities.empty());
    check(result.content.presentation.size() == 18);
    check(result.content.encounters.size() == 2);
    check(result.content.assets.contains("ability_panel"));
    check(result.content.sounds.contains("critical_hit"));
    check(result.content.sounds.contains("status.decay"));
    check(result.content.sounds.contains("status.dynamite"));
    const auto& simon = result.content.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
    check(!simon.abilities.empty());
    check(simon.attributes.maximumHealth == 25);

    const auto& kutschfahrt =
        result.content.encounters.at(soma::gameplay::ContentDefinitionId{"encounter.kutschfahrt.default"});
    check(kutschfahrt.music.filename() == "Bang_battle.ogg");

    const auto& bigSpencer = result.content.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.big_spencer"});
    check(!bigSpencer.abilities.empty());
    check(bigSpencer.attributes.maximumHealth == 100);

    const auto& markus = result.content.combatants.at(soma::gameplay::ContentDefinitionId{"player.markus"});
    check(markus.marksAttackersForRevenge);

    const auto& fistOfRevenge = result.content.abilities.at(soma::gameplay::AbilityId{"ability.markus.0"});
    check(fistOfRevenge.targeting.requiresRevengeMarker);

    const auto& allAgainstOne = result.content.abilities.at(soma::gameplay::AbilityId{"ability.simon.3"});
    check(allAgainstOne.targeting.excludesActor);

    const auto& indians = result.content.abilities.at(soma::gameplay::AbilityId{"ability.enemy.indians"});
    check(indians.summon.has_value());
    check(indians.summon->combatant == soma::gameplay::ContentDefinitionId{"enemy.indianer"});
    check(indians.summon->count == 3);

    const auto& hardDestruction = result.content.abilities.at(soma::gameplay::AbilityId{"ability.markus.2"});
    check(hardDestruction.targeting.relation == soma::content::TargetRelation::everyoneElse);
    check(hardDestruction.targeting.scope == soma::content::TargetScope::team);
    check(hardDestruction.friendlyEffect.damageFactor == 0.4);
    check(hardDestruction.hostileEffect.damageFactor == 0.4);

    const auto& indomitableHappiness = result.content.abilities.at(soma::gameplay::AbilityId{"ability.anna.0"});
    check(indomitableHappiness.targeting.relation == soma::content::TargetRelation::ownTeam);
    check(indomitableHappiness.targeting.scope == soma::content::TargetScope::team);
    check(indomitableHappiness.fatigueTarget == soma::content::FatigueTarget::actor);

    const auto& randomWordGush = result.content.abilities.at(soma::gameplay::AbilityId{"ability.anna.1.alternative"});
    check(randomWordGush.targeting.scope == soma::content::TargetScope::randomSingle);
    check(randomWordGush.fatigueTarget == soma::content::FatigueTarget::randomPlayer);

    const auto& transfusion = result.content.abilities.at(soma::gameplay::AbilityId{"ability.anna.2.alternative"});
    check(transfusion.hostileEffect.decayDamage == 1);
    check(transfusion.hostileEffect.decayRounds == 3);
    check(transfusion.hostileEffect.transfusionHealing == 1);

    const auto& noseRam = result.content.abilities.at(soma::gameplay::AbilityId{"ability.markus.1"});
    check(noseRam.hostileEffect.transferredMissingHealthDamageFactor == 0.25);
    check(noseRam.hostileEffect.transferredMissingHealthHealingFactor == 0.25);

    const auto& spreadingNoseRam = result.content.abilities.at(soma::gameplay::AbilityId{"ability.markus.1.alternative"});
    check(spreadingNoseRam.hostileEffect.successiveTargetDamageReduction == 0.75);

    const auto& horribleGrin = result.content.abilities.at(soma::gameplay::AbilityId{"ability.markus.3"});
    check(horribleGrin.hostileEffect.panicStacks == 2);

    const auto& horribleGrinAlternative = result.content.abilities.at(soma::gameplay::AbilityId{"ability.markus.3.alternative"});
    check(horribleGrinAlternative.hostileEffect.targetBuff.damagePerNegativeStatus == -0.05);

    const auto& gravediggerGaze = result.content.abilities.at(soma::gameplay::AbilityId{"ability.enemy.gravedigger_gaze"});
    check(gravediggerGaze.fatigueTarget == soma::content::FatigueTarget::targets);

    const auto& psychic = result.content.abilities.at(soma::gameplay::AbilityId{"ability.enemy.psychic"});
    check(psychic.fatigueTarget == soma::content::FatigueTarget::allPlayers);

    const auto& hackIntoSystem = result.content.abilities.at(soma::gameplay::AbilityId{"ability.ole.2"});
    check(hackIntoSystem.targeting.scope == soma::content::TargetScope::count);
    check(hackIntoSystem.targeting.targetCount == 2);
    const auto& criticalGummiArm = result.content.abilities.at(soma::gameplay::AbilityId{"ability.ole.0.alternative"});
    check(criticalGummiArm.hostileEffect.criticalHitModifier == 5);
    const auto& alternativePoetrySlam = result.content.abilities.at(soma::gameplay::AbilityId{"ability.ole.3.alternative"});
    check(alternativePoetrySlam.targeting.scope == soma::content::TargetScope::count);
    check(alternativePoetrySlam.targeting.targetCount == 2);
}
