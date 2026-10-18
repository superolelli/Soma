#pragma once

#include "content/RuleContent.hpp"
#include "game/CampaignState.hpp"
#include "gameplay/common/Result.hpp"

namespace soma::game
{
    enum class RuleChange { fragmentGranted, fragmentAssigned, fragmentRemoved, slotUnlocked };

    /// Mutates the persistent rule-fragment collection and enforces valid rule construction.
    class RuleBuilderService
    {
      public:
        RuleBuilderService(CampaignState& state, const content::RuleContent& content);

        [[nodiscard]] gameplay::Result<RuleChange> grant(const gameplay::RuleFragmentId& fragment, int quantity = 1);
        [[nodiscard]] gameplay::Result<RuleChange> purchase(const gameplay::RuleFragmentId& fragment);
        [[nodiscard]] gameplay::Result<RuleChange> assign(std::size_t slot, const gameplay::RuleFragmentId& fragment);
        [[nodiscard]] gameplay::Result<RuleChange> remove(std::size_t slot, content::RuleFragmentType type);
        [[nodiscard]] gameplay::Result<RuleChange> unlockNextSlot();
        [[nodiscard]] std::optional<content::RuleMode> mode(std::size_t slot) const;
        [[nodiscard]] bool accepts(std::size_t slot, const gameplay::RuleFragmentId& fragment) const;
        [[nodiscard]] int availableCount(const gameplay::RuleFragmentId& fragment) const;
        [[nodiscard]] bool typeDiscovered(content::RuleFragmentType type) const;

      private:
        [[nodiscard]] const content::RuleFragmentDefinition* definition(const gameplay::RuleFragmentId& id) const;
        [[nodiscard]] static std::optional<gameplay::RuleFragmentId>& field(ComposedRule& rule, content::RuleFragmentType type);
        [[nodiscard]] static const std::optional<gameplay::RuleFragmentId>& field(const ComposedRule& rule,
                                                                                 content::RuleFragmentType type);
        [[nodiscard]] bool validCombination(const ComposedRule& rule) const;

        CampaignState& state_;
        const content::RuleContent& content_;
    };
} // namespace soma::game
