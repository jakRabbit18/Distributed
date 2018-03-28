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


#define BUFFSIZE 1024
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

	// we have a connection! now we can start up the shell
	int read_res; 
	while(1) {
		while(!cfd) {
			cfd = accept(lfd, (struct sockaddr*) NULL, NULL);
		}
		printf("Connected!\n");
		// printf("%s\n", readBuffer);
		while((read_res = read(cfd, readBuffer, sizeof(readBuffer)-1)) > 0){
			readBuffer[read_res] = '\0';
			printf("%s, %ld, %d\n", readBuffer, strlen(readBuffer), read_res);
			memset(readBuffer, '\0', sizeof(readBuffer));
		}

		close(cfd);
		cfd = 0;
		memset(readBuffer, '\0', sizeof(readBuffer));
		
	}

	if(read_res < 0) {
		printf("read error\n");
	}

	close(cfd);
	return 0;
}
