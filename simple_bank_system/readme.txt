Bank Server/Client by Max Davatelis

server – server program for managing centralized banking system
client – client program for connection to centralized banking system
	
Synopsis
	./server [PORT_NUMBER]
	./client [HOST_MACHINE_NAME] [PORT_NUMBER]
	
Client commands:
	-create [NAME]: creates new user with input name
	-serve [NAME]: serves user such that subsequent commands will act on user
	-deposit [AMOUNT]: deposits money into user's bank account
	-withdraw [AMOUNT]: withdraws money from user's bank account 
	-query: returns current user's balance
	-end: stops serving specific user
	-quit: close client
