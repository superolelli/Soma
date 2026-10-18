# Command-line options

Run all examples from the repository root. Replace `Debug` with `Release` for a release build.

## Normal game

```powershell
.\build\bin\Debug\Soma.exe
.\build\bin\Debug\Soma.exe --menu
.\build\bin\Debug\Soma.exe --savegame <path>
.\build\bin\Debug\Soma.exe --save-game <path>
```

- No arguments and `--menu` open the save-slot menu.
- `--savegame`/`--save-game` bypasses the menu, loads exactly one save file, and writes subsequent saves back to it.
- `Data/Saves/developer.json` is a development save containing all current skills, many items and rule fragments, unlocked rule/shop slots, and ample currency.

Example:

```powershell
.\build\bin\Debug\Soma.exe --savegame .\Data\Saves\developer.json
```

## Exploration

```powershell
.\build\bin\Debug\Soma.exe --explore [level-id] [--seed <integer>] [--level-layout <path>]
```

- The default level is `level.bang.1`.
- `--seed` reproduces procedural room, encounter, and loot choices. Without it, the launcher uses a fresh random seed.
- `--level-layout` replaces generated rooms with an authored layout while retaining the selected level's content and campaign rules.

Examples:

```powershell
.\build\bin\Debug\Soma.exe --explore level.kutschfahrt.1
.\build\bin\Debug\Soma.exe --explore level.bang.3 --seed 1234
.\build\bin\Debug\Soma.exe --explore level.bang.5 --level-layout .\docs\examples\level-layout.json
```

Available levels are defined in `Data/content/v1/world/content.json`. Currently shipped IDs are `level.bang.1` through `level.bang.6` and `level.kutschfahrt.1` through `level.kutschfahrt.2`.

## Battle

Any invocation that does not match a top-level mode is interpreted as a battle launch. The following flags may be combined:

```text
--encounter <id>             Use a battle encounter definition (default: encounter.bang.default).
--enemies <id> [id ...]      Override the encounter with one to four explicit enemies.
--seed <integer>             Reproduce random battle decisions and results.
--auto                       Let the launcher select and execute actions automatically.
--auto-ability <ability-id>  Prefer one ability during automatic play; also enables --auto.
--exit-on-finish             Close the application as soon as the battle resolves.
```

Examples:

```powershell
.\build\bin\Debug\Soma.exe --encounter encounter.bang.default
.\build\bin\Debug\Soma.exe --enemies enemy.greg enemy.apache_kid enemy.big_spencer
.\build\bin\Debug\Soma.exe --enemies enemy.abtruenniger enemy.abtruenniger enemy.abtruenniger --auto --seed 42 --exit-on-finish
```

PowerShell does not require underscores to be escaped. The parser also accepts the historical `\_` spelling and normalizes it to `_`.

## Direct panel launchers

```powershell
.\build\bin\Debug\Soma.exe --skills
.\build\bin\Debug\Soma.exe --inventory
.\build\bin\Debug\Soma.exe --shop
.\build\bin\Debug\Soma.exe --rules
```

These modes create temporary preview state and do not load or modify a save file:

- `--skills` grants preview dice and opens the skill panel.
- `--inventory` grants a representative equipment selection.
- `--shop` grants representative inventory and discovered rule-fragment types.
- `--rules` grants two copies of every configured rule fragment.

## Authored level layout

An authored layout contains room content IDs and optional parallax overrides:

```json
{
  "rooms": [
    { "background": "background.bang_gang_1", "lootable": "lootable.chest" },
    { "background": "background.bang_gang_ende", "encounter": "encounter.bang.5.boss", "boss": true }
  ],
  "parallax": {
    "second_layer_tiles": ["parallax.bang.second.1", "parallax.bang.second.2"],
    "third_layer_tiles": ["parallax.bang.third.1", "parallax.bang.third.2"],
    "fourth_layer": "parallax.bang.fourth",
    "second_layer_divisor": 3,
    "third_layer_divisor": 6,
    "fourth_layer_divisor": 9
  }
}
```

Every referenced ID must exist in the regular world catalog. See [the checked-in example](examples/level-layout.json).
