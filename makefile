all: bin/client bin/server

bin/client: src/bel_common.c src/bel_client.c
	gcc -Wall -Wextra -o bin/client src/bel_common.c src/bel_client.c
	
bin/server: src/bel_common.c src/bel_server.c
	gcc -Wall -Wextra -o bin/server src/bel_common.c src/bel_server.c
