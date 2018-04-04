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

int main(int argc, char **argv) {
	//first set up some connection variables
	int sfd = 0, cfd = 0, read_loc=0;
	char recieveBuffer[BUFFSIZE];
	char sendBuffer[BUFFSIZE];
	struct sockaddr_in server_address;

	// set up the buffers and the socket
	memset(recieveBuffer, '0', sizeof(recieveBuffer));
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

	//read and send username
	char x;
	int idx = 0;
	int buff_idx = 0;
	char uname[50];
	while((x = getchar()) != '\n') {
		uname[idx++] = x;
		sendBuffer[buff_idx++] = x;
		if(idx == 49){
			uname[idx] = '\0';
			sendBuffer[buff_idx] = '\0';
			// printf("\nbuff_idx: %d, %s, %ld\n", buff_idx, sendBuffer, strlen(sendBuffer));
			printf("uname: %s\n", uname);
			write(sfd, sendBuffer, strlen(sendBuffer));
			buff_idx = 0;
			memset(sendBuffer,'\0', sizeof(sendBuffer));
		}
	}
	sendBuffer[buff_idx] = '\0';
	uname[idx] = '\0';
	// printf("\nbuff_idx: %d, %s, %ld\n", buff_idx, sendBuffer, strlen(sendBuffer));
	printf("uname: %s\n", uname);
	write(sfd, sendBuffer, strlen(sendBuffer));
	buff_idx = 0;
	memset(sendBuffer, '\0', sizeof(sendBuffer));

    //read and send command
	char c;
	buff_idx =0;
	while((c = getchar()) != '\n') {
		sendBuffer[buff_idx++] = c;
		if(buff_idx == BUFFSIZE-1){
			sendBuffer[buff_idx] = '\0';
			//printf("\nbuff_idx: %d, %s, %ld\n", buff_idx, sendBuffer, strlen(sendBuffer));
			write(sfd, sendBuffer, strlen(sendBuffer));
			buff_idx = 0;
			memset(sendBuffer,'\0', sizeof(sendBuffer));
		}
	}
	sendBuffer[buff_idx] = '\0';
	// printf("\nbuff_idx: %d, %s, %ld\n", buff_idx, sendBuffer, strlen(sendBuffer));
	write(sfd, sendBuffer, strlen(sendBuffer));
	buff_idx = 0;
	memset(sendBuffer, '\0', sizeof(sendBuffer));

	// this bit reads the socket for the response from the server
	if((read_loc = read(sfd, recieveBuffer, sizeof(recieveBuffer)-1)) > 0) {
		recieveBuffer[read_loc] = 0;
		if(fputs(recieveBuffer, stdout) == EOF) {
			printf("\nError: fputs is bad\n");
		}
	}

	if(read_loc < 0) {
		printf("\nError: reading error\n");
	}

	return 0;




}
