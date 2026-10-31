#include "game/EconomyQueries.hpp"

#include <algorithm>
#include <tuple>

namespace soma::game
{
    namespace
    {
        template <typename Entry>
        void sortEntries(std::vector<Entry>& entries, ItemSort order)
        {
            if (order == ItemSort::name)
            {
                std::ranges::sort(entries, {}, &Entry::name);
                return;
            }
            std::ranges::sort(entries, [](const auto& left, const auto& right) {
                return std::tie(left.item.color.red, left.item.color.green, left.item.color.blue, left.name) <
                       std::tie(right.item.color.red, right.item.color.green, right.item.color.blue, right.name);
            });
        }
    } // namespace

    EconomyQueries::EconomyQueries(const CampaignState& state, const content::EconomyContent& content)
        : state_(state), content_(content)
    {
    }

    std::vector<InventoryEntry> EconomyQueries::inventory(ItemSort order) const
    {
        std::vector<InventoryEntry> result;
        result.reserve(state_.inventory().size());
        for (const auto& item : state_.inventory())
        {
            const auto& definition = content_.items.at(item.definition);
            result.push_back({item, definition.name, definition.icon, definition.price / 5});
        }
        sortEntries(result, order);
        return result;
    }

    std::vector<ConsumableEntry> EconomyQueries::consumables(ItemSort order) const
    {
        std::vector<ConsumableEntry> result;
        for (const auto& [id, definition] : content_.items)
        {
            const auto quantity = state_.consumableCount(id);
            if (definition.category == gameplay::ItemCategory::consumable && quantity > 0)
            {
                result.push_back({id, definition.name, definition.icon, quantity, definition.price / 5});
            }
        }
        std::ranges::sort(result, {}, &ConsumableEntry::name);
        if (order == ItemSort::color)
        {
            // Consumables have no per-instance color, so a stable name order keeps the row predictable.
        }
        return result;
    }

    const content::AbilitySetDefinition* EconomyQueries::abilitySet(const gameplay::ContentDefinitionId& player,
                                                                     std::size_t abilitySlot) const
    {
        const auto found = std::ranges::find_if(content_.abilitySets, [&](const auto& set) {
            return set.player == player && set.slot == abilitySlot;
        });
        return found == content_.abilitySets.end() ? nullptr : &*found;
    }

    std::vector<SkillEntry> EconomyQueries::skills(const gameplay::ContentDefinitionId& player,
                                                    std::size_t abilitySlot,
                                                    std::size_t version) const
    {
        std::vector<SkillEntry> result;
        const auto* set = abilitySet(player, abilitySlot);
        if (set == nullptr || version >= set->versions.size()) return result;

        for (const auto& id : set->versions.at(version).skills)
        {
            const auto& skill = content_.skills.at(id);
            result.push_back({id, skill.name, skill.description, skill.icon, skill.slot, skill.cost, skillAvailability(skill)});
        }
        return result;
    }

    SkillAvailability EconomyQueries::skillAvailability(const content::SkillDefinition& skill) const
    {
        if (state_.skillAcquired(skill.id))
        {
            return SkillAvailability::acquired;
        }
        const auto prerequisitesMet = std::ranges::all_of(
            skill.prerequisites, [this](const auto& prerequisite) { return state_.skillAcquired(prerequisite); });
        return prerequisitesMet ? SkillAvailability::available : SkillAvailability::locked;
    }
} // namespace soma::game
