CC=gcc
DEBUG=-g -DDEBUG
OFLAGS=-Wall -Wshadow -c 
OBJ=./obj/
OUT=./out/
SRC=./src/
HEADERS=-Iheaders

data_debug: dev_console.o data.o
	$(CC) $(DEBUG) dev_console.o data.o -l sqlite3 -o dev_console
	mv ./*.o $(OBJ)
	mv ./dev_console $(OUT)

client_debug: utils.o network.o client.o
	$(CC) $(DEBUG) client.o utils.o network.o -l sqlite3 -o client
	mv ./*.o $(OBJ)
	mv ./client $(OUT)

server_debug: server.o data.o utils.o network.o
	$(CC) $(DEBUG) server.o data.o utils.o network.o -l sqlite3 -o server
	mv ./*.o $(OBJ)
	mv ./server $(OUT)

network.o:
	$(CC) $(OFLAGS) -g $(HEADERS) $(SRC)network.c
utils.o:
	$(CC) $(OFLAGS) -g $(HEADERS) $(SRC)utils.c
server.o:
	$(CC) $(OFLAGS) -g $(HEADERS) $(SRC)server.c
client.o:
	$(CC) $(OFLAGS) -g $(HEADERS) $(SRC)client.c
dev_console.o:
	$(CC) $(OFLAGS) -g $(HEADERS) $(SRC)dev_console.c
data.o:
	$(CC) $(OFLAGS) -g $(HEADERS) $(SRC)data.c

clean:
	rm $(OBJ)* $(OUT)*