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
#define _XOPEN_SOURCE
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
#define PASSWORD "password"
#define USERNAME "user"
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
#define FALSE 0
#define TRUE 1

unsigned long hash;
// simple hash function from 
// https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned long hash_str (unsigned char *str) {
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

//https://brennan.io/2015/01/16/write-a-shell-in-c/
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;
 
  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }
 
  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;
 
    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
 
    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

struct Node{
    // Any data type can be stored in this node
    pid_t  pid;
    struct Node *next;
};

/* Function to add a node at the beginning of Linked List. */
struct Node * push(struct Node* head_ref, pid_t new_data) {
    // Allocate memory for node
    // printf("Pushing new data %ld\n", (long) new_data);
    struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
 	
 	// printf("adding pid: %ld\n", (long) new_data);
    new_node->pid = new_data;
    // printf("updating head ref\n");
    new_node->next = head_ref;
    // Change head pointer as new node is added at the beginning
    // printf("updating head ref to be new node\n");
    head_ref = new_node;
    return new_node;
}

/* function to remove a node from the list */
struct Node *delete_node(struct Node *head, pid_t pid){
	// printf("deleteing pid %d\n", pid);
	struct Node *n;
	// if there's no head, there's no pid in the list
	// if there is a head and there's no next node, 
	// 	check the pid, if it doesn't match, the pid isn't in the list
	// ===== end the "just a head" case =====
	// there is now definitely a head and a next node
	// if the next node's pid matches,
	// 	set head->next to next->next
	// 	free n
	// if none of that matched, call it on the next node
	if(head == NULL) {
		printf("No processes in the list\n");
		return NULL;
	}

	// handles the case that the pid is the only one in the list
	if(head->next == NULL) {
		if(head->pid == pid) {
			free(head);
			return NULL;
		}
	}

	n = head->next;
	if(n->pid == pid){
		//the next node contains the pid
		head->next = n->next;
		free(n);
		return head;
	}

	//still looking for the pid
	return delete_node(n, pid);
}

int execute_command(char **args){
	pid_t cmd_pid;
	pid_t cmd_wpid;
	int cmd_status;
	cmd_pid = fork();
	if(cmd_pid == 0){
		//child proc
		if(execvp(args[0], args) == -1) {
			printf("Error in execvp: %s\n", strerror(errno));
		}
	}
	else if (cmd_pid < 0){
		perror("error in cmd fork");
	}
	else{
		//parent proc
		do {
			cmd_wpid = waitpid(cmd_pid, &cmd_status, WUNTRACED);
		} while (!WIFEXITED(cmd_status) && !WIFSIGNALED(cmd_status));
	}
	return 0;
}

int execute_help(){
	printf("distributed shell\n");
	//specify possible command line options here
	return 0;
}

int execute_cd(char **args){
	//printf("%s\n", args[0]);
	//printf("%s\n", args[1]);
	if(chdir(args[1]) != 0){
		perror("cd failed");
	}
	return 0;
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
	pid_t pid;
	while(!exit) {
		cfd = accept(lfd, (struct sockaddr*) NULL, NULL);
		// we have a connected client! now fork to handle their requests
		// and to keep accepting more. More clients more money, more problems
		pid = fork();
		if(pid != 0) {
			// this is the parent, hang around and wait for the child to return
			// while also waiting for more clients
			// printf("Parent: %ld\n", (long) pid);
			pid_head = push(pid_head, pid);

			pid_t child = waitpid(-1, &status, WNOHANG);
			// printf("child returned: %ld\n", (long) child);
			if(child > 0) {
				// a child has returned! remove it from the list
				delete_node(pid_head, child);
			}

			if(pid_head == NULL) {
				exit = 1;
			}

			// close the accepted socket here
			close(cfd);
			cfd = 0;

		} else {
			// this is the child process, where all the magic happens
			printf("Connected!\n");
			pid_t me = getpid();
			// printf("Child pid: %ld\n", (long) me);

			// close the listening socket
			//close(lfd);
			// printf("close lfd\n");

			// check the user name given
			while((read_res = read(cfd, readBuffer, sizeof(readBuffer)-1)) > 0) {
				readBuffer[read_res] = '\0';
				printf("user name given: %s\n", readBuffer);
				if(strcmp(readBuffer, USERNAME) != 0) {
					return EXIT_FAILURE;
				}
				break;
			}

			//user name checked out, generate a random number
			memset(readBuffer, '\0', sizeof(readBuffer));
			hash = (unsigned long) rand();
			// printf("hash on server: %ld\n", hash);
			sprintf(sendBuffer, "%ld", hash);
			// printf("hash sent to client: %s\n", sendBuffer);
			write(cfd, sendBuffer, strlen(sendBuffer));
			// printf("server password: %s\n", PASSWORD);
			unsigned long local_hash = hash_str(PASSWORD);

			while((read_res = read(cfd, readBuffer, sizeof(readBuffer)-1)) > 0) {
				unsigned long foreign_hash = strtoul(readBuffer, NULL, 10);
				// printf("local hash: %ld\n", local_hash);
				// printf("foreign hash: %ld\n", foreign_hash);
				readBuffer[read_res] = '\0';
				if(local_hash != foreign_hash) {
					printf("failing in hash comparison\n");
					sprintf(sendBuffer, "%s", "failed");
					write(cfd, sendBuffer, strlen(sendBuffer));
					close(cfd);
					return EXIT_FAILURE;
				}
				else{
					printf("successfully authenticated\n");
					sprintf(sendBuffer, "%s", "authenticated");
					write(cfd, sendBuffer, strlen(sendBuffer));
				}
				break;
			}
			memset(readBuffer, '\0', sizeof(readBuffer));
			memset(sendBuffer, '\0', sizeof(sendBuffer));

			//now client is authenticated

			// pipe output to the socket
			dup2(cfd, STDOUT_FILENO);
			dup2(cfd, STDERR_FILENO);
			// read whatever is sent from the client in batches of BUFFSIZE
			int exit = FALSE;
			while(!exit){
				while((read_res = read(cfd, readBuffer, sizeof(readBuffer)-1)) > 0){
					readBuffer[read_res] = '\0';
					// printf("Recieved the following: %s\n", readBuffer);
					if(strcmp(readBuffer, "exit") == 0) {
						exit = TRUE;
						break;
					}
					char **args;
					args = lsh_split_line(readBuffer);
					if(strcmp(args[0], "help") == 0){
						execute_help();
					}
					else if(strcmp(args[0], "cd") == 0){
						execute_cd(args);
						printf("%d", EOF);
					}
					else{
						execute_command(args);
						printf("%d", EOF);
					}
					// TODO:
					// add specific commands (cd, help, etc)
					memset(readBuffer, '\0', sizeof(readBuffer));
					free(args);
					break;
				}
			}
			//clean up at the end
			close(cfd);
			cfd = 0;
			memset(readBuffer, '\0', sizeof(readBuffer));
			// printf("ending child process %ld...\n", (long) me);
			break;
		}
	}

	if(read_res < 0) {
		printf("read error\n");
	}

	while(pid_head != NULL && pid != 0){
		// printf("waiting for children of %ld to finish\n", (long) getpid());
		pid_t child = wait(0);
		// a child has returned! remove it from the list
		delete_node(pid_head, child);
	}

	close(cfd);
	// printf("PID %d ended successfully\n", getpid());
	return 0;
}
