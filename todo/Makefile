CC=gcc
DB_LIBS=sqlite3
DEBUG=-g -D DEBUG
OBJ=./obj/
OUT=./out/

db_debug: db_test.o data.o
	$(CC) -Wall $(DEBUG) db_test.o data.o -l $(DB_LIBS) -o db_debug
	mv ./*.o $(OBJ)
	mv ./db_debug $(OUT)
db_test.o:
	$(CC) -c db_test.c
data.o:
	$(CC) -c data.c

clean:
	rm $(OBJ)* $(OUT)*