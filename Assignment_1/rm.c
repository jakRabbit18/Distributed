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


// TESTS:
//	- remove one file
// 	- remove multiple files
// 	- remove one directory
// 	- remove multiple directories
//  - remove multiple files/directories
//	- remove one file with multiple hard links
//	- remove multiple files with multiple hard links
//  - remove multiple files/directories with multiple hardlinks
//  - remove multiple files/directories across a disk with multiple hardlinks
//  - can you have more than one hard link to a directory? 
//		- if so, test removing one and multiple directories with multiple hardlinks


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <ftw.h>
#include <errno.h>
#include <libgen.h>
#include <ctype.h>

#define FALSE 0
#define TRUE 1

extern int errno;

//list which will hold paths of directories for later deletion
struct Node *start = NULL;
/* A linked list node */
//https://www.geeksforgeeks.org/generic-linked-list-in-c-2/
struct Node
{
    // Any data type can be stored in this node
    void  *data;
 
    struct Node *next;
};

char *dumpster;

int copy_item(const char *fpath, const struct stat *sb, int tflag);
int delete_dir(const char *fpath, const struct stat *sb, int tflag);
char get_extension(const char*);


/* Function to add a node at the beginning of Linked List.
   This function expects a pointer to the data to be added
   and size of the data type */
void push(struct Node** head_ref, void *new_data, size_t data_size)
{
    // Allocate memory for node
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
 
    new_node->data  = malloc(data_size);
    new_node->next = (*head_ref);
 
    // Copy contents of new_data to newly allocated memory.
    // Assumption: char takes 1 byte.
    int i;
    for (i=0; i<data_size; i++)
        *(char *)(new_node->data + i) = *(char *)(new_data + i);
 
    // Change head pointer as new node is added at the beginning
    (*head_ref) = new_node;
}

void delete_directories(struct Node *node){
	while(node != NULL){
		rmdir((char*)node->data);
		printf("node path: %s\n", (char*)node->data);
		node = node->next;
	}
}

// with reference from:
// https://stackoverflow.com/questions/2256945/removing-a-non-empty-directory-programmatically-in-c-or-c
int copy_item(const char *fpath, const struct stat *sb, int tflag){
	printf("path: %s\n", fpath);
	const char *dumpster = getenv("DUMPSTER");
	// thisis the bit that actually moves the file  the dumpster
	int dumpsterPathLength = strlen(dumpster);
	dumpsterPathLength += strlen(fpath) + 4; // to cover new null terminator, extra forward slash, possible num extension
	printf("Dumpster path length: %i\n", dumpsterPathLength);
	
	// build the new string for the dumpster pathname
	char *dumpsterPath = malloc(sizeof(char) * dumpsterPathLength);
	sprintf(dumpsterPath, "%s/%s", dumpster, fpath);
		printf("New Name: %s\n", dumpsterPath);
	//if we are walking over a directory, create a new empty dir under the dumpster
	if(tflag == FTW_D){
		struct stat st = {0};
		if(stat(dumpsterPath, &st) == -1){
			mkdir(dumpsterPath, 0700);
			push(&start, (void*)fpath, sizeof(char) * strlen(fpath) + 4);
		}
	}
	//if we are walking over a file, unlink it then link it to dumpster path
	else if(tflag == FTW_F){
		int result;
		if(!(access(dumpsterPath, F_OK) != -1)){
			result = rename(fpath, dumpsterPath);
		}
		printf("rename result: %d\n", result);
	}
	return 0;
}

//return the number of files in the given directory
int getNumFiles(const char *directory, char *filename) {
	int numFiles = 0; // we don't already know if there's one of these files here
	DIR *dir;
	struct dirent *ent;
	printf("File to count: %s\n", filename);
	// try to open the dumster directory,
	// this shouldn't be an issue by now, 
	// but just to be sure...
	if ((dir = opendir(directory))!=NULL) {
		//now we can loop over all the files in the directory
		while((ent = readdir(dir))!=NULL) {
			//for each file, we can check if the base directory name is a substring of that file name (this covers saving .1's)
			char name[strlen(ent->d_name)]; 
			strcpy(name, ent->d_name);
			
			int strlength = strlen(name);
			name[strlength-2] = '\0';
			// char *res = strstr(ent->d_name, filename);

			printf("Ent name: %s\n", ent->d_name);
			if(strcmp(name, filename) == 0 || strcmp(ent->d_name, filename) == 0) {
				// we've found another instance of this filename! increase our count
				numFiles ++;
			}
		}
	}

	printf("Number of pre-exisitng files: %d\n", numFiles);
	return numFiles;
}


// copies a given file from source to dest
int copyfile(char *source, char *dest) {
	FILE *srcf, *destf;
	int c;
	srcf = fopen(source, "r");
	destf = fopen(dest, "w");
	if(!srcf) {
		printf("Couldn't open the file %s to copy it to the dumpster\n", source);
		return -1;
	}
	if(!destf) {
		printf("Couldn't open the file %s to recieve the copy\n", dest);
		return -1;
	}

	if(destf && srcf) {
		while((c = getc(srcf))!= EOF) {
			putc(c, destf);
		}
		fclose(destf);
		fclose(srcf);
		return 0;
	}
	printf("something else went wrong I guess\n");
	return -1;
}

