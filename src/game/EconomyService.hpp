#pragma once

#include "game/CampaignState.hpp"
#include "gameplay/common/Result.hpp"

#include <span>

namespace soma::game
{
    enum class CampaignChange
    {
        itemAdded,
        itemRemoved,
        itemEquipped,
        itemUnequipped,
        itemPurchased,
        itemSold,
        skillAcquired,
        dicePurchased,
        shopSlotUnlocked,
        dicePriceReduced,
        itemConsumed,
        abilityVersionSelected
    };

    /// Applies validated inventory, equipment, purchase, sale, currency, and skill changes.
    class EconomyService
    {
      public:
        explicit EconomyService(CampaignState& state);

        [[nodiscard]] gameplay::Result<CampaignChange> grant(const gameplay::ItemDefinition& definition,
                                                              int quantity = 1,
                                                              gameplay::ItemColor color = {});
        [[nodiscard]] gameplay::Result<CampaignChange> purchase(const gameplay::ItemDefinition& definition,
                                                                 int price,
                                                                 gameplay::ItemColor color = {});
        [[nodiscard]] gameplay::Result<CampaignChange> sell(const gameplay::ItemDefinition& definition,
                                                             const gameplay::ItemInstance& item,
                                                             int quantity = 1);
        [[nodiscard]] gameplay::Result<CampaignChange> equip(const EquipmentLocation& location,
                                                              const gameplay::ItemInstance& item);
        [[nodiscard]] gameplay::Result<CampaignChange> unequip(const EquipmentLocation& location);
        [[nodiscard]] gameplay::Result<CampaignChange> swapEquipment(const EquipmentLocation& first,
                                                                      const EquipmentLocation& second);
        [[nodiscard]] gameplay::Result<CampaignChange> acquireSkill(const gameplay::SkillId& skill,
                                                                     int dicePrice,
                                                                     std::span<const gameplay::SkillId> prerequisites = {});
        [[nodiscard]] gameplay::Result<CampaignChange> selectAbilityVersion(const gameplay::ContentDefinitionId& player,
                                                                             std::size_t abilitySlot,
                                                                             std::size_t version);
        [[nodiscard]] gameplay::Result<CampaignChange> purchaseDice();
        [[nodiscard]] gameplay::Result<CampaignChange> unlockShopSlot();
        [[nodiscard]] gameplay::Result<CampaignChange> reduceDicePrice();
        [[nodiscard]] gameplay::Result<CampaignChange> consume(const gameplay::ItemDefinition& definition, int quantity = 1);
        void replaceShopStock(std::vector<ShopStockEntry> stock);
        void removeShopStockEntry(const ShopStockEntry& entry);
        void clearShopStock() noexcept;

        [[nodiscard]] static constexpr int maximumShopSlotCount() noexcept { return maximumShopSlots; }
        [[nodiscard]] static constexpr int shopUpgradePrice() noexcept { return shopUpgradeCost; }
        [[nodiscard]] static constexpr int minimumDicePurchasePrice() noexcept { return minimumDicePrice; }
        [[nodiscard]] static constexpr int dicePurchasePriceReduction() noexcept { return dicePriceReduction; }

      private:
        static constexpr std::size_t maximumDistinctConsumables = 5;
        static constexpr int maximumShopSlots = 5;
        static constexpr int shopUpgradeCost = 200;
        static constexpr int minimumDicePrice = 50;
        static constexpr int dicePriceReduction = 10;
        CampaignState& state_;
    };
} // namespace soma::game
