all: server1 client1

server1: bank_server.c
	gcc -pthread -o bankingServer bank_server.c
	
client1: bank_client.c
	gcc -pthread -o bankingClient bank_client.c
