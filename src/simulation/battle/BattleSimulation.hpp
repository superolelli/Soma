#pragma once

#include "content/BattleContent.hpp"
#include "content/RuleContent.hpp"
#include "gameplay/combat/Fatigue.hpp"
#include "gameplay/common/Result.hpp"
#include "gameplay/events/GameplayEvent.hpp"
#include "gameplay/items/ItemTypes.hpp"
#include "gameplay/random/IRandom.hpp"
#include "gameplay/rules/ComposedRule.hpp"
#include "simulation/battle/BattleCommand.hpp"
#include "simulation/battle/BattleState.hpp"

#include <optional>
#include <unordered_set>
#include <vector>

namespace soma::simulation
{
    /// Owns authoritative combat state and resolves commands into ordered gameplay events.
    class BattleSimulation
    {
      public:
        BattleSimulation(const content::BattleContent& content,
                         std::vector<BattleSpawn> combatants,
                         gameplay::IRandom& random,
                         const content::RuleContent* rules = nullptr,
                         const gameplay::ComposedRules* activeRules = nullptr);

        [[nodiscard]] gameplay::Result<std::vector<gameplay::GameplayEvent>> execute(const BattleActionCommand& command);
        [[nodiscard]] std::vector<gameplay::GameplayEvent> begin();
        [[nodiscard]] gameplay::Result<std::vector<gameplay::GameplayEvent>> passTurn(const gameplay::EntityId& actor);
        [[nodiscard]] gameplay::Result<std::vector<gameplay::GameplayEvent>> useConsumable(const gameplay::EntityId& target,
                                                                                           const gameplay::ConsumableItem& consumable);
        [[nodiscard]] std::vector<gameplay::EntityId> validPrimaryTargets(const gameplay::AbilityId& ability) const;
        [[nodiscard]] const CombatantState* find(const gameplay::EntityId& id) const;
        [[nodiscard]] const std::vector<CombatantState>& combatants() const noexcept;
        [[nodiscard]] std::vector<gameplay::EntityId> initiativeOrder() const;
        [[nodiscard]] std::vector<gameplay::EntityId> remainingInitiativeOrder() const;
        [[nodiscard]] const gameplay::EntityId& currentActor() const;
        [[nodiscard]] gameplay::CombatantAttributes effectiveAttributes(const CombatantState& combatant) const;
        [[nodiscard]] int modifiedDamage(const CombatantState& source, const CombatantState& target, int baseDamage) const;
        [[nodiscard]] bool finished() const noexcept;
        [[nodiscard]] std::optional<gameplay::Team> winner() const noexcept;
        [[nodiscard]] const content::BattleContent& content() const noexcept;
        [[nodiscard]] bool enemyDiedLastRound() const noexcept;
        [[nodiscard]] const std::unordered_set<gameplay::AbilityId>& usedPlayerAbilities() const noexcept;

