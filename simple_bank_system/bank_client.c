#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <netdb.h>
#include <errno.h>
#include <fcntl.h> 

#define _QUIT(x) {_ERR(x); exit(EXIT_FAILURE);}
#define _ERR(x) write(2, x, strlen(x))

#define TRUE 1
#define FALSE 0

// size of bytes to send/recieve
#define QUERY_SIZE 512

pthread_t 			query_thread, retrieve_thread;
int					sockfd;
struct sockaddr_in 	server;

void *query();
void *fetch_response();

int main(int argc, char **argv)
{
	
	char *server_name;
	char ip[100];
	int port;

	// improper arguments
	if(argc != 3)
		_QUIT("Proper use: [SERVER_NAME] [PORT_NUMER]\n")
	
	// get server information from input arguments
	server_name = argv[1];
	port = atoi(argv[2]);
	
	// initialize socket
	if(!(sockfd = socket(AF_INET, SOCK_STREAM, 0)))
		_QUIT("Could not create socket\n")
	
	// obtain server IP from name
	struct hostent *he;
    if(!(he = gethostbyname(server_name)))
		_QUIT("Couldn't resolve host IP\n")
	
	// configure sockaddr
	memset(&server, '0', sizeof(server)); 
	server.sin_family = AF_INET; 
    server.sin_port = htons(port);
    server.sin_addr = *((struct in_addr *)he->h_addr_list[0]);
    
    // set server socket to non blocking
    if(fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK))
		_QUIT("Couldn't set socket to non-blocking\n")
	
	// obtain IP string for printing later
	char str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(server.sin_addr.s_addr), str, INET_ADDRSTRLEN);
   
    
    int err;			// error message from connect
    int attemps = 10;	// max number of attempts before quitting
    
    // make connection to server
    while(attemps)
    {
		err = connect(sockfd, (struct sockaddr *)&server, sizeof(server));
		
		// couldn't connect for some reason
		if(err<0)
		{
			// already connected
			if(errno == EISCONN) break;
			
			// would have blocked, sleep and continue
			else{
				printf("Attempting to connect...\n");
				sleep(3);
				continue;
			}
		}
		// connection succeeded!
		else continue;
	}
	
	// used all attemptps, still not connected
	if(!attemps) {
		printf("Failed to connect to server\n");
		exit(EXIT_SUCCESS);
	}
	
	// print message indicating client has connection to server
	printf("Connected to: %s:%i (%s)\n", str, port, he->h_name);
	printf("Commands: create,serve,deposit,withdraw,query,end,quit\n");
    
    // create worker threads
    pthread_create(&query_thread, NULL, query, NULL);
    pthread_create(&retrieve_thread, NULL, fetch_response, NULL);
    
    // wait on threads completion
    pthread_join(query_thread, NULL);
    pthread_join(retrieve_thread, NULL);
    
    // close socket
    close(sockfd);
    
	return EXIT_SUCCESS;
	
}

/** query thread startup function **/
/** waits to read from stdin and sends messages to server **/
void *query()
{
	
	char in[QUERY_SIZE]; // input buffer
	long block_timer;	 // keeps time to throttle input
	long wait_time = 2;	 // time in seconds input should be throttled
	
	while(TRUE)
	{
		// clear input buffer
		memset(in, 0, QUERY_SIZE-1);
		
		
		
		// print console to indicate client is read to take command
		printf("client.c > ");
		
		// read command
		fgets(in, QUERY_SIZE-1, stdin);
		
		// remove newline out of input
		in[strcspn(in, "\n")] = 0;
		
		// set timer
		block_timer = time(NULL);
		
		// send message and print error if there is one
		if(send(sockfd, in, QUERY_SIZE-1, 0)==-1)
			printf("Error sending: %s\n", strerror(errno));
			
		// handle quit message on client side
		if(strncmp(in, "quit", 4)==0)
			break;
		
		// block the program from going to the next loop
		// until it has been 2 seconds since start of last loop
		if(block_timer>((long)time(NULL))-wait_time)
			sleep(block_timer + wait_time - ((long)time(NULL)));
	}
	
	printf("Shutting down all threads...\n");
	
	// tell retrieve thread to shut down
	pthread_cancel(retrieve_thread);
	
	// unblock pthread_join() in main
	return;
}

/** retrieve thread startup function **/
/** continuously recieves data from server **/
void *fetch_response()
{
	int _read;			
	char buf[QUERY_SIZE];
	while(TRUE)
	{
		// clear buffer before recieving
		memset(buf, 0, QUERY_SIZE-1);
		_read = recv(sockfd, buf, QUERY_SIZE, 0);
		
		
		// no data recieved
		if(_read==-1)
		{
			// would have blocked, just continue
			if(errno == EAGAIN)
				continue;
				
			// error recieving, print error and continue after waiting
			else
			{
				_ERR(strerror(errno));
				sleep(3);
				continue;
			}
		}
		
		if(_read==0)
			continue;
			
		// server returning data query
		if(strncmp(buf, "ret:", 4)==0)
		{
			printf("Command completed. Return message: %s\n", buf+4);
		}
		
		// server sent message indicating it's shutting down
		if(strcmp(buf, "quit")==0)
			break;
		
		// handle errors from server
		if(strncmp(buf, "S_ERR:", 6)==0)
		{
			printf("Server Error: %s\n", buf+7);
		}
	}
	
	printf("\nServer shutting down... exiting\n");
	
	// tell query thread to shut down
	pthread_cancel(query_thread);
	
	// unblock pthread_join() in main
	return;
	
}
