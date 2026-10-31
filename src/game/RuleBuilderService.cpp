#include "game/RuleBuilderService.hpp"

#include <algorithm>

namespace soma::game
{
    RuleBuilderService::RuleBuilderService(CampaignState& state, const content::RuleContent& content) : state_(state), content_(content) {}

    gameplay::Result<RuleChange> RuleBuilderService::grant(const gameplay::RuleFragmentId& fragment, int quantity)
    {
        if (!definition(fragment)) return gameplay::Result<RuleChange>::failure("Unknown rule fragment");
        if (quantity <= 0) return gameplay::Result<RuleChange>::failure("Rule fragment quantity must be positive");
        state_.ruleFragments_[fragment] += quantity;
        return gameplay::Result<RuleChange>::success(RuleChange::fragmentGranted);
    }

    gameplay::Result<RuleChange> RuleBuilderService::purchase(const gameplay::RuleFragmentId& fragment)
    {
        const auto* source = definition(fragment);
        if (!source) return gameplay::Result<RuleChange>::failure("Unknown rule fragment");
        if (!typeDiscovered(source->type)) return gameplay::Result<RuleChange>::failure("This fragment type has not been discovered");
        auto& cards = state_.currencies_[gameplay::Currency::cards];
        if (cards < content_.shopPrice) return gameplay::Result<RuleChange>::failure("Not enough cards");
        cards -= content_.shopPrice;
        ++state_.ruleFragments_[fragment];
        return gameplay::Result<RuleChange>::success(RuleChange::fragmentGranted);
    }

    gameplay::Result<RuleChange> RuleBuilderService::assign(std::size_t slot, const gameplay::RuleFragmentId& fragment)
    {
        if (slot >= static_cast<std::size_t>(state_.ruleSlotsUnlocked_))
            return gameplay::Result<RuleChange>::failure("Rule slot is locked");
        const auto* source = definition(fragment);
        if (!source) return gameplay::Result<RuleChange>::failure("Unknown rule fragment");
        auto candidate = state_.rules_.at(slot);
        auto& destination = field(candidate, source->type);
        const auto replacingSame = destination && *destination == fragment;
        if (!replacingSame && availableCount(fragment) <= 0)
            return gameplay::Result<RuleChange>::failure("No unused copy of this rule fragment is available");
        destination = fragment;
        if (!validCombination(candidate)) return gameplay::Result<RuleChange>::failure("This fragment is incompatible with the rule");
        state_.rules_.at(slot) = std::move(candidate);
        return gameplay::Result<RuleChange>::success(RuleChange::fragmentAssigned);
    }

    gameplay::Result<RuleChange> RuleBuilderService::remove(std::size_t slot, content::RuleFragmentType type)
    {
        if (slot >= static_cast<std::size_t>(state_.ruleSlotsUnlocked_))
            return gameplay::Result<RuleChange>::failure("Rule slot is locked");
        auto& destination = field(state_.rules_.at(slot), type);
        if (!destination) return gameplay::Result<RuleChange>::failure("Rule part is already empty");
        destination.reset();
        return gameplay::Result<RuleChange>::success(RuleChange::fragmentRemoved);
    }

    gameplay::Result<RuleChange> RuleBuilderService::unlockNextSlot()
    {
        if (state_.ruleSlotsUnlocked_ >= static_cast<int>(gameplay::maximumRuleSlots))
            return gameplay::Result<RuleChange>::failure("All rule slots are already unlocked");
        const auto cost = content_.slotUnlockCosts.at(static_cast<std::size_t>(state_.ruleSlotsUnlocked_ - 1));
        auto& cards = state_.currencies_[gameplay::Currency::cards];
        if (cards < cost) return gameplay::Result<RuleChange>::failure("Not enough cards");
        cards -= cost;
        ++state_.ruleSlotsUnlocked_;
        return gameplay::Result<RuleChange>::success(RuleChange::slotUnlocked);
    }

    std::optional<content::RuleMode> RuleBuilderService::mode(std::size_t slot) const
    {
        if (slot >= static_cast<std::size_t>(state_.ruleSlotsUnlocked_)) return std::nullopt;
        bool triggered = true;
        bool standing = true;
        const auto& rule = state_.rules_.at(slot);
        for (const auto& id : {rule.trigger, rule.subject, rule.effect, rule.modifier})
        {
            if (!id) continue;
            const auto* fragment = definition(*id);
            triggered = triggered && fragment->allowedModes.supports(content::RuleMode::triggered);
            standing = standing && fragment->allowedModes.supports(content::RuleMode::standing);
        }
        if (triggered == standing) return std::nullopt;
        return triggered ? content::RuleMode::triggered : content::RuleMode::standing;
    }

    bool RuleBuilderService::accepts(std::size_t slot, const gameplay::RuleFragmentId& fragment) const
    {
        if (slot >= static_cast<std::size_t>(state_.ruleSlotsUnlocked_)) return false;
        const auto* source = definition(fragment);
        if (!source) return false;
        auto candidate = state_.rules_.at(slot);
        const auto& current = field(candidate, source->type);
        if ((!current || *current != fragment) && availableCount(fragment) <= 0) return false;
        field(candidate, source->type) = fragment;
        return validCombination(candidate);
    }

    int RuleBuilderService::availableCount(const gameplay::RuleFragmentId& fragment) const
    {
        auto result = state_.ruleFragmentCount(fragment);
        for (const auto& rule : state_.rules_)
            for (const auto& id : {rule.trigger, rule.subject, rule.effect, rule.modifier})
                if (id && *id == fragment) --result;
        return result;
    }

    bool RuleBuilderService::typeDiscovered(content::RuleFragmentType type) const
    {
        return std::ranges::any_of(state_.ruleFragments_, [&](const auto& entry) {
            const auto* fragment = definition(entry.first);
            return entry.second > 0 && fragment && fragment->type == type;
        });
    }

    const content::RuleFragmentDefinition* RuleBuilderService::definition(const gameplay::RuleFragmentId& id) const
    {
        const auto found = content_.fragments.find(id);
        return found == content_.fragments.end() ? nullptr : &found->second;
    }

    std::optional<gameplay::RuleFragmentId>& RuleBuilderService::field(gameplay::ComposedRule& rule,
                                                                       content::RuleFragmentType type)
    {
        switch (type)
        {
        case content::RuleFragmentType::trigger: return rule.trigger;
        case content::RuleFragmentType::subject: return rule.subject;
        case content::RuleFragmentType::effect: return rule.effect;
        case content::RuleFragmentType::modifier: return rule.modifier;
        }
        return rule.trigger;
    }

    const std::optional<gameplay::RuleFragmentId>& RuleBuilderService::field(const gameplay::ComposedRule& rule,
                                                                             content::RuleFragmentType type)
    {
        return field(const_cast<gameplay::ComposedRule&>(rule), type);
    }

    bool RuleBuilderService::validCombination(const gameplay::ComposedRule& rule) const
    {
        bool triggered = true;
        bool standing = true;
        for (const auto& id : {rule.trigger, rule.subject, rule.effect, rule.modifier})
        {
            if (!id) continue;
            const auto* fragment = definition(*id);
            if (!fragment) return false;
            triggered = triggered && fragment->allowedModes.supports(content::RuleMode::triggered);
            standing = standing && fragment->allowedModes.supports(content::RuleMode::standing);
        }
        if (!triggered && !standing) return false;
        if (triggered && rule.modifier) return false;
        if (standing && (rule.trigger || rule.effect)) return false;
        return true;
    }
} // namespace soma::game