      private:
        [[nodiscard]] CombatantState* findMutable(const gameplay::EntityId& id);
        [[nodiscard]] const content::CombatantDefinition& definition(const CombatantState& combatant) const;
        [[nodiscard]] std::vector<CombatantState*> expandTargets(CombatantState& actor,
                                                                 const content::AbilityDefinition& ability,
                                                                 CombatantState& primaryTarget);
        void applyAbilityToTargets(CombatantState& actor,
                                   const content::AbilityDefinition& ability,
                                   const std::vector<CombatantState*>& targets,
                                   std::vector<gameplay::GameplayEvent>& events);
        void moveTargetsRandomly(const std::vector<CombatantState*>& targets);
        [[nodiscard]] CombatantState* nextLivingTeamMember(const CombatantState& combatant);
        void resolveRiposte(CombatantState& counteringCombatant, CombatantState& attacker, std::vector<gameplay::GameplayEvent>& events);
        void resolveEquipmentEffects(std::vector<gameplay::GameplayEvent>& events);
        void applyUnavoidableDamage(CombatantState& source, CombatantState& target, int damage, std::vector<gameplay::GameplayEvent>& events);
        void recordDamageApplied(const gameplay::EntityId& source,
                                 CombatantState& target,
                                 int damage,
                                 gameplay::DamageKind kind,
                                 std::vector<gameplay::GameplayEvent>& events);
        [[nodiscard]] bool attackIsDodged(CombatantState& actor,
                                          CombatantState& target,
                                          const content::AbilityDefinition& ability,
                                          std::vector<gameplay::GameplayEvent>& events);
        void summonCombatants(const content::SummonDefinition& summon, std::vector<gameplay::GameplayEvent>& events);
        void applyEffect(CombatantState& actor,
                         CombatantState& target,
                         const content::AbilityEffect& effect,
                         double additionalDamage,
                         double damageFactorMultiplier,
                         std::vector<gameplay::GameplayEvent>& events);
        void applyDamageEffect(CombatantState& actor,
                               CombatantState& target,
                               const content::AbilityEffect& effect,
                               double additionalDamage,
                               double damageFactorMultiplier,
                               int effectFactor,
                               std::vector<gameplay::GameplayEvent>& events);
        void applyHealingEffects(CombatantState& actor,
                                 CombatantState& target,
                                 const content::AbilityEffect& effect,
                                 int effectFactor,
                                 std::vector<gameplay::GameplayEvent>& events);
        void applyStatusEffects(CombatantState& actor,
                                CombatantState& target,
                                const content::AbilityEffect& effect,
                                int durationBonus,
                                std::vector<gameplay::GameplayEvent>& events);
        [[nodiscard]] bool tryApplyDecay(
            CombatantState& target, int damage, int duration, bool ignoreResistance, std::vector<gameplay::GameplayEvent>& events);
        void applyRandomNegativeStatus(CombatantState& target,
                                       const content::AbilityEffect& effect,
                                       int durationBonus,
                                       std::vector<gameplay::GameplayEvent>& events);
        void prolongNegativeEffects(CombatantState& target, int rounds, std::vector<gameplay::GameplayEvent>& events);
        void inflictRemainingDecay(CombatantState& actor, CombatantState& target, std::vector<gameplay::GameplayEvent>& events);
        void transferDecayAfterDefeat(CombatantState& actor, CombatantState& defeated, std::vector<gameplay::GameplayEvent>& events);
        void triggerTransfusions(CombatantState& source, std::vector<gameplay::GameplayEvent>& events);
        void expireTransfusionsWhenDecayEnds(CombatantState& target, std::vector<gameplay::GameplayEvent>& events);
        void removeDecayAndHealLivingPlayers(CombatantState& actor, CombatantState& target, std::vector<gameplay::GameplayEvent>& events);
        void applyUtilityEffects(CombatantState& actor,
                                 CombatantState& target,
                                 const content::AbilityEffect& effect,
                                 int durationBonus,
                                 std::vector<gameplay::GameplayEvent>& events);
        void applyBuff(CombatantState& target,
                       const content::BuffDefinition& buff,
                       int duration,
                       std::vector<gameplay::GameplayEvent>& events,
                       bool ignoresHacked = false);
        void applyHealing(const gameplay::EntityId& source, CombatantState& target, int baseHealing, std::vector<gameplay::GameplayEvent>& events);
        void changePanic(CombatantState& target, int amount, std::vector<gameplay::GameplayEvent>& events);
        void recordStatusGained(CombatantState& target, const gameplay::StatusId& status, std::vector<gameplay::GameplayEvent>& events);
        [[nodiscard]] int negativeStatusCategoryCount(const CombatantState& target) const;
        [[nodiscard]] int modifiedHealingReceived(const CombatantState& target, int baseHealing) const;
        [[nodiscard]] double damageAgainstMarkedBonus(const CombatantState& source) const;
        void addAttributes(CombatantState& target, const content::AttributeDelta& attributes, int sign);
        void removeAllBuffs(CombatantState& target, std::vector<gameplay::GameplayEvent>& events);
        void removeAllDebuffs(CombatantState& target, std::vector<gameplay::GameplayEvent>& events);
        void updateStatuses(CombatantState& combatant, std::vector<gameplay::GameplayEvent>& events);
        void rebuildTurnOrder();
        void advanceTurn(std::vector<gameplay::GameplayEvent>& events);
        void advanceToPlayableTurn(std::vector<gameplay::GameplayEvent>& events);
        void updateFinished(std::vector<gameplay::GameplayEvent>& events);
        void emitDefeatIfNeeded(CombatantState& target, std::vector<gameplay::GameplayEvent>& events);
        void updateNofaceBuff(CombatantState& target);
        void changeFatigue(CombatantState& target, int amount, std::vector<gameplay::GameplayEvent>& events);
        void updateFatiguePenalties(CombatantState& target);
        void applyAbilityFatigue(CombatantState& actor,
                                 const content::AbilityDefinition& ability,
                                 const std::vector<CombatantState*>& targets,
                                 std::vector<gameplay::GameplayEvent>& events);
        void resolveTriggeredRules(std::vector<gameplay::GameplayEvent>& events, const gameplay::EntityId& source, std::size_t firstEvent);
        [[nodiscard]] bool standingSubjectMatches(content::SubjectKind subject, const CombatantState& target) const;
        [[nodiscard]] int standingModifiedValue(content::ModifierKind modifier, const CombatantState& target, int baseValue) const;
        [[nodiscard]] std::optional<gameplay::EntityId> triggeredBy(const content::TriggerFragment& trigger,
                                                                    const gameplay::GameplayEvent& event,
                                                                    const gameplay::EntityId& source) const;
        [[nodiscard]] std::vector<CombatantState*> ruleTargets(content::SubjectKind subject, const gameplay::EntityId& trigger);
        void applyRuleEffect(const gameplay::EntityId& source,
                             CombatantState& target,
                             const content::EffectFragment& effect,
                             std::vector<gameplay::GameplayEvent>& events);

        const content::BattleContent& content_;
        gameplay::IRandom& random_;
        std::vector<CombatantState> combatants_;
        std::vector<gameplay::EntityId> turnOrder_;
        std::size_t turnIndex_{};
        std::unordered_set<gameplay::EntityId> actedThisRound_;
        bool finished_{};
        bool begun_{};
        std::optional<gameplay::Team> winner_;
        int nextSummonNumber_{};
        int roundsSinceEnemyDefeat_{-1};
        std::unordered_set<gameplay::AbilityId> usedPlayerAbilities_;
        std::unordered_set<std::string> activatedEquipmentEffects_;
        const content::RuleContent* rules_{};
        const gameplay::ComposedRules* activeRules_{};
    };
} // namespace soma::simulation
