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

#define BUFFSIZE 1024

unsigned long hash;
// simple hash function from 
// https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned long hash_str (const char *str) {
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void wipe(char *buffer) {
	memset(buffer, '\0', sizeof(buffer));
}


//TODO: add error checking for invalid username/password
int authenticate(const char *uname, const char *password, char *receiveBuffer, char *sendBuffer, int sfd) {
	printf("uname: %s\n", uname);
	printf("password: %s\n", password);
	write(sfd, uname, strlen(uname));
	memset(sendBuffer, '\0', sizeof(sendBuffer));

	//read for rand number sent from server
	int read_res;
	while((read_res = read(sfd, receiveBuffer, sizeof(receiveBuffer)-1)) > 0) {
		receiveBuffer[read_res] = '\0';
		hash = strtoul(receiveBuffer, NULL, 10);
		break;
	}
	wipe(receiveBuffer);

	//create client hash and send to server for comparison
	unsigned long client_hash = hash_str(password);
	sprintf(sendBuffer, "%ld", client_hash);
	write(sfd, sendBuffer, strlen(sendBuffer));
	wipe(sendBuffer);

	return 0;
}

int main(int argc, char **argv) {
	//first set up some connection variables
	int sfd = 0, cfd = 0, read_loc=0, opt;
	char receiveBuffer[BUFFSIZE];
	char sendBuffer[BUFFSIZE];
	char *command;
	struct sockaddr_in server_address;

	if(argc < 2) {
		// no server was specified, print error and exit
		printf("Error: no server specified\n");
		exit(EXIT_FAILURE);
	}

	while(argc > 2 && (opt = getopt(argc, argv, "ch:")) != -1) {
		switch(opt) {
			case 'c':
				//set the command to execute
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

	if(inet_pton(AF_INET, argv[1], &server_address.sin_addr)<=0) {
		printf("\nError: inet error\n");
		return 1;
	}

	if(cfd = connect(sfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
		printf("\nError: connection error, %s\n", strerror(errno));
		return 1;
	}

	//se
	char x;
	int idx = 0;
	int buff_idx = 0;
	char *uname = "user";
	char *password = "password";
	if(authenticate(uname, password, receiveBuffer, sendBuffer, sfd) == -1){
		exit(EXIT_FAILURE);
	}

    // read and send commands
    // if no specific command was specified.
	if(command != NULL) {
		// this has been set, let's just put it in the send buffer and be done with it
		if(strlen(command) > BUFFSIZE-1){
			printf("Error: command contains too many characters\n");
			exit(EXIT_FAILURE);
		}
		for(int i = 0; i < strlen(command); i++){
			sendBuffer[i] = command[i];
		}
	} else {
		// put this into "shell mode"
		char c;
		buff_idx =0;
		char done = 0;
		while(!done) {
			while((c = getchar()) != '\n') {
				sendBuffer[buff_idx++] = c;
				if(buff_idx == BUFFSIZE-1){
					sendBuffer[buff_idx] = '\0';
					//printf("\nbuff_idx: %d, %s, %ld\n", buff_idx, sendBuffer, strlen(sendBuffer));
					write(sfd, sendBuffer, strlen(sendBuffer));
					buff_idx = 0;
					wipe(sendBuffer);
				}
			}
			sendBuffer[buff_idx] = '\0';
			buff_idx = 0;
			if(write(sfd, sendBuffer, strlen(sendBuffer)) == -1) {
				printf("Error: %s\n", strerror(errno));
			}

			if(strcmp(sendBuffer, "exit")){
				done = 1;
			}

			wipe(sendBuffer);
		}
	}

	
	
	
	
	memset(sendBuffer, '\0', sizeof(sendBuffer));

	// this bit reads the socket for the response from the server
	if((read_loc = read(sfd, receiveBuffer, sizeof(receiveBuffer)-1)) > 0) {
		receiveBuffer[read_loc] = 0;
		if(fputs(receiveBuffer, stdout) == EOF) {
			printf("\nError: fputs is bad\n");
		}
	}

	if(read_loc < 0) {
		printf("\nError: reading error\n");
	}

	return 0;




}