// the following is based on a solution for reading dir from StackOverflow answer here:
// https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
int removeFile(char *filename) {
		// this is the bit that actually moves the file  the dumpster
		int dumpsterPathLength = strlen(dumpster);
		dumpsterPathLength += strlen(filename) + 4; // to cover new null terminator, extra forward slash, possible num extension
		printf("Dumpster path length: %i\n", dumpsterPathLength);
		
		// build the new string for the dumpster pathname
		char *dumpsterPath = malloc(sizeof(char) * dumpsterPathLength);
		sprintf(dumpsterPath, "%s/%s", dumpster, basename(filename));
		
	
		// figure out if this ends in a number or not
		char *fileExt = strrchr(filename,'.');
		fileExt++;
		int ext;
		ext = atoi(fileExt);
		printf("File extention: %s\t%d\n", fileExt, ext);
		
		if(ext) {
			printf("we found a digit\n");
			// the end of the file was in fact a .num (we assume) so strip the extension
			int len = strlen(filename);
			filename[len-2] = '\0';
		}

		int numFiles = getNumFiles(dumpster, basename(filename));
		
		// if the extension was valid, we in business
		if(ext) {
			printf("we found a digit\n");
			// the end of the file was in fact a .num (we assume) so strip the extension
			numFiles ++;
			int len = strlen(dumpsterPath);
			dumpsterPath[len-2] = '\0';
		}

		if(numFiles && numFiles < 10){
			printf("%s\n", dumpsterPath);
			sprintf(dumpsterPath, "%s.%i", dumpsterPath, numFiles);
		} else if (numFiles) {
			// dumpster is full of this filename, print error and return
			// TODO
		}
		printf("New Name: %s\n", dumpsterPath);

		// now rename the file
		// just renaming doesn't do anything if the deleted file is on a separate disk
		// so we first add the new link, and then we delete the old link
		// TODO: add check for disk change
		struct stat file_stat;
		struct stat dump_stat;
		int file_res = stat(filename, &file_stat);
		int dump_res = stat(dumpster, &dump_stat);
		int linkResult, unlinkResult;

		if(file_res = 0) {
			printf("no stat for %s, %s\n", filename, strerror(errno));
		}
		if(!file_res && !dump_res && file_stat.st_dev != dump_stat.st_dev){
			// the files are on the same partition
			printf("File stat: %ld\tDump Stat: %ld\n", file_stat.st_dev, dump_stat.st_dev);
			int copyRes = copyfile(filename, dumpsterPath);
			if(copyRes) {
				printf("copy error, %s\n", strerror(errno));
				return -1;
			}
			unlinkResult= unlink(filename);
			if(unlinkResult) {
				printf("Unlink error: %s\n", strerror(errno));
				return -1;
			}

		} else {
			linkResult = link(filename, dumpsterPath);
			if(linkResult) {
				printf("Link error: %s\n", strerror(errno));
				return -1;
			}
			unlinkResult= unlink(filename);
			if(unlinkResult) {
				printf("Unlink error: %s\n", strerror(errno));
				return -1;
			}
			printf("link/unlink results: %i\t%i\n", linkResult, unlinkResult);
		}
		free(dumpsterPath);
		// TODO: return either link or unlink or combination of both
		return 0;
}

int removeAllFilesInList(int numfiles, char **filenames, const char force) {
	// get rid of all the files in the given list
	for(int i =0; i < numfiles; i ++){

		printf("file: %s\n", filenames[i]);

		// if force is on, just get rid of the file. Easy-peasy
		if(force){
			//here we punt to the og rm function
			remove(filenames[i]);
			continue;

		} else {
			removeFile(filenames[i]);
		}
	}
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
			int stat_res = stat(argv[i], &argStats);
			if(stat_res) {
				perror(strerror(errno));
			}
			// check if the arg is a filename
			if(!stat_res && S_ISREG(argStats.st_mode)){
				printf("Regular file: %s\n", argv[i]);
				filenames[numfiles++] = argv[i];
			} 

			//check if it is a directory
			else if(! stat_res && S_ISDIR(argStats.st_mode)) {
				// for now we'll just keep track of the directories
				// adding handling for them will come later. Probably a monday afternoon project
				dirnames[numDirs++] = argv[i];
				printf("dir: %s\n", argv[i]);
				ftw(argv[i], copy_item, 1);
				//delete remaining empty directories by referencing LL
				delete_directories(start);
			}

			//TODO: add error message if the pathname is neither a valid file or directory
		}
	}

	if(help) {
		// display the help message, then exit
		return 0;
	}

	// we do this after we parse everything else, in case the help flag was used
	// if no dumpster is declared, print an error and exit
	dumpster = getenv("DUMPSTER");
	if(dumpster == NULL){
		perror("No dumpster declared\n");
		return -1;
	}

	printf("Dumpster: %s\n", dumpster);
	printf("Force: %i\nHelp: %i\nRecursive: %i\n", force, help, recursive);
	printf("NumFiles: %i\n", numfiles);

	// remove all the top level filenames
	removeAllFilesInList(numfiles, filenames, force);

	free(filenames);
	free(dirnames);


}