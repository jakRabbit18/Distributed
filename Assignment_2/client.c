//Everett Harding
//Assignment 2
//client code

/* 
Things we need for a client
 - socket descriptor sfd
 - recieve buffer (to recieve command outputs)
 - struct sockaddr_in server_address

Steps:
 - allocate read buffer
 - try to create socket (AF_INET, SOCK_STREAM)
 - allocate server_Address (sa, '0', 0)
 - set server_address fields
 	- AF_INET
 	- htons(5000)
 - attempt to establish ip connection with server
 	- inet_pton(AF_INET, address, &serv_addr.sin_addr)
 	- connect(sfd, server address, size of(server addr))
 - While (not exit)
	 - send command to server
	 - while (reading socket successfully)
	 	- print the returned outputs from the command
 - on exit, close the socekt
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
#include <fcntl.h>

#define BUFFSIZE 1024
#define FALSE 0
#define TRUE 1

char receiveBuffer[BUFFSIZE];
char sendBuffer[BUFFSIZE];

unsigned long hash;
// simple hash function from 
// https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned long hash_str (unsigned char *str) {
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void wipe(char *buffer) {
	memset(buffer, '\0', sizeof(buffer));
}


//TODO: add error checking for invalid username/password
int authenticate(const char *uname, const char *password, int sfd) {
	printf("uname: %s\n", uname);
	printf("password: %s\n", password);
	write(sfd, uname, strlen(uname));
	memset(sendBuffer, '\0', sizeof(sendBuffer));

	memset(receiveBuffer, '\0', sizeof(receiveBuffer));
	//read for rand number sent from server
	int read_res;
	while((read_res = read(sfd, receiveBuffer, sizeof(receiveBuffer)-1)) > 0) {
		receiveBuffer[read_res] = '\0';
		hash = strtoul(receiveBuffer, NULL, 10);
		break;
	}
	memset(receiveBuffer, '\0', sizeof(receiveBuffer));

	//create client hash and send to server for comparison
	unsigned long client_hash = hash_str((unsigned char*)password);
	sprintf(sendBuffer, "%ld", client_hash);
	write(sfd, sendBuffer, strlen(sendBuffer));
	memset(sendBuffer, '\0', sizeof(sendBuffer));
	if((read_res = read(sfd, receiveBuffer, sizeof(receiveBuffer)-1)) > 0) {
		receiveBuffer[read_res] = '\0';
		if(strcmp(receiveBuffer, "failed") == 0){
			printf("Auth failed\n");
			return -1;
		}
		else if(strcmp(receiveBuffer, "authenticated") == 0){
			printf("auth success\n");
			return 0;
		}
	}
	printf("gets here\n");

	return 0;
}

int main(int argc, char **argv) {
	//first set up some connection variables
	int sfd = 0, cfd = 0,opt;
	ssize_t read_loc;
	char *command;
	char *server;
	struct sockaddr_in server_address;

	while(argc > 2 && (opt = getopt(argc, argv, "hs:c:")) != -1) {
		switch(opt) {
			case 's':
				server = optarg;
				break;
			case 'c':
				//set the command to execute
				// printf("Found a command!\n");
				command = optarg;
				break;
			case 'h':
				// help message!
				printf("Usage: ./client <server address> [-c command] [-h]");
				break;
			default: 
				// unrecognized option
				printf("What is this? %c: %s\n", opt, optarg);
				break;
		}
	}

	if(server == NULL) {
		printf("No server specified\n");
		exit(EXIT_FAILURE);
	}

	// set up the buffers and the socket
	memset(receiveBuffer, '0', sizeof(receiveBuffer));
	memset(sendBuffer, '0', sizeof(sendBuffer));
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd < 0) {
		printf("\nError: socket was not created\n");
		return 1;
	}

	// set up the server address and fields
	memset(&server_address, '0', sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(5000);

	if(inet_pton(AF_INET, server, &server_address.sin_addr)<=0) {
		printf("\nError: inet error, %s\n", argv[1]);
		return 1;
	}

	if(cfd = connect(sfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
		printf("\nError: connection error, %s\n", strerror(errno));
		return 1;
	}

	// authenticate then run commands
	char x;
	int idx = 0;
	int buff_idx = 0;
	int isCD = FALSE;
	char *uname = "user";
	char *password = "password";
	if(authenticate(uname, password, sfd) == -1){
		printf("Error: authentication failed\n");
		exit(EXIT_FAILURE);
	}

	// read and send commands
    // if no specific command was specified.
    struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	if(command) {
		// printf("command: %s\n", command);
		// this has been set, let's just put it in the send buffer and be done with it
		if(strlen(command) > BUFFSIZE-1){
			printf("Error: command contains too many characters\n");
			exit(EXIT_FAILURE);
		}
		for(int i = 0; i < strlen(command); i++){
			sendBuffer[i] = command[i];
		}

		if(write(sfd, sendBuffer, strlen(sendBuffer)) == -1) {
			printf("Error: %s\n", strerror(errno));
		}

		isCD = strcmp(sendBuffer, "cd") == 0;

		// this bit reads the socket for the response from the server (no response from cd)
		
		if(!isCD){
			int blocks = 0;
			while((read_loc = recv(sfd, receiveBuffer, sizeof(receiveBuffer)-1, 0)) > 0) {
				receiveBuffer[read_loc] = 0;
				blocks++;
				if(fputs(receiveBuffer, stdout) == EOF) {
					printf("\nError: fputs is bad\n");
				}
			}
		}
		char *end = "exit";
		if(write(sfd, end, strlen("exit")) == -1) {
			printf("Error: %s\n", strerror(errno));
		}

	} else {
		// put this into "shell mode"
		printf("In shell mode\n");
		char c;
		buff_idx =0;
		char done = FALSE;
		while(!done) {
			while((c = getchar()) != '\n') {
				sendBuffer[buff_idx++] = c;
				if(buff_idx == BUFFSIZE-1){
					isCD = sendBuffer[0] == 'c' && sendBuffer[1] == 'd';
					sendBuffer[buff_idx] = '\0';
					//printf("\nbuff_idx: %d, %s, %ld\n", buff_idx, sendBuffer, strlen(sendBuffer));
					write(sfd, sendBuffer, strlen(sendBuffer));
					buff_idx = 0;
					memset(sendBuffer, '\0', sizeof(sendBuffer));
				}
			}
			// printf("Sending %s\n", sendBuffer);
			sendBuffer[buff_idx] = '\0';
			buff_idx = 0;
			if(write(sfd, sendBuffer, strlen(sendBuffer)) == -1) {
				printf("Error: %s\n", strerror(errno));
			}

			if(strcmp(sendBuffer, "exit") == 0){
				done = TRUE;
			}

			memset(sendBuffer, '\0', sizeof(sendBuffer));
			
			if(!isCD){
				while((read_loc = recv(sfd, receiveBuffer, sizeof(receiveBuffer)-1,0)) > 0) {
					receiveBuffer[read_loc] = 0;
					if(fputs(receiveBuffer, stdout) == EOF) {
						printf("\nError: fputs is bad\n");
					}
				}
			}
		}
	}
  	
  	printf("\nend shell\n");
	return 0;
}
