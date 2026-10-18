# Content authoring

Soma loads all game definitions from `Data/content/v1`. JSON is the canonical source: do not add parallel registries in C++ when an existing content type can express the change.

## File map

| File | Owns |
| --- | --- |
| `battle/content.json` | Combatants, abilities, and direct battle encounters |
| `battle/presentation.json` | Combatant models, ability icons, battle assets, sounds, layout, and playback timing |
| `economy/items.json` | Equipment, consumables, currencies, rarity, prices, and diamond bonuses |
| `economy/skills.json` | Skills, upgrades, ability versions, and five-skill trees |
| `economy/presentation.json` | Shared economy assets and inventory/skill/shop layout |
| `world/content.json` | Campaign assets, generated levels, world encounters, loot tables, lootables, rewards, and exploration layout |
| `rules/content.json` | Rule fragments, acquisition constraints, rule economy, sprites, and rule-panel layout |
| `flow/presentation.json` | Startup, main menu, main room, pause menu, toolbar, doors, music, and cursors |

All IDs are stable strings. Prefer the existing namespaces: `player.*`, `enemy.*`, `ability.*`, `skill.*`, `item.*`, `encounter.*`, `level.*`, `lootable.*`, `loot.*`, `reward.*`, and `rule.*`. Changing an ID is a breaking content change because saves and other definitions reference it.

## Validation workflow

1. Copy the nearest existing definition of the same kind.
2. Give it a unique stable ID and update every referencing catalog.
3. Add referenced assets under `Data` and use repository-relative paths with forward slashes in JSON.
4. Validate and run the most relevant direct launcher.

```powershell
cmake --build build --config Debug --target validate_content
ctest --test-dir build -C Debug --output-on-failure
```

The validator checks all typed catalogs, files, IDs, cross-catalog ability sets, player ability slots, and item battle-effect presentation. Loader errors include the catalog and offending ID where possible.

## Items

Add items to `economy/items.json`. Every item has `id`, `name`, `category`, `level`, `price`, `icon`, and `campaigns`.

- `level` is rarity, not a progression gate: `1` = gewöhnlich (70%), `2` = selten (25%), `3` = episch (5%).
- `campaigns` controls the worlds in which the item may be generated.
- `category` is `equipment`, `consumable`, or `currency`.
- Color is assigned to an equipment instance when it is generated; it is not part of the definition.

Equipment example:

```json
{
  "id": "item.bang.iron_plate",
  "name": "Eisenplatte",
  "category": "equipment",
  "level": 2,
  "price": 80,
  "icon": "Data/Sprites/Items/Bang/iron_plate.png",
  "campaigns": ["bang"],
  "bonuses": { "armour": 10 },
  "miss_on_high_damage": false,
  "heal_on_pass": false
}
```

Supported bonus keys follow the existing definitions: health, minimum/maximum damage, armour, critical chance, dodge, initiative, precision, healing, and status resistances. Special equipment may define its typed battle effect; such an item also needs a matching entry in `battle/presentation.json` under `item_activations` for its speech and notification.

Consumables define the immediate exploration effect, such as healing or fatigue change. Currency items map to `cards` or `dice`. Add new icons directly by path in the item definition; shared panel textures belong in `economy/presentation.json`.

## Enemies

An enemy needs a combat definition and a presentation definition.

In `battle/content.json`:

```json
{
  "id": "enemy.example",
  "team": "enemies",
  "name": "Beispielgegner",
  "abilities": ["ability.enemy.example.attack"],
  "attributes": {
    "maximum_health": 30,
    "damage_minimum": 4,
    "damage_maximum": 7,
    "armour": 0,
    "critical_hit_chance": 5,
    "dodge": 3,
    "initiative": 4,
    "precision": 5
  },
  "resistances": { "sleep": 0, "confusion": 0, "debuff": 0, "decay": 0 },
  "encounter_cost": 3,
  "ai_policy": "default"
}
```

`encounter_cost` is used by procedural level budgets. `ai_policy` selects an implemented decision policy; use `default` unless the enemy needs a distinct tactical sequence. Content-specific AI behavior that cannot be described by existing ability data requires a new policy implementation and tests.

In `battle/presentation.json`, add the model, Spriter entity, preferred slot, scale adjustment, and icons for every ability the combatant can expose. The Spriter entity should provide `bounding_box` or `hitbox`; `ability_effect_point` is optional because effects fall back to the current visual bounds.

## Abilities and targeting

Abilities live in `battle/content.json`. Their tooltip is generated from mechanics; do not author a separate formatted tooltip.

```json
{
  "id": "ability.enemy.example.attack",
  "data": {
    "name": "Beispielangriff",
    "animation": "attack",
    "effectAnimation": "hit",
    "fatigue": 0,
    "fatigue_target": "actor",
    "effect": { "damageFactor": 1.0 },
    "targeting": { "relation": "opposing_team", "scope": "single", "count": 1 }
  }
}
```

Target relations are `self`, `own_team`, `opposing_team`, `either_team`, and `everyone_else`. Scopes are `single`, `count`, and `team`. Formation positions never restrict an ability. Multi-target selection wraps around the four logical slots and counts empty slots.

Effects can combine direct damage/healing, fatigue, buffs/debuffs, mark, confusion, sleep, decay, dynamite, hacked, bounty, rage, riposte, panic, transfusion, cleansing, status extension, summoning, and the specialized mechanics already represented in existing definitions. Reuse the exact property spelling from a working neighboring ability; the loader rejects unsupported combinations and invalid references.

