# qt-exercise-calculator

A desktop calculator built with Qt 6 Widgets. It takes a full expression
rather than one operation at a time, so you can type `5 + 3 * (2 - 1)`
and get the answer with correct operator precedence. Supports the four
binary operators, parentheses, square root, square, sign flip, decimals,
backspace, and both a hard and a soft clear. The keypad works with the
mouse or the keyboard.

Expressions are parsed and evaluated by a header-only shunting-yard
implementation in `expressionevaluator.h`. The window, button wiring and
input handling live in `mainwindow.{h,cpp}`, `mainwindow_inits.cpp` and
`mainwindow_handlers.cpp`.

## Build

Needs Qt 6 (Widgets and Core), CMake 3.16 or newer, and a C++17 compiler.
Note that although `CMakeLists.txt` mentions Qt 5 in its `find_package`
call, only the Qt 6 path actually creates a target, so Qt 5 will not build.

On Arch or CachyOS:

```bash
sudo pacman -S qt6-base cmake
```

On Debian or Ubuntu:

```bash
sudo apt install qt6-base-dev cmake g++
```

Then, from the project root:

```bash
cmake -B build -S .
cmake --build build
```

The binary lands at `build/Calculator`. To produce a .deb instead, run
`cpack -G DEB` from inside `build/`.

## Test

There is no automated test suite. Verification is manual: build, run
`./build/Calculator`, and check a few expressions.

```bash
./build/Calculator
```

Worth checking after any change to the evaluator or the handlers:

- `2 + 3 * 4` gives `14`, not `20`, so precedence holds
- `(2 + 3) * 4` gives `20`, so parentheses hold
- `9` then the root key gives `3`
- `1 / 0` shows an error rather than crashing or printing `inf`
- `Escape` clears everything, `Backspace` deletes one character
- typing on the keyboard matches clicking the equivalent buttons

One known failure to be aware of while testing: a number immediately
followed by `(` is broken. Pressing `5` `(` `3` displays `53` instead of
`3`, because the digit entry buffer is not cleared when the parenthesis
is committed. See the first two TODO items.

## TODO

**Separate the display string from the model.** This is the main one, and
most of the items below are symptoms of it. `m_completeExpression` is
currently three things at once: the text shown to the user, the only
representation of the expression, and the edit buffer. Because the
expression is flat text, several helpers exist purely to re-derive
structure that was thrown away a moment earlier, among them
`isExpressionIncomplete`, `findLastOperatorPosition`,
`findRightmostElement` and `applyUnaryToRightmostElement`. Holding the
expression as a token list and rendering the display string from it would
delete most of that code and make the `5` `(` `3` bug structurally
impossible. Two handlers currently read the expression back out of a
`QLineEdit` and re-parse it; a one-way render would rule that out too.

Other items, roughly in order of how much they matter:

- `addBra()` commits the current value but never clears it, unlike
  `addKet()`. This is the cause of the `5` `(` `3` bug.
- `m_waitingForOperand` is written in 13 places and read in one. It is
  only load-bearing because it masks the `addBra()` omission above, so
  the two have to be fixed together.
- Results round-trip through `QString::number`, which defaults to six
  significant digits, and the truncated text becomes the next operand.
  So `1 / 3` followed by `* 3` gives `0.999999` instead of `1`. Keep the
  result as a double.
- Division guards against `|divisor| < 1e-32`, an arbitrary threshold
  that rejects legitimate small divisors. Nothing checks for overflow
  either, so squaring `1e200` happily displays `inf`. Compare against
  zero and check `std::isfinite` at the single exit point.
- `eventFilter` returns `true` for every non-digit key, which disables
  copy, paste, arrow keys, Home, End, text selection and Tab traversal
  on the display field.
- The operator set is hardcoded in six separate places in
  `expressionevaluator.h`, which defeats the point of the `BINARY_OPS`
  macro in `mainwindow.h`. Adding an operator means editing seven places.
- The digit key lookup does a linear search over a hash's values. An
  array indexed by digit would be direct.
- The `neg` unary function and the `"±"` symbol mapping are dead:
  nothing ever sends that symbol, because `onPlusMinusClicked` does its
  own arithmetic. The precedence entries for `sqrt`, `sqr` and `neg` are
  unreachable for the same kind of reason.
- No licence file. The repository is public, so default copyright
  currently applies.
