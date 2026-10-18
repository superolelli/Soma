#include "game/CampaignState.hpp"
#include "game/EconomyService.hpp"
#include "game/ExplorationConsumableUseService.hpp"

#include <cstdlib>
#include <string>

namespace
{
    void check(bool condition)
    {
        if (!condition)
        {
            std::abort();
        }
    }

    soma::gameplay::ItemDefinition equipment(std::string id, int price = 40)
    {
        return {soma::gameplay::ItemId{std::move(id)},
                "Equipment",
                soma::gameplay::ItemCategory::equipment,
                1,
                price,
                soma::gameplay::EquipmentItem{},
                soma::gameplay::AssetId{"icon.equipment"}};
    }

    soma::gameplay::ItemDefinition consumable(std::string id, int price = 10)
    {
        return {soma::gameplay::ItemId{std::move(id)},
                "Consumable",
                soma::gameplay::ItemCategory::consumable,
                1,
                price,
                soma::gameplay::ConsumableItem{},
                soma::gameplay::AssetId{"icon.consumable"}};
    }

    soma::gameplay::ItemDefinition currency(std::string id, soma::gameplay::Currency type)
    {
        return {soma::gameplay::ItemId{std::move(id)},
                "Currency",
                soma::gameplay::ItemCategory::currency,
                1,
                0,
                soma::gameplay::CurrencyItem{type},
                soma::gameplay::AssetId{"icon.currency"}};
    }
} // namespace

int main()
{
    soma::game::CampaignState state;
    soma::game::EconomyService economy{state};
    check(state.currency(soma::gameplay::Currency::cards) == 1000);
    check(state.currency(soma::gameplay::Currency::dice) == 0);

    const auto ironPlate = equipment("item.bang.iron_plate", 80);
    check(economy.purchase(ironPlate, ironPlate.price).hasValue());
    check(state.currency(soma::gameplay::Currency::cards) == 920);
    check(state.inventory().size() == 1);
    check(state.itemUnlocked(ironPlate.id));

    const soma::game::EquipmentLocation location{soma::gameplay::ContentDefinitionId{"player.simon"},
                                                  soma::gameplay::EquipmentSlotId{"equipment.north"}};
    const auto item = state.inventory().front();
    check(economy.equip(location, item).hasValue());
    check(state.inventory().empty());
    check(state.equipped(location) != nullptr);
    check(economy.unequip(location).hasValue());
    check(state.inventory().size() == 1);

    check(economy.sell(ironPlate, state.inventory().front()).hasValue());
    check(state.currency(soma::gameplay::Currency::cards) == 936);
    check(state.inventory().empty());

    for (auto index = 0; index < 5; ++index)
    {
        check(economy.grant(consumable("item.test." + std::to_string(index))).hasValue());
    }
    check(state.distinctConsumables() == 5);
    const auto fullInventory = economy.grant(consumable("item.test.overflow"));
    check(!fullInventory.hasValue());
    check(economy.grant(consumable("item.test.0"), 2).hasValue());
    check(state.consumableCount(soma::gameplay::ItemId{"item.test.0"}) == 3);

    check(economy.grant(currency("currency.dice", soma::gameplay::Currency::dice), 4).hasValue());
    check(!state.itemUnlocked(soma::gameplay::ItemId{"currency.dice"}));
    const soma::gameplay::SkillId skill{"skill.simon.0.1"};
    check(economy.acquireSkill(skill, 3).hasValue());
    check(state.currency(soma::gameplay::Currency::dice) == 1);
    check(state.skillAcquired(skill));
    check(!economy.acquireSkill(skill, 0).hasValue());

    const soma::gameplay::SkillId advancedSkill{"skill.simon.0.2"};
    check(economy.grant(currency("currency.dice", soma::gameplay::Currency::dice), 3).hasValue());
    check(economy.acquireSkill(advancedSkill, 3, std::span{&skill, 1}).hasValue());

    check(state.shopSlotsUnlocked() == 1);
    check(economy.unlockShopSlot().hasValue());
    check(state.shopSlotsUnlocked() == 2);
    check(state.shopDicePrice() == 100);
    check(economy.reduceDicePrice().hasValue());
    check(state.shopDicePrice() == 90);
    const auto cardsBeforeDice = state.currency(soma::gameplay::Currency::cards);
    check(economy.purchaseDice().hasValue());
    check(state.currency(soma::gameplay::Currency::cards) == cardsBeforeDice - 90);

    const soma::game::ShopStockEntry persistentOffer{soma::gameplay::ItemInstance{ironPlate.id, {}}, std::nullopt};
    economy.replaceShopStock({persistentOffer, persistentOffer});
    economy.removeShopStockEntry(persistentOffer);
    check(state.shopStock().size() == 2U);
    check(!state.shopStock().front().item.has_value());
    check(state.shopStock().back() == persistentOffer);

    soma::game::CampaignState swapState;
    soma::game::EconomyService swapEconomy{swapState};
    const auto first = equipment("item.test.first");
    const auto second = equipment("item.test.second");
    check(swapEconomy.grant(first).hasValue());
    check(swapEconomy.grant(second).hasValue());
    check(swapEconomy.equip(location, swapState.inventory().front()).hasValue());
    const auto replacement = swapState.inventory().front();
    check(swapEconomy.equip(location, replacement).hasValue());
    check(swapState.equipped(location)->definition == second.id);
    check(swapState.inventory().size() == 1);
    check(swapState.inventory().front().definition == first.id);

    const soma::game::EquipmentLocation eastLocation{soma::gameplay::ContentDefinitionId{"player.simon"},
                                                      soma::gameplay::EquipmentSlotId{"equipment.east"}};
    check(swapEconomy.equip(eastLocation, swapState.inventory().front()).hasValue());
    check(swapEconomy.swapEquipment(location, eastLocation).hasValue());
    check(swapState.equipped(location)->definition == first.id);
    check(swapState.equipped(eastLocation)->definition == second.id);
    check(swapState.inventory().empty());

    soma::game::CampaignState runState;
    soma::game::EconomyService runEconomy{runState};
    const soma::gameplay::ItemDefinition tonic{soma::gameplay::ItemId{"item.test.tonic"},
                                                "Tonic",
                                                soma::gameplay::ItemCategory::consumable,
                                                1,
                                                10,
                                                soma::gameplay::ConsumableItem{5, {.armour = 4, .precision = 2}, 10},
                                                soma::gameplay::AssetId{"icon.tonic"}};
    check(runEconomy.grant(tonic, 2).hasValue());
    soma::game::ExplorationConsumableUseService runConsumables{runEconomy};
    int health = 7;
    std::vector<soma::simulation::ActiveBuff> buffs;
    check(runConsumables.use(tonic, health, 10, buffs).hasValue());
    check(health == 10);
    check(buffs.size() == 1 && buffs.front().remainingTurns == 10);
    check(runConsumables.use(tonic, health, 10, buffs).hasValue());
    check(buffs.size() == 1 && buffs.front().remainingTurns == 20);
    check(runState.consumableCount(tonic.id) == 0);
}
