# AEGIS DOMINION roadmap

## Completed foundation slice

- Product-facing rename to AEGIS DOMINION.
- Renderer-independent V1 `MapDocument` with controlled version handling.
- Central SFML UTF-8/font service with German glyph verification.
- Regression coverage for map round-trips, malformed/versioned input, validation, editor undo/redo, built-in maps and text conversion.

## Next bounded phase

Extract screen/state orchestration and build the first usable MAP FORGE vertical slice: editor screen, camera/grid, path/spawn/goal editing, validation, save/load and direct play-test while preserving the current match.

## Later phases

1. Extend `MapDocument` compatibly for terrain, elevation, environment, zones, decorations and wave sets.
2. Establish reusable UI widgets and typed input actions.
3. Replace the legacy asset container with stable IDs, fallbacks and structured logging.
4. Separate gameplay simulation from SFML render views, then introduce data-driven combat, waves, difficulty and economy.
5. Add progression, codex/arsenal, statistics and savegames.
6. Profile, pool high-volume objects and polish presentation before expanding content breadth.

This roadmap describes direction, not completed features.
