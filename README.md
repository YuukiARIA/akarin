Akarin - Whitespace Transpiler
==============================

Currently under development.

Akarin is a transpiler from C-like procedural language to Whitespace.

## Build and run

```
make
sudo make install # ofcourse, you can use ./bin/akarin instead without installation

akarin samples/00_hello.txt
akarin samples/00_hello.txt -s
akarin samples/00_hello.txt -p
```

### Output mode

Whitespace code is beautiful but hard to see a little.
Some other (visible) output modes are available.

Symbolic (`-s`) output:

Transpiles into Whitespace but uses S, T and L characters.

```
$ akarin -s samples/00_hello.txt
LSTSLLLLLSSSLSSSSLTLTTSSSTLTLTTSSSSLTTTSSSTLTTTTSSSTLSTSSSTSTSLTLSSSSSSLLTL
```

Pseudo-code (`-p`) output:

Transpiles into pseudo-mnemonic code.
Each informal mnemonic represents each Whitespace instruction.

```
$ akarin -p samples/00_hello.txt
        CALL L0
        HALT
L0:
        PUSH 0
        GETI
        PUSH 1
        GETI
        PUSH 0
        LOAD
        PUSH 1
        LOAD
        ADD
        PUTI
        PUSH 10
        PUTC
        PUSH 0
        RET
```

Syntax tree (`-d`) output:

Not to transpile into Whitespace but to show parsed (abstract) syntax tree.

```
$ akarin -d samples/00_hello.txt
Func
├─Ident main
├─FuncParam
├─Geti-Statement
│ └─Ident a
├─Geti-Statement
│ └─Ident b
├─Puti-Statement
│ └─Binary ADD
│   ├─Variable
│   │ └─Ident a
│   └─Variable
│     └─Ident b
├─Putc-Statement
│ └─Integer 10
└─Return
  └─Integer 0
```
