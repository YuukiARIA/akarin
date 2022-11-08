Akarin - Whitespace Transpiler
==============================

Currently under development.

Akarin is a transpiler from C-like procedural language to Whitespace.

## Build and run

```
make
sudo make install # ofcourse, you can use ./bin/akarin instead without installation

akarin samples/00_sample.txt
akarin samples/00_sample.txt -s
akarin samples/00_sample.txt -p
```

### Output mode

Symbolic output:

```
$ akarin -s samples/00_sample.txt
LSTSLLLLLSSSLSSSSLTLTTSSSSLTTTSSSTSTSLTSSTLTTTTLSSSSLLSLTSSLLSSTTLSSSTLLSSTSSLLTSTLSSSTSSTTTTLTLSSSSSTSSTSTTLTLSSLSLTSLLSSTLSSSTSSTTTSLTLSSSSSTSSSTTTLTLSSLSSTSLSSSTSTSLTLSSLTL
```

Pseudo-code output:

```
$ akarin -p samples/00_sample.txt
        CALL L0
        HALT
L0:
        PUSH 0
        GETI
        PUSH 0
        LOAD
        PUSH 10
        SUB
        JNEG L3
        PUSH 0
        JMP L4
L3:
        PUSH 1
L4:
        JZ L1
        PUSH 79
        PUTC
        PUSH 75
        PUTC
        JMP L2
L1:
        PUSH 78
        PUTC
        PUSH 71
        PUTC
L2:
        PUSH 10
        PUTC
        RET
```
