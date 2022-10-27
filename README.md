Akarin - Whitespace Transpiler
==============================

Currently under development.

Akarin is a transpiler from C-like procedural language to Whitespace.

## Build and run

```
make
sudo make install # ofcourse, you can use ./bin/akarin instead without installation

akarin -i samples/00_basic.txt
akarin -i samples/00_basic.txt -s
akarin -i samples/00_basic.txt -p
```

### Input by stdin

Symbolic output:

```
$ echo "{ geti n; if (n < 10) { putc 'O'; putc 'K'; } else { putc 'N'; putc 'G'; } putc '\n' }" | akarin -s
SSSSLTLTTSSSSLTTTSSSTSTSLTSSTLTTTSLSSSSLLSLTTLLSSTSLSSSTLLSSTTLLTSSLSSSTSSTTTTLTLSSSSSTSSTSTTLTLSSLSLTLLSSSLSSSTSSTTTSLTLSSSSSTSSSTTTLTLSSLSSTLSSSSLTLSSSSSTSSSSSLLLL
```

Pseudo-code output:

```
$ echo "{ geti n; if (n < 10) { putc 'O'; putc 'K'; } else { putc 'N'; putc 'G'; } putc '\n' }" | akarin -p
        PUSH 0
        GETI
        PUSH 0
        LOAD
        PUSH 10
        SUB
        JNEG L2
        PUSH 0
        JMP L3
L2:
        PUSH 1
L3:
        JZ L0
        PUSH 79
        PUTC
        PUSH 75
        PUTC
        JMP L1
L0:
        PUSH 78
        PUTC
        PUSH 71
        PUTC
L1:
        PUSH 0
        PUTC
        PUSH 32
        HALT
```
