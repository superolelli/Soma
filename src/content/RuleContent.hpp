#pragma once

#include "content/EconomyContent.hpp"

#include <array>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

namespace soma::content
{
    enum class RuleFragmentType
    {
        trigger,
        subject,
        effect,
        modifier
    };
    enum class RuleMode
    {
        triggered,
        standing
    };
    enum class TriggerKind
    {
        healthLost,
        healthHealed,
        markedReceived,
        decayReceived,
        buffReceived,
        debuffReceived,
        combatantDied,
        confused,
        highDamageDealt,
        turnStarted,
        wokeUp,
        markedOther,
        decayApplied,
        healedOther,
        buffedOther,
        debuffedOther,
        killedOther,
        confusedOther,
        sleepApplied,
        highDamageReceived,
        criticalHitDealt,
        fatigueReachedNine,
        attackDodged,
        decayDamageReceived
    };
    enum class SubjectKind
    {
        thisCombatant,
        allOthers,
        allFriends,
        allOpponents,
        randomCombatant,
        randomOpponent,
        randomFriend,
        allMarked,
        allConfused,
        allSleeping,
        allDecaying,
        allFullHealth,
        allBuffed,
        allDebuffed,
        allBelowNineFatigue,
        allAboveEightFatigue
    };
    enum class EffectKind
    {
        damage,
        healing,
        fatigue,
        applyStatus,
        applyAttribute,
        dynamite
    };
    enum class ModifierKind
    {
        damageReceived,
        damageDealt,
        armour,
        dodge,
        criticalHitChance,
        precision,
        initiative,
        nofaceBuff
    };

    struct TriggerFragment
    {
        TriggerKind kind{};
    };

    struct SubjectFragment
    {
        SubjectKind kind{};
        bool plural{};
    };

    struct EffectFragment
    {
        EffectKind kind{};
        int amount{};
        std::optional<gameplay::StatusId> status;
        int duration{};
        std::optional<ModifierKind> attribute;
    };

    struct ModifierFragment
    {
        ModifierKind kind{};
        int value{};
    };

    using RuleFragmentPayload = std::variant<TriggerFragment, SubjectFragment, EffectFragment, ModifierFragment>;

    struct RuleFragmentSource
    {
        bool enabled{};
        std::vector<std::string> campaigns;
        std::vector<gameplay::LevelId> levels;
        std::vector<gameplay::ContentDefinitionId> enemies;
    };

    struct RuleFragmentAcquisition
    {
        RuleFragmentSource lootables{true};
        RuleFragmentSource battles{true};
    };

    struct RuleFragmentDefinition
    {
        gameplay::RuleFragmentId id;
        std::string name;
        std::string text;
        std::string sentenceTemplate;
        std::string pluralSentenceTemplate;
        RuleFragmentType type{};
        std::array<bool, 2> allowedModes{};
        RuleFragmentPayload payload;
        RuleFragmentAcquisition acquisition;
    };

    struct RuleContent
    {
        struct Presentation
        {
            int panelX{};
            int panelY{};
            int fragmentListX{};
            int fragmentListY{};
            int fragmentListWidth{};
            int fragmentRowHeight{};
            int visibleFragmentRows{};
            int ruleX{};
            int ruleY{};
            int ruleRowSpacing{};
            int partWidth{};
            int partHeight{};
            int partSpacing{};
            int ruleContentWidth{};
            int sigilOffsetX{};
            int sigilOffsetY{};
            int fragmentTextOffsetX{};
            int fragmentTextOffsetY{};
            int fragmentCountOffsetX{};
            int fragmentCountOffsetY{};
            int lockX{};
            int lockY{};
            int lockPriceX{};
            int lockPriceY{};
            int lockPriceWidth{};
            int lockPriceHeight{};
            int closeX{};
            int closeY{};
            int scrollbarX{};
            int scrollbarY{};
            int scrollbarWidth{};
            int scrollbarHeight{};
            int sentenceOffsetY{};
            int confirmationX{};
            int confirmationY{};
            int confirmationMessageY{};
            int confirmationYesX{};
            int confirmationYesY{};
            int confirmationNoX{};
            int confirmationNoY{};
            int titleSize{};
            int textSize{};
            int referenceWidth{};
            int referenceHeight{};
            int confirmationMessageWidth{};
            int confirmationTextSize{};
            int confirmationButtonTextSize{};
        } presentation;

        struct ActiveRulesPresentation
        {
            Point togglePosition;
            Point listPosition;
            Point textInset;
            int textWidth{};
            int textSize{};
            int rowSpacing{};
            std::string emptyText;
        } activeRulesPresentation;

        std::unordered_map<gameplay::RuleFragmentId, RuleFragmentDefinition> fragments;
        std::unordered_map<RuleFragmentType, std::filesystem::path> fragmentSprites;
        std::unordered_map<RuleFragmentType, std::filesystem::path> fragmentSigils;
        std::filesystem::path panel;
        std::filesystem::path fragmentBackground;
        std::filesystem::path slotLock;
        std::filesystem::path closeButton;
        std::filesystem::path confirmationDialog;
        std::filesystem::path confirmationButton;
        std::filesystem::path activeRulesToggle;
        double lootChance{};
        int shopPrice{};
        std::array<int, 3> slotUnlockCosts{};
    };
} // namespace soma::content
