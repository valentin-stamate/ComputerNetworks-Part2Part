#fisier folosit pentru compilarea serverului&clientului TCP iterativ

all:
	gcc server.c ./import/sqlite/sqlite3.c ./import/sql_low.c ./import/sql.c -lpthread -ldl -o server
	gcc client.c -o client
clean:
	rm -f *~server client