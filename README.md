# Nixly Kalk

Fast, modern calculator for NixlyOS. Standard and scientific mode, real operator
precedence, live result preview, full keyboard control. Text, decimal sign and
digit grouping follow the system locale. The window has a fixed size, so
nixlytile always shows it floating.

## Run

```sh
nix run
```

Develop:

```sh
nix develop
meson setup build && meson compile -C build && meson test -C build
./build/src/nixlykalk
```

## Keyboard

| Key                     | Action                         |
| ----------------------- | ------------------------------ |
| `0-9` `.` `,`           | number                         |
| `+ - * / ^ ( ) ! %`     | operators                      |
| `s c t l g r`           | sin, cos, tan, ln, log, √      |
| `p` `e`                 | π, e                           |
| `Enter` `=`             | evaluate                       |
| `Backspace`             | delete                         |
| `Esc` `Delete`          | clear                          |
| `Alt+1` `Alt+2`         | standard / scientific          |
| `Ctrl+C` `Ctrl+V`       | copy result / paste expression |

## Layout

```
desktop/     .desktop entry and icon
src/math/    expression evaluator and number formatting
src/entry/   editing rules for the expression line
src/text/    locale, UI phrases, display formatting
src/keypad/  keys and the two key layouts
src/window/  window, readout, mode switch, keyboard
tests/       engine tests (meson test)
```
