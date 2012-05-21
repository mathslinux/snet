all: test
test: snet_ia.c snet_tcp.c snet.h
	gcc -o test snet_ia.c snet_tcp.c -g -Wall -lpthread
clean:
	rm -rf test
