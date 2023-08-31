all: keygen enc_server dec_server enc_client dec_client

keygen: keygen.c
	gcc -std=c99 -o keygen keygen.c

enc_server: enc_server.c
	gcc -std=c99 -o enc_server enc_server.c

dec_server: dec_server.c
	gcc -std=c99 -o dec_server dec_server.c

enc_client: enc_client.c
	gcc -std=c99 -o enc_client enc_client.c

dec_client: dec_client.c
	gcc -std=c99 -o dec_client dec_client.c
