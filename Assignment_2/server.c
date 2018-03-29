// Everett Harding
// Assignment 2
// Server code
// boiler plate setup code based on 
// http://www.thegeekstuff.com/2011/12/c-socket-programming/

/*
things we need for a server
 - listen descriptor (lfd)
 - connection descriptor (cfd)
 - sendBuffer (sbuff)
 - time counter 
 - sturct sockaddr_in server_address

 Steps:
  - open the socket with AF_INET and SOCK_STREAN
  - set the memory for server_address
  - initialized the send buffer
  - set the fields of the server_address
  	- sin_family = AF_INET
  	- sin_add.s_addr = htonl(INADDR_ANY) (change the last to restrict access?)
	- sin_port = htons(5000)
  - use bind() to bind lfd to the socket at server_address, with size of(server_address)
  - use listen(listenfd, 10) to open the socket with 10 maximum clients
  - wait for connections
  	- while(1) {
		cfd = accept(listenfd, (struct sockaddr*) NULL, NULL)
  	}

  	- once accepted, can break
  - can send/read over the connection cfd
  - use snprintf() and write(cfd, "message", len(message))
  - run close(cfd) to close the connection when done

 Remaining Questions:
  - how to check for timeout errors?
  - how to terminate connection with commands
  - how to write command outputs back to client

  */

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>



#define BUFFSIZE 1024

struct Node{
    // Any data type can be stored in this node
    pid_t  *pid;
    struct Node *next;
};

/* Function to add a node at the beginning of Linked List. */
void push(struct Node* head_ref, pid_t new_data) {
    // Allocate memory for node
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
 	
 	printf("adding pid: %ld\n", (long) new_data);
    *(new_node->pid) = new_data;
    new_node->next = head_ref;
 
    // Change head pointer as new node is added at the beginning
    head_ref = new_node;
}

/* function to remove a node from the list */
int delete_node(struct Node *head, pid_t pid){
	if(head->next == NULL) {
		// if we've reached the end of the list
		// the node is not in here... odd. return error
		perror("Error: given node is not present in the list.\n");
		return -1;
	}

	if(*(head->pid) == pid) {
		// we found it! 
		// connect the node after the target node to the current head
		// then get rid of the target node
		struct Node *n = head->next;
		head->next = n->next;
		free(n);
		return 0;
	}

	// we still have some list left and we haven't found the node
	// keep looking!
	return delete_node(head->next, pid);
}

int main(int argc, char **argv) {
	// this is the main function where the main stuff happens
	int lfd = 0, cfd = 0;
	char sendBuffer[BUFFSIZE];
	char readBuffer[BUFFSIZE];

	struct sockaddr_in server_add;

	// set up the socket
	lfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&server_add, '0', sizeof(server_add));
	memset(sendBuffer, '0', sizeof(sendBuffer));

	// set up the socket struct
	server_add.sin_family = AF_INET;
	server_add.sin_addr.s_addr = htonl(INADDR_ANY);
	server_add.sin_port = htons(5000);

	// bind the socket
	bind(lfd, (struct sockaddr*)&server_add, sizeof(server_add));

	// set up the client queue on the socekt to 10
	listen(lfd, 10);

	// wait for a client to connect
	// TODO: add timeouts
	char connect = 0;
	int read_res; 
	char exit = 0;
	struct Node *pid_head = NULL;
	int status;
	while(!exit) {
		cfd = accept(lfd, (struct sockaddr*) NULL, NULL);
		// we have a connected client! now fork to handle their requests
		// and to keep accepting more. More clients more money, more problems
		pid_t pid = fork();
		if(pid != 0) {
			// this is the parent, hang around and wait for the child to return
			// while also waiting for more clients
			printf("Parent: %ld\n", (long) pid);
			push(pid_head, pid);
			pid_t child = waitpid(-1, &status, WNOHANG);
			if(child) {
				// a child has returned! remove it from the list
				delete_node(pid_head, child);
			}


		} else {
			// this is the child process, where all the magic happens
			printf("Connected!\n");
			pid_t me = getpid();
			printf("Child pid: %ld\n", (long) me);
			// read whatever is sent from the client in batches of BUFFSIZE
			while((read_res = read(cfd, readBuffer, sizeof(readBuffer)-1)) > 0){
				readBuffer[read_res] = '\0';
				printf("%s, %ld, %d\n", readBuffer, strlen(readBuffer), read_res);
				if(strcmp(readBuffer, "exit") == 0) {
					exit = 1;
					break;
				}
				memset(readBuffer, '\0', sizeof(readBuffer));

			}

			//clean up at the end
			close(cfd);
			cfd = 0;
			memset(readBuffer, '\0', sizeof(readBuffer));
			printf("ending child process %ld...\n", (long) me);
		}
	}

	if(read_res < 0) {
		printf("read error\n");
	}

	while(pid_head != NULL){
		pid_t child = wait(0);
		// a child has returned! remove it from the list
		delete_node(pid_head, child);
	}

	close(cfd);
	return 0;
}
