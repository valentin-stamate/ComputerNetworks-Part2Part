all:
	gcc server.c ./import/sqlite/sqlite3.c ./import/server/server_low.c ./import/server/server_functions.c ./import/models.c ./import/utility.c -lpthread -ldl -o server
	gcc client.c ./import/client/client_functions.c ./import/models.c ./import/utility.c -lpthread -ldl -o client
clean:
	rm -f *~server client
