all: client server

client: obj/client.o
ifneq ("$(wildcard bin)", "")
	@echo "bin exists"
else
	mkdir bin
endif
	gcc -o bin/client obj/client.o

obj/client.o: src/client/client.c
ifneq ("$(wildcard obj)", "")
	@echo "obj exists"
else
	mkdir obj
endif
	gcc -o obj/client.o -c src/client/client.c

server: obj/server.o
ifneq ("$(wildcard bin)", "")
	@echo "bin exists"
else
	mkdir bin
endif
	gcc -o bin/server obj/server.o

obj/server.o: src/server/server.c
ifneq ("$(wildcard obj)", "")
	@echo "obj exists"
else
	mkdir obj
endif
	gcc -o obj/server.o -c src/server/server.c