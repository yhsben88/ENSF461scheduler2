.PHONY: all clean
all: scheduler.out

scheduler.out: scheduler.c
	gcc -o $@ $<

clean:
	rm scheduler.out