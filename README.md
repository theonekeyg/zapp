# Zapp - simple source-to-source compiler

## Overview
Given sequences of expressions and statements, the compiler should either emit
corresponding C code, or execute resulting AST (depending on flags passed to
executable).

Currently `zapp` supports very few statements, beside general expression
operations (if-else conditions, for-in loops). Support for other stuff **will**
be added in near future.

### Examples:

* for-in loops:
```
for i in 0..10 {
  print i
}
```

* if-else conditions:
```
var = 2 + 2 * 2
if (var != 6) {
  print var
} else {
  print 0
}
```

### Credits:
A bunch of design decisions were taken from [this project](https://github.com/rui314/chibicc).
