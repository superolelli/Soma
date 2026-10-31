#pragma once
#include <compare>
#include <functional>
#include <stdexcept>
#include <string>

namespace soma::gameplay
{
    /// Gives content identifiers distinct compile-time types while retaining their JSON string form.
    template <typename Tag>
    class StrongId
    {
      public:
        StrongId() = default;

        explicit StrongId(std::string value)
            : value_(std::move(value))
        {
            if (value_.empty())
            {
                throw std::invalid_argument("A content ID cannot be empty");
            }
        }

        [[nodiscard]] const std::string& value() const noexcept
        {
            return value_;
        }

        [[nodiscard]] bool empty() const noexcept
        {
            return value_.empty();
        }

        auto operator<=>(const StrongId&) const = default;

      private:
        std::string value_;
    };
    struct ContentDefinitionTag;
    struct AssetTag;
    struct EntityTag;
    struct RuleTag;
    struct RuleFragmentTag;
    struct ItemTag;
    struct EnemyTag;
    struct AbilityTag;
    struct EncounterTag;
    struct LevelTag;
    struct LootableTag;
    struct StatusTag;
    struct EquipmentSlotTag;
    struct SkillTag;
    using ContentDefinitionId = StrongId<ContentDefinitionTag>;
    using AssetId = StrongId<AssetTag>;
    using EntityId = StrongId<EntityTag>;
    using RuleId = StrongId<RuleTag>;
    using RuleFragmentId = StrongId<RuleFragmentTag>;
    using ItemId = StrongId<ItemTag>;
    using EnemyId = StrongId<EnemyTag>;
    using AbilityId = StrongId<AbilityTag>;
    using EncounterId = StrongId<EncounterTag>;
    using LevelId = StrongId<LevelTag>;
    using LootableId = StrongId<LootableTag>;
    using StatusId = StrongId<StatusTag>;
    using EquipmentSlotId = StrongId<EquipmentSlotTag>;
    using SkillId = StrongId<SkillTag>;
} // namespace soma::gameplay

template <typename Tag>
struct std::hash<soma::gameplay::StrongId<Tag>>
{
    std::size_t operator()(const soma::gameplay::StrongId<Tag>& id) const noexcept
    {
        return std::hash<std::string>{}(id.value());
    }
};
