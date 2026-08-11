# UI icon glyph fix

## Goal

Remove font-dependent navigation and status symbols from visible UI strings and render them through one reusable SFML geometry component. Preserve the UTF-8 text path for actual language text and make known icon-glyph regressions testable.

## Files in scope

- Shared UI: `src/ui/UiRenderer.hpp`, `src/ui/UiRenderer.cpp`.
- Existing symbol users: `src/screens/MainMenuScreen.cpp`, `src/editor/MapForgeScreen.cpp`.
- Regression coverage: `tests/text_service_tests.cpp`, `CMakeLists.txt` only if the existing test needs the source root.

## Invariants

- German UTF-8 text continues through `TextService` unchanged.
- Button hit areas, actions, layout and game/editor behavior do not change.
- Icons do not depend on font glyph coverage.
- No asset redesign or Phase-3 work is introduced.
- Existing dirty worktree and generated build data remain untouched.

## Implementation steps

1. Add typed UI icons and icon-aware buttons to `UiRenderer`.
2. Draw current and anticipated common icons using SFML geometry.
3. Replace visible arrow, undo/redo and status glyph strings with text plus typed icons.
4. Add a source-level regression check for reserved UI icon glyphs.
5. Run fresh Debug and Release builds plus all CTest targets outside the repository build tree.

## Verification

- Repository audit finds no reserved icon glyphs in visible UI string literals.
- Navigation labels contain only `ZURÜCK`, `WEITER`, `MENÜ`; icons are separate draw calls.
- Debug and Release remain clean under `-Wall -Wextra -Wpedantic`.
- All CTest targets pass.

## Result (2026-08-12)

`UiRenderer` now owns typed, font-independent geometry for navigation, undo/redo, state, play/pause, settings and delete icons. Main-menu map selection, the full tutorial navigation and MAP FORGE no longer embed the affected arrow/status glyphs in font text. A source audit test rejects the known reserved icon codepoints anywhere below `src/` while leaving language text and typographic UTF-8 untouched.

Fresh GCC 16.1.1 Debug and Release builds completed without warnings. Both runs passed all five CTest targets; the suite now reports 44 domain checks, including the new UI-icon regression check. Builds were isolated below `/tmp` to preserve the pre-existing dirty repository build tree and saved map.