## Player ability versions and skills

Each player has four logical ability slots. Each slot has two mutually exclusive versions, and each version has exactly five skills. Mechanics belong to `battle/content.json`; version/skill assignments belong to `economy/skills.json`.

```json
{
  "player": "player.simon",
  "slot": 0,
  "versions": [
    {
      "ability": "ability.simon.0",
      "skills": ["skill.simon.0.1", "skill.simon.0.2", "skill.simon.0.3", "skill.simon.0.4", "skill.simon.0.5"]
    },
    {
      "ability": "ability.simon.0.alternative",
      "skills": ["skill.simon.0.alternative.1", "skill.simon.0.alternative.2", "skill.simon.0.alternative.3",
                 "skill.simon.0.alternative.4", "skill.simon.0.alternative.5"]
    }
  ]
}
```

A skill defines its player, ability, tree slot, plain German title/description, dice cost, prerequisites, icon, and structured modifier. Formatting and status-keyword highlighting are generated automatically. A modifier may change precision, fatigue, target count, targeting, or merge additional friendly/hostile/self effects into the ability. Duration extensions apply to the complete combined status/buff.

When adding a player ability version, also add its battle-panel icon mapping in `battle/presentation.json`.

## Encounters and levels

World encounters in `world/content.json` list enemy IDs, whether the encounter is a boss, and its reward table:

```json
{
  "id": "encounter.bang.example",
  "combatants": ["enemy.example", "enemy.example"],
  "boss": false,
  "reward_table": "reward.bang.example"
}
```

Generated levels specify their campaign, difficulty, room count, encounter budget, lootable probability, backgrounds, eligible regular encounters, and one boss encounter. Generation draws a target budget within the inclusive range, selects encounters whose enemy costs exactly satisfy it, distributes them across available rooms, and reserves the last room for the authored boss. Loot placement is independent.

```json
{
  "id": "level.bang.example",
  "campaign": "bang",
  "difficulty": 2,
  "room_count": 6,
  "encounter_budget": { "minimum": 8, "maximum": 12 },
  "lootable_probability": 0.4,
  "room_backgrounds": ["background.bang_gang_1", "background.bang_gang_2"],
  "end_background": "background.bang_gang_ende",
  "encounters": ["encounter.bang.example"],
  "boss_encounter": "encounter.bang.example.boss"
}
```

Every integer in the budget range must be achievable with the eligible encounters and available non-boss rooms; validation reports impossible ranges.

Boss video/music presentation is attached through the corresponding battle encounter and campaign assets. Add the video file under `Data/Videos` and reference it using the same pattern as existing bosses.

## Loot tables and lootables

Loot tables in `world/content.json` contain weighted/repeated item and currency choices. A lootable references one table plus asset IDs for its two-state sprite and opening sound. Its sprite contains the full and looted states side by side; `hitbox` and `position` are exploration-space values.

To add a lootable:

1. Register its sprite and sound in the world `assets` object.
2. Add or reuse a loot table.
3. Add the lootable definition with campaign eligibility, table, sprite/sound IDs, hitbox, and position.
4. Include its campaign in the relevant level generation content.

Rule fragments are rolled independently using their own global probability and acquisition constraints.

## Rule fragments

Rule fragments live in `rules/content.json`. Adding a fragment that uses an existing mechanic requires no C++ or UI change.

Every fragment has a stable `id`, tooltip `name`, compact `text`, German `sentence_template`, `type`, `allowed_modes`, and mechanic-specific fields. Effects may also provide `plural_sentence_template`.

The types are:

- `trigger`: observes an event and binds its combatant.
- `subject`: selects affected combatants.
- `effect`: performs a one-time state change in a triggered rule.
- `modifier`: changes a queried value while a standing rule is active.

Optional acquisition constraints limit discovery:

```json
"acquisition": {
  "lootables": { "campaigns": ["bang"], "levels": ["level.bang.3"] },
  "battles": { "levels": ["level.bang.3"], "enemies": ["enemy.hilfssheriff"] }
}
```

Omitting `acquisition` allows both sources everywhere. Within one source all non-empty constraints must match. Trigger-relative subjects (`this_combatant`, friends/opponents, and random relative selectors) are for triggered rules; absolute predicates such as all marked/sleeping/full-health combatants can also support standing rules. `allowed_modes` is authoritative.

Use `Soma.exe --rules` to preview the complete fragment library.

## Presentation and assets

Coordinates, scales, hitboxes, spacings, font sizes, playback delays, and asset paths that tune a screen belong in its presentation JSON. Gameplay formulas and invariants belong in simulation/application code. This distinction keeps art iteration data-driven without hiding core rules in layout files.

- Battle positions, HUD anchors, sounds, and action timing: `battle/presentation.json`
- Inventory, skill, shop, and shared counters: `economy/presentation.json`
- Exploration, loot, rewards, speech bubbles, and campaign presentation: `world/content.json`
- Main menu, main room, toolbar, loading, and pause settings: `flow/presentation.json`
- Rule builder and active-rule drawer art/layout: `rules/content.json`

Spriter animation names referenced by abilities must exist on the selected entity. Spriter soundlines are played automatically and obey the global sound-effects volume. Non-Spriter one-shot cues are registered by their presentation/application sound player.

Keep source and JSON files UTF-8. German text can contain `ä`, `ö`, `ü`, and `ß` directly.
