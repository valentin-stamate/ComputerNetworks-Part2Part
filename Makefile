#fisier folosit pentru compilarea serverului&clientului TCP iterativ

all:
	gcc server.c ./sqlite/sqlite3.c -lpthread -ldl -o server
	# gcc client.c -o client
# clean:
# 	rm -f *~server client