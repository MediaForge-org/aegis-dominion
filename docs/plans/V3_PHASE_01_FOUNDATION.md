# V3 Phase 01 – foundation

## Goal
Stop growing the monolithic SFML prototype and establish a data-driven architecture that can support a powerful Map Builder and a later 3D renderer.

## Deliverables
- Renderer-independent `MapDocument` with serialization and validation.
- Editor model with tools, save/load and undo/redo foundations.
- Existing built-in maps represented as `.aegismap` data and loaded by `GameMap` with a compatibility fallback.
- UTF-8 text path fixed in the SFML UI.
- Codex repo instructions and architecture documentation.
- Core serialization test.

## Follow-up
Phase 02 adds an actual graphical Map Builder screen. Phase 03 extracts rendering interfaces and starts the new asset pipeline.
