TARGET = akarin

.PHONY: default clean

default:
	gcc -Wall -O2 *.c -o $(TARGET)

clean:
	-rm -f *.o $(TARGET)
