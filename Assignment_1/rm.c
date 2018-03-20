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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

#define FALSE 0
#define TRUE 1

int student_rm();
int copy_file();
char get_extension(const char*);

int student_rm(){
	return 0;
}



int main(int argc, char **argv) {

	char force = FALSE;
	char help = FALSE;
	char recursive = FALSE;
	// Check command line options
	
	char **filenames = malloc(sizeof(char *) * argc); // use this array to accumulate the filenames
	char **dirnames = malloc(sizeof(char *) * argc);  // use this array to accumulate the directories
	int numfiles = 0;
	int numDirs = 0;
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
			// check if the name is a filename, a directory, or nonexistent
			// check rm'd file exists

			struct stat argStats;
			stat(argv[i], &argStats);

			// check if the arg is a filename
			if(S_ISREG(argStats.st_mode)){
				printf("Regular file: %s\n", argv[i]);
				filenames[numfiles++] = argv[i];
			} 

			//check if it is a directory
			else if(S_ISDIR(argStats.st_mode)) {
				// for now we'll just keep track of the directories
				// adding handling for them will come later. Probably a monday afternoon project
				dirnames[numDirs++] = argv[i];
			}

			//TODO: add error message if the pathname is neither a valid file or directory
		}
	}

	if(help) {
		// display the help message, then exit
		return 0;
	}

	const char *dumpster = getenv("DUMPSTER");

	if(dumpster == NULL){
		return -1;
	}

	printf("Dumpster: %s\n", dumpster);
	printf("Force: %i\nHelp: %i\nRecursive: %i\n", force, help, recursive);
	printf("NumFiles: %i\n", numfiles);

	
	for(int i =0; i < numfiles; i ++){

		printf("file: %s\n", filenames[i]);

		// if force is on, just get rid of the file. Easy-peasy
		if(force){
			//here we punt to the og rm function
			remove(filenames[i]);
			continue;
		}

		// thisis the bit that actually moves the file  the dumpster
		int dumpsterPathLength = strlen(dumpster);
		dumpsterPathLength += strlen(filenames[i]) + 4; // to cover new null terminator, extra forward slash, possible num extension
		printf("Dumpster path length: %i\n", dumpsterPathLength);
		
		// build the new string for the dumpster pathname
		char *dumpsterPath = malloc(sizeof(char) * dumpsterPathLength);
		sprintf(dumpsterPath, "%s/%s", dumpster, filenames[i]);
		printf("New Name: %s\n", dumpsterPath);

		// the following is based on a solution for reading dir from StackOverflow answer here:
		// https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
		
		int numFiles = 0; // we don't already know if there's one of these files here
		DIR *dir;
		struct dirent *ent;
		// try to open the dumster directory,
		// this shouldn't be an issue by now, 
		// but just to be sure...
		if ((dir = opendir(dumpster))!=NULL) {
			//now we can loop over all the files in the directory
			while((ent = readdir(dir))!=NULL) {
				//for each file, we can check if the base dumpster name is a substring of that file name (this covers saving .1's)
				char *res = strstr(ent->d_name, filenames[i]);
				printf("Ent name: %s\n", ent->d_name);
				if(res !=NULL) {
					// we've found another instance of this filename! increase our count
					printf("file already exists %i times!\n", numFiles);
					numFiles ++;
				}
			}
			if(numFiles){
				sprintf(dumpsterPath, "%s.%i", dumpsterPath, numFiles);
			}
		}

		//now rename the file
		// just renaming doesn't do anything if the deleted file is on a separate disk
		// so we first add the new link, and then we delete the old link
		// TODO: add check for disk change
		int linkResult = link(filenames[i], dumpsterPath);
		int unlinkResult= unlink(filenames[i]);
		printf("link/unlink results: %i\t%i\n", linkResult, unlinkResult);

		free(dumpsterPath);
		free(filenames);
		free(dirnames);
	}


}