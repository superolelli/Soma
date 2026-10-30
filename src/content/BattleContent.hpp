#pragma once

#include "gameplay/combat/CombatTypes.hpp"
#include "gameplay/items/ItemTypes.hpp"

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace soma::content
{
    struct AttributeDelta
    {
        int armour{};
        int damageMinimum{};
        int damageMaximum{};
        int initiative{};
        int maximumHealth{};
        int criticalHitChance{};
        int precision{};
        int dodge{};
        int healing{};
        int sleepResistance{};
        int confusionResistance{};
        int debuffResistance{};
        int decayResistance{};

        auto operator<=>(const AttributeDelta&) const = default;
    };

    struct BuffDefinition
    {
        int duration{};
        bool positive{true};
        AttributeDelta attributes;
        double healingReceived{};
        double damageAgainstMarked{};
        double damagePerNegativeStatus{};
        bool damageFromFatigue{};
        bool blocksFatigueStageOne{};
        bool blocksFatigueStageTwo{};
    };

    struct AbilityEffect
    {
        int criticalHitModifier{};
        double lessTargetsMoreDamage{};
        double damageFactor{};
        double damageAgainstMarked{};
        double damageAgainstRevengeMarked{};
        double successiveTargetDamageReduction{};
        double transferredMissingHealthDamageFactor{};
        double transferredMissingHealthHealingFactor{};
        int heal{};
        int healSelf{};
        int targetFatigue{};
        double confusionProbability{};
        int confusionRounds{};
        int decayDamage{};
        int decayRounds{};
        int decayDamageAgainstMarked{};
        int decayRoundsAgainstConfused{};
        int decayRoundsAgainstSleeping{};
        bool removeDecayAndHealParty{};
        int markRounds{};
        double sleepProbability{};
        bool removeBuffs{};
        bool removeDebuffs{};
        bool stealBuffs{};
        int addMiss{};
        int addMissSelf{};
        bool dynamite{};
        int bountyRounds{};
        int riposteRounds{};
        int riposteSelfRounds{};
        int rageRounds{};
        int hackedRounds{};
        int transfusionHealing{};
        int panicStacks{};
        int panicAgainstMarked{};
        int spreadPanicTargets{};
        bool damageFromPanic{};
        bool doublePanic{};
        bool moveTargetsRandomly{};
        bool ignoreArmour{};
        bool ignoreDecayResistance{};
        bool inflictAllDecayAndRemove{};
        bool transferDecayOnKill{};
        double randomNegativeStatusProbability{};
        int randomNegativeStatusDuration{2};
        int prolongNegativeEffects{};
        bool increaseSleepProbabilityByFatigue{};
        int sleepFailureDecayDamage{};
        int sleepFailureDecayRounds{};
        BuffDefinition targetBuff;
        BuffDefinition actorBuff;
    };

    enum class TargetRelation
    {
        self,
        ownTeam,
        opposingTeam,
        eitherTeam,
        everyoneElse
    };

    enum class TargetScope
    {
        single,
        randomSingle,
        count,
        team
    };

    struct Targeting
    {
        TargetRelation relation{TargetRelation::opposingTeam};
        TargetScope scope{TargetScope::single};
        int targetCount{1};
        bool requiresRevengeMarker{};
        bool excludesActor{};
    };

    struct SummonDefinition
    {
        gameplay::ContentDefinitionId combatant;
        int count{};
        std::vector<int> allowedSlots;
    };

    enum class FatigueTarget
    {
        actor,
        targets,
        allPlayers,
        randomPlayer
    };

    struct AbilityDefinition
    {
        gameplay::AbilityId id;
        std::string name;
        std::string animation;
        std::string friendlyEffectAnimation{"empty"};
        std::string hostileEffectAnimation{"empty"};
        Targeting targeting;
        AbilityEffect friendlyEffect;
        AbilityEffect hostileEffect;
        int precisionModifier{};
        int fatigue{};
        FatigueTarget fatigueTarget{FatigueTarget::actor};
        BuffDefinition allCombatantsBuff;
        std::optional<SummonDefinition> summon;
    };

    struct CombatantDefinition
    {
        gameplay::ContentDefinitionId id;
        std::string name;
        gameplay::Team team{};
        gameplay::CombatantAttributes attributes;
        AttributeDelta resistances;
        std::vector<gameplay::AbilityId> abilities;
        std::string aiPolicy;
        bool marksAttackersForRevenge{};
        bool addsMissOnHighDamage{};
        bool healsOnPass{};
        int encounterCost{};
        std::vector<gameplay::EquippedBattleEffect> equipmentEffects;
    };

    struct CombatantPresentationDefinition
    {
        gameplay::ContentDefinitionId id;
        int preferredSlot{};
        std::filesystem::path model;
        std::string entity;
        double scaleAdjustment{};
        std::unordered_map<gameplay::AbilityId, std::filesystem::path> abilityIcons;
    };

    struct BattleLayoutDefinition
    {
        int referenceWidth{1920};
        int referenceHeight{1080};
        double normalScale{0.6};
        double abilityScale{0.8};
        int groundY{750};
        int playerStartX{100};
        int playerSpacing{160};
        int enemyStartFromViewCenter{20};
        int enemySpacing{40};
        int abilityStageY{800};
        int abilityAttackerXPlayer{480};
        int abilityTargetXPlayer{1200};
        int abilityGroupPadding{50};
        int abilityPanelX{100};
        int abilityPanelY{860};
        int abilityButtonStartX{140};
        int abilityButtonY{892};
        int abilityButtonSpacing{132};
        int informationPanelX{820};
        int informationPanelY{850};
        int announcementX{1060};
        int announcementY{150};
        int announcementTextX{1110};
        int announcementTextY{168};
        int battleResultX{820};
        int battleResultY{420};
        int turnOrderNumberOffsetY{5};
        int turnOrderNumberCharacterSize{20};
        int expectedAttackCenterX{1080};
        int expectedAttackSingleY{1015};
        int expectedAttackMultipleY{1003};
        int expectedAttackLineSpacing{22};
        int expectedAttackTextSize{16};
        double announcementDuration{1.5};
        double executionTimeout{5.0};
        double reactionDuration{0.8};
        double resultDuration{1.0};
        double wakeResultDuration{2.0};
        double defeatDuration{2.0};
        double itemSpeechDuration{2.5};
        int itemSpeechMaximumWidth{380};
        int itemSpeechCharacterSize{22};
        int itemSpeechPaddingX{56};
        int itemSpeechPaddingY{38};
        int itemSpeechSpeakerOffsetX{-10};
        int itemSpeechSpeakerOffsetY{-10};
        int itemSpeechTextVerticalOffset{-17};
    };

    struct ItemActivationPresentation
    {
        std::string speech;
        std::string notification;
    };

    struct BattleContent
    {
        struct Encounter
        {
            gameplay::ContentDefinitionId id;
            std::vector<std::pair<gameplay::ContentDefinitionId, int>> combatants;
            std::filesystem::path background;
            std::filesystem::path music;
        };

        std::unordered_map<gameplay::ContentDefinitionId, CombatantDefinition> combatants;
        std::unordered_map<gameplay::AbilityId, AbilityDefinition> abilities;
        std::unordered_map<gameplay::ContentDefinitionId, CombatantPresentationDefinition> presentation;
        std::unordered_map<gameplay::ContentDefinitionId, Encounter> encounters;
        std::unordered_map<std::string, std::filesystem::path> assets;
        std::unordered_map<std::string, std::filesystem::path> sounds;
        std::unordered_map<std::string, std::string> statTooltips;
        std::unordered_map<gameplay::ItemId, ItemActivationPresentation> itemActivations;
        double bossAttributeMultiplier{1.0};
        BattleLayoutDefinition layout;
    };
} // namespace soma::content
