.PHONY: clean

prog: main.c
	gcc main.c -o prog

clean: 
	rm -f main prog