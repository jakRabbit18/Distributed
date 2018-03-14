// Everett Harding
// Distributed Systems
// Programming Assignment 1

// rm moves the designated file (from command line) to a
// "Dumpster" from which the file can be recovered if the 
// user wishes. It accepts the following command line options:
// 	-f: force complete remove (punts to system rm)
//  -r: recursive dumpster remove for directories
//  -h: display help and usage message

// Additionally, it will take 1..* filenames in the command line

// TODO:
// 		- if a directory is specified to be removed but not recursively, this is an error
// 		- do we need to understand the order of options/filenames:
//			rm foo.txt -f bar.c
//			 does the above only force remove bar.c? Does it throw an error?


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define FALSE 0
#define TRUE 1

int main(int argc, char **argv) {

	char force = FALSE;
	char help = FALSE;
	char recursive = FALSE;
	// Check command line options
	
	char *filenames[10];
	int numfiles = 0;
	for(int i = 0; i < argc; i++){
		printf("argv[%d]: %s\n", i, argv[i]);
		if(strcmp(argv[i], "-f") == 0){
			force = TRUE;
		}

		else if(strcmp(argv[i], "-h") == 0){
			help = TRUE;
		}

		else if(strcmp(argv[i], "-r") == 0){
			recursive = TRUE;
		}

		else if(i > 0) {
			filenames[numfiles++] = argv[i];
		}
	}

	const char *dumpster = getenv("DUMPSTER");

	printf("Dumpster: %s\n", dumpster);
	printf("Force: %i\nHelp: %i\nRecursive: %i\n", force, help, recursive);
	printf("NumFiles: %i\n", numfiles);

	if(force){
		//here we punt to the og rm function
		int x = 1;
	}
	
	for(int i =0; i < numfiles; i ++){
		printf("file: %s\n", filenames[i]);

		// thisis the bit that actually moves the file  the dumpster
		int dumpsterPathLength = strlen(dumpster);
		dumpsterPathLength += strlen(filenames[i]) + 4; // to cover new null terminator, extra forward slash, possible num extension
		printf("Dumpster path length: %i\n", dumpsterPathLength);
		
		// build the new string for the dumpster pathname
		char *dumpsterPath = malloc(sizeof(char) * dumpsterPathLength);
		sprintf(dumpsterPath, "%s/%s", dumpster, filenames[i])
		// strcat(dumpsterPath, dumpster);
		// strcat(dumpsterPath, "/");
		// strcat(dumpsterPath, filenames[i]);
		printf("New Name: %s\n", dumpsterPath);

		//check if the file already exists
		int fileExists = access(dumpsterPath, F_OK);
		if(fileExists == 0) {
			printf("file already exists!\n");
			strcat(dumpsterPath, ".1");
		} 
		
		//now rename the file
		int result = rename(filenames[i], dumpsterPath);
		printf("rename result: %i\n", result);

		free(dumpsterPath);
	}


}