#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <stdbool.h>
#include <signal.h>
#include <sys/time.h>
#include <netinet/in.h> 
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>

#define INTERVAL 15000
#define QUERY_SIZE 512

#define ERR(x) write(2, x, strlen(x))
#define QUIT(x) {ERR(x); exit(EXIT_FAILURE);}

// linked list struct that holds client connection information
typedef struct __clientdat {
	int fd;
	pthread_t id;
	struct __clientdat *next;
} clientdat;

// link list that holds account information
typedef struct __account {
	char name[255];
	double balance;
	bool in_session;;
	struct __account *next;
	clientdat *session;
} account;


void sigint_handle();
void*accept_clients();
void *service_client(void *arg);
void init_diagnostic();
void print_diagnostic();

clientdat *_clientdat(const int _fd, const pthread_t _id);
clientdat *search_client(int fd);
int add_client(clientdat *data);
int rem_client(int fd);
account *_account(const char *_name);
account *search_account(const char *name);
void add_account(account *add);


// static list pointers
clientdat 			*client_root;
  account			*account_root;

// single thread that accepts connections
// and creates service threads
pthread_t 			acceptor_thread;

// semaphores
	sem_t 			client_sem;
	sem_t			run_sem;

// global variables
struct sockaddr_in  server, client;
	int 			sockfd = 0;
	bool 			quit_request = false;

int main(int argc, char **argv)
{
	int opt = 1;
	
	if(argc != 2){
		write(2, "Server requires port # as an argument!\n", 39);
		return 1;
	}
	
	//initialize semaphores
	if(sem_init(&client_sem, 0, 1))
		QUIT("Semaphore init failed\n");
	if(sem_init(&run_sem, 0, 1))
		QUIT("Semaphore init failed\n");
	
	int port = atoi(argv[1]);
	
	signal(SIGINT, sigint_handle);
	signal(SIGTSTP, sigint_handle);
	init_diagnostic();
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(!sockfd){
		ERR("Socket creation failed\n");
		exit(EXIT_FAILURE);
	}
	
	client_root = _clientdat(0, 0);
	account_root = _account("\0");
	
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = htonl(INADDR_ANY); 
    server.sin_port = htons(port); 
    
    bind(sockfd,(struct sockaddr *)&server , sizeof(server));
   
    pthread_create(&acceptor_thread, NULL, accept_clients, NULL);
	
	pthread_join(acceptor_thread, NULL);
}


/** handles stutdown of server on SIGINT (ctrl+c)  **/
void sigint_handle(){
	
	printf("\nShutting down server...\n");
	
	setitimer(0, 0, NULL);
	
	// set quit flag to true to shut non blocking threads down
	sem_wait(&run_sem);
	quit_request = true;
	sem_post(&run_sem);
	
	// cancel acceptor thread since it's non blocking
	pthread_cancel(acceptor_thread);
	
	// flag quit request
	sem_wait(&client_sem);
	
	printf("Closing clients....\n");
	// close all connections and stop all service threads
	clientdat *ptr = client_root->next;
	while(ptr)
	{
		// for every client, send quit command
		send(ptr->fd, "quit", 4, 0);
		
		// close client's socket
		close(ptr->fd);
		
		ptr = ptr->next;
	}
	
	printf("Freeing clients....\n");
	// free all clientdat nodes
	ptr = client_root->next;
	clientdat *prev = client_root;
	while(ptr)
	{
		free(prev);
		prev = ptr;
		ptr = ptr->next;
	}
	free(ptr);
	
	
	printf("Freeing accounts....\n");
	// free all account nodes
	account *ptr2 = account_root->next;
	account *prev2 = account_root;
	while(ptr2)
	{
		free(prev2);
		prev2 = ptr2;
		ptr2 = ptr2->next;
	}
	free(ptr2);
	
	sem_post(&client_sem);
	
	// close server socket
	if(sockfd)
		close(sockfd);
		
	printf("Waiting on threads....\n");
	sleep(1);
	printf("Shut down complete\n");
		
	// exit program
	exit(EXIT_SUCCESS);
}


/** startup function for session acceptor thread **/
void *accept_clients()
{
	printf("Server started successfully. Listening for connections...\n");
	if(listen(sockfd, 10))
	{
		printf(strerror(errno));
		exit(EXIT_FAILURE);
	}
	while(true)
	{
		
		// check quit request every loop
		sem_wait(&run_sem);
		if(quit_request)
			break;
		sem_post(&run_sem);
		
		// accept client to obtain socketfd
		socklen_t client_size = sizeof(client);
		int clientfd = accept(sockfd, (struct sockaddr *)&client, &client_size);
		
		if(clientfd < 0){
			if(errno != 0 && (errno != EAGAIN && errno != EWOULDBLOCK))
			{
				ERR("Failed connection with error: ");
				ERR(strerror(errno));
				ERR("\n");
			}
		} 
		else 
		{
			// set non-blocking flag in new client socket
			if(fcntl(clientfd, F_SETFL, fcntl(clientfd, F_GETFL, 0) | O_NONBLOCK))
				QUIT("Couldn't set socket to non-blocking\n")
			
			// obtain IP and print status message
			char str_ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(client.sin_addr.s_addr), str_ip, INET_ADDRSTRLEN);
			printf("Accepted connection from client: %s\n", str_ip);
			
			// create service thread and add it to 
			// client thread pool if threre is room
			pthread_t service_thread;
			clientdat *new_client = _clientdat(clientfd, service_thread);
			if(add_client(new_client))
				ERR("Concurrent client service limit reached!\n");
			else
				pthread_create(&service_thread, NULL, service_client, (void*)&clientfd);
			
		}
	}
	return;
}

/** startup function for client service threads **/
void *service_client(void *arg)
{
	// cast argument to int
	int clientfd = *((int *)arg);
	
	// pointer to the account node belonging to shared link-list 
	account *served_account;
	
	// this pthread's personal char buffer; used to read, write, and 
	// decode queries from the single socket it is connected to
	char buf[QUERY_SIZE];
	int _read = 0;
	
	while(true)
	{
		sem_wait(&run_sem);
		if(quit_request)
			break;
		sem_post(&run_sem);
		
		// clean thread buffer which is dirty from all the other uses it has
		memset(buf, 0, QUERY_SIZE-1);
		
		// read from socket to thread buffer
		_read = recv(clientfd, buf, QUERY_SIZE, 0);
		
		// most likely would have blocked but there could be other errors
		if(_read <= 0)
		{
			if(errno != 0 && (errno != EAGAIN && errno != EWOULDBLOCK))
			{
				ERR("Error reading from client: ");
				ERR(strerror(errno));
				ERR("\n");
			}
		}
		
		// there is an incoming query
		else
		{
			
			// decode and execute
			
			if(strncmp(buf, "quit", 4)==0)
			{
				sem_wait(&client_sem);
				// close account session
				if(served_account)
				{
					served_account->session = NULL;
					served_account->in_session = false;
				}
					
				sem_post(&client_sem);	
				rem_client(clientfd);
				printf("Client (socket # %i) quit\n", clientfd);
				return;
			}
			
			else if(strncmp(buf, "query", 5)==0)
			{
				if(served_account){
					sem_wait(&client_sem);
					snprintf(buf, QUERY_SIZE-1, "ret:Account balance for %s: %0.2lf", 
						served_account->name, served_account->balance);
					send(clientfd, buf, QUERY_SIZE, 0);
					sem_post(&client_sem);
				}
				else
					send(clientfd, "S_ERR: Session not active on any account", 40, 0);
				
			}
			
			else if(strncmp(buf, "end", 3)==0)
			{
				// in session, close out
				if(served_account)
				{
					char name[255];
					
					// change the account in the list and 
					// get rid of pointer to it
					sem_wait(&client_sem);
						strcpy(name, served_account->name);
						served_account->in_session = false;
						served_account->session == NULL;
						served_account = 0;
					sem_post(&client_sem);
					
					// send confirmation message
					snprintf(buf, QUERY_SIZE-1, "ret:Ended service to %s", name);
					send(clientfd, buf, QUERY_SIZE, 0);
				}
				// not in session, send error message
				else {
					send(clientfd, "S_ERR: No session to end", 24, 0);
				}
			}
			
			else if(strncmp(buf, "serve", 5)==0)
			{
				
				// session already has account in service
				if(served_account){
					send(clientfd, "S_ERR: Use 'end' command to change to new user", 48, 0);
					continue;
				}
				
				// obtain name of account client wants to switch to
				char name[256];
				strncpy(name, buf+6, 255);
				
				// search user in account list
				account *usr;
				
				// user does not exist
				if(!(usr = search_account(name)))
					send(clientfd, "S_ERR: Account does not exist", 29, 0);
				// user found
				else
				{
					// set current served account to user
					sem_wait(&client_sem);
					
					// other client in session with user
					if(usr->in_session) {
						send(clientfd, "S_ERR: User in session with other client", 40, 0);
						sem_post(&client_sem);
						continue;
					}
					
					served_account = usr;
					sem_post(&client_sem);
					
					// bind user with client
					clientdat *session = search_client(clientfd);
					
					if(!session)
						send(clientfd, "S_ERR: Fatal error retrieving session", 37, 0);
					else
					{
						// update account info
						sem_wait(&client_sem);
							usr->in_session = true;
							usr->session = session;
						sem_post(&client_sem);
						
						// send confirmation message
						snprintf(buf, QUERY_SIZE-1, "ret:Now serving %s", name);
						send(clientfd, buf, QUERY_SIZE, 0);
					}
				}
			}
			
			else if(strncmp(buf, "create", 6)==0)
			{
				if(served_account)
					send(clientfd, "S_ERR: Session already serving account", 39, 0);
				else
				{
					char name[256];
					strncpy(name, buf+7, 255);
					
					account *usr = search_account(name);
					
					if(usr)
					{
						send(clientfd, "S_ERR: Account already exists", 29, 0);
						continue;
					}
					
					account *add = _account(name);
					add_account(add);
					
					send(clientfd, "ret:Account sucessfully created", 31, 0);
					
				}
			}
		
			else if(strncmp(buf, "deposit", 7)==0)
			{
				if(!served_account)
					send(clientfd, "S_ERR: No session active", 24, 0);
				else 
				{
					double deposit;
					// parse double
					sscanf(buf+8, "%lf", &deposit);
					
					// update value and send message
					sem_wait(&client_sem);
					served_account->balance += deposit;
					snprintf(buf, QUERY_SIZE-1, "ret:Balance for %s now %0.2lf",
									served_account->name, served_account->balance);
					send(clientfd, buf, QUERY_SIZE, 0);
					sem_post(&client_sem);
					
				}
			}
			
			else if(strncmp(buf, "withdraw", 8)==0)
			{
				if(!served_account)
					send(clientfd, "S_ERR: No session active", 24, 0);
				else 
				{
					double withdraw;
					sscanf(buf+8, "%lf", &withdraw);
					
					sem_wait(&client_sem);
					
					// not enough money to withdraw
					if(withdraw > served_account->balance)
					{
						snprintf(buf, QUERY_SIZE-1, 
							"S_ERR: Cannot withdraw %0.2lf out of account with %0.2lf", 
							withdraw, served_account->balance);
					} 
					
					// enough money
					else 
					{
						served_account->balance -= withdraw;
						snprintf(buf, QUERY_SIZE-1, "ret:Balance for %s now %0.2lf",
									served_account->name, served_account->balance);
					}
					
					sem_post(&client_sem);
					send(clientfd, buf, QUERY_SIZE, 0);
					
				}
			}
			
			else 
				send(clientfd, "S_ERR: Invalid operation", 39, 0);
			
		}	
	}
	return;
}

/** initializes SIGALARM signal that signals every 15 seconds **/
void init_diagnostic(){
	
	signal(SIGALRM, print_diagnostic);
	
	struct itimerval it_val;
	it_val.it_value.tv_sec =     INTERVAL/1000;
	it_val.it_value.tv_usec =    (INTERVAL*1000) % 1000000;	
	it_val.it_interval = it_val.it_value;
	if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
		ERR("error calling setitimer()");
		exit(EXIT_FAILURE);
	}
}


/** prints contents of user database on SIGALARM **/
void print_diagnostic(){
	
	sem_wait(&client_sem);
	account *ptr = account_root->next;
	printf("---------------------DIAG---------------------\n");
	while(ptr)
	{
		char *in_service = ptr->in_session ? "IN_SERVICE":" ";
		printf("%s\t%0.2lf\t%s\n", ptr->name, ptr->balance, in_service);
		ptr = ptr->next;
	}
	printf("----------------------------------------------\n");
	sem_post(&client_sem);
}

// CLIENT DATA STRUCTURE

clientdat *_clientdat(const int _fd, const pthread_t _id)
{
	clientdat *ret = (clientdat *)malloc(sizeof(struct __clientdat));
	ret->fd = _fd;
	ret->id = _id;
	return ret;
}

clientdat *search_client(int fd)
{
	sem_wait(&client_sem);
	clientdat *ptr = client_root->next;
	while(ptr)
	{
		if(ptr->fd == fd)
		{
			sem_post(&client_sem);
			return ptr;
		}
		ptr = ptr->next;
	}
	sem_post(&client_sem);
	return NULL;
}

int add_client(clientdat *data)
{
	int i;
	sem_wait(&client_sem);
	
	clientdat *ptr = client_root;
	
	while(ptr->next)
		ptr = ptr->next;
		
	ptr->next = data;
	
	sem_post(&client_sem);
	return 0;
}

int rem_client(int fd)
{
	close(fd);
	sem_wait(&client_sem);
	clientdat *ptr = client_root->next;
	clientdat *prev = client_root;
	while(ptr)
	{
		if(ptr->fd == fd)
		{
			prev->next = ptr->next;
			free(ptr);
			sem_post(&client_sem);
			return 0;
		}
		
		prev = ptr;
		ptr = ptr->next;
	}
	sem_post(&client_sem);
	return 1;
}

account *_account(const char *_name){
	account *ret = (account *)malloc(sizeof(struct __account));
	strncpy(ret->name, _name, 256);
	ret->in_session = false;
	ret->balance = 0;
	return ret;
}

void add_account(account *add)
{
	sem_wait(&client_sem);
	account *ptr = account_root;
	
	while(ptr->next)
		ptr = ptr->next;
		
	ptr->next = add;
	sem_post(&client_sem);
}

account *search_account(const char *_name)
{
	sem_wait(&client_sem);
	account *ptr = account_root->next;
	while(ptr)
	{
		if(strcmp(ptr->name, _name) == 0)
		{
			sem_post(&client_sem);
			return ptr;
		}
		ptr = ptr->next;
	}
	sem_post(&client_sem);
	return NULL;
}


