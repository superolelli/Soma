# Soma JSON content

`v1/` is the only runtime content tree. Its typed catalogs are loaded and cross-validated at startup; JSON is the canonical source for game definitions and presentation layout.

The complete file map, examples, and procedures for adding items, enemies, abilities, skills, encounters, levels, lootables, and rule fragments are documented in [Content authoring](../../docs/CONTENT_AUTHORING.md).

Validate the complete tree after every edit:

```powershell
cmake --build build --config Debug --target validate_content
```
