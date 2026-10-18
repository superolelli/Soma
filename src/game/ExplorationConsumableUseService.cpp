#include "game/ExplorationConsumableUseService.hpp"

#include <algorithm>
#include <ranges>

namespace soma::game
{
    namespace
    {
        content::AttributeDelta attributes(const gameplay::AttributeBonuses& value)
        {
            return {value.armour,
                    value.damageMinimum,
                    value.damageMaximum,
                    value.initiative,
                    value.maximumHealth,
                    value.criticalHitChance,
                    value.precision,
                    value.dodge,
                    value.healing,
                    value.sleepResistance,
                    value.confusionResistance,
                    value.debuffResistance,
                    value.decayResistance};
        }
    } // namespace

    ExplorationConsumableUseService::ExplorationConsumableUseService(EconomyService& economy)
        : economy_(economy)
    {
    }

    gameplay::Result<CampaignChange> ExplorationConsumableUseService::use(const gameplay::ItemDefinition& definition,
                                                                           int& health,
                                                                           int maximumHealth,
                                                                           std::vector<simulation::ActiveBuff>& buffs) const
    {
        const auto* consumable = std::get_if<gameplay::ConsumableItem>(&definition.properties);
        if (consumable == nullptr)
        {
            return gameplay::Result<CampaignChange>::failure("The item is not a consumable");
        }
        auto consumed = economy_.consume(definition);
        if (!consumed.hasValue())
        {
            return consumed;
        }
        health = std::min(maximumHealth, health + consumable->healing);
        if (consumable->duration > 0)
        {
            const auto bonus = attributes(consumable->temporaryBonuses);
            const auto same = [&bonus](const simulation::ActiveBuff& buff) { return buff.attributes == bonus; };
            const auto existing = std::ranges::find_if(buffs, same);
            if (existing == buffs.end())
            {
                buffs.push_back({bonus, consumable->duration, true});
            }
            else
            {
                existing->remainingTurns += consumable->duration;
            }
        }
        return consumed;
    }
} // namespace soma::game
