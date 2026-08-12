# UI system

AEGIS DOMINION renders UI in a 1600×900 reference coordinate system. `Application` updates an aspect-preserving SFML view on resize; wide or tall windows letterbox instead of stretching. `UiScale` exposes the same calculation for tests and future settings integration.

`ui::Theme::command()` is the single tactical command-interface theme. It includes semantic colors, five spacing tokens, button/panel/border/icon sizes and body/caption/heading/title/display typography. Screens should select semantic roles instead of duplicating RGB values for new shipping UI.

`UiRenderer` provides panels/cards, labels/wrapped text, separators, buttons/icon buttons, progress bars, tooltips, toggles, sliders, dropdown shells, scroll areas, modal dialogs, toasts and tab bars. Existing immediate-mode click routing remains compatible while `ButtonInteraction` supplies testable Normal, Hover, Pressed, Selected, Disabled and Focused logic for new components. `LinearLayout` supports horizontal/vertical fixed and flex allocation with gap and padding.

`UiIcon` is geometry-based and font-independent. Do not reintroduce Unicode arrows, undo/redo, play/pause, settings, delete, check/close or modified markers into text. Real texture icons can later implement the same typed icon calls.

The migrated surfaces are the main menu, map selection, all tutorial pages, the gameplay command panel/cards and MAP FORGE toolbar, tool palette, inspector shell, status bar and dialogs. These form a consistent foundation, not the final authored art pass.
