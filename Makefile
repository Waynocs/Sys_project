all: client server

client: obj/client.o obj/sockets.o
ifneq ("$(wildcard bin)", "")
	@echo "bin exists"
else
	mkdir bin
endif
	gcc -o bin/client obj/client.o obj/sockets.o


obj/client.o: src/client/client.c
ifneq ("$(wildcard obj)", "")
	@echo "obj exists"
else
	mkdir obj
endif
	gcc -o obj/client.o -c src/client/client.c

server: obj/server.o obj/sockets.o
	gcc -o bin/server obj/server.o obj/sockets.o -pthread

obj/server.o: src/server/server.c
	gcc -o obj/server.o -c src/server/server.c


obj/sockets.o: src/common/sockets.c
	gcc -o obj/sockets.o -c src/common/sockets.c