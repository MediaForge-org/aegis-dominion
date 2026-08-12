# Input system

`input::InputSystem` maps semantic `Action` values to rebindable `Binding` records. Bindings contain key plus Control, Shift and Alt modifiers and are selected through one active `Context`: Menu, Gameplay, MapForge or Modal.

Application records mouse press/release/wheel state centrally. UI and modal regions can consume a pointer press or capture the pointer, preventing one click from activating a button and also placing a tower or editing the map. MAP FORGE switches to Modal context while a dialog is open, so gameplay/editor actions cannot leak through.

Main-menu back, gameplay pause/wave/speed/tower/upgrade/sell/target/help and MAP FORGE tool/history/file/grid/snap/fit commands use Actions. Direct keys remain only for text-entry commit/cancel, directional point nudging and low-level modifier-assisted canvas editing; these are editor primitives rather than globally bindable commands.

Future control settings should call `bind(context, action, binding)` and persist those values. Gameplay/screens must not embed replacement shortcut tables.
