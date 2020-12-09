#fisier folosit pentru compilarea serverului&clientului TCP iterativ

all:
	gcc server.c ./import/sqlite/sqlite3.c ./import/sql/sql_low.c ./import/sql/sql.c ./import/models.c -lpthread -ldl -o server.exe
	gcc client.c ./import/client/client_functions.c ./import/models.c -o client.exe
clean:
	rm -f *~server client