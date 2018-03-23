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
#include <assert.h>
#include "utils.h"

#define FALSE 0
#define TRUE 1

/*
 * Tests:
 * ./dv file dne.txt
 * ./dv file exists.txt -> error
 * ./dv mydir
 * ./dv mydir/sub/sub2
 */

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

char* str_to_match;
char d_path[256];
int result = 0;

/* Function to add a node at the beginning of Linked List.
   This function expects a pointer to the data to be added
   and size of the data type */
void push(struct Node** head_ref, void *new_data, size_t data_size) {
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
    (*head_ref)    = new_node;
}

void chopN(char *str, size_t n) {
    assert(n != 0 && str != 0);
    size_t len = strlen(str);
    if (n > len)
        return;  // Or: n = len;
    memmove(str, str+n, len - n + 1);
}


int match_string(const char *fpath, const struct stat *sb, int tflag){
	if(tflag == FTW_F){
		printf("file path: %s\n", fpath);
		//get the name of the file
		char *name = basename((char*)fpath);
		//check if it equals str_to_match
		char *res = strstr(name, str_to_match);
		if(res != NULL){
			//file exists
			strncpy(d_path, fpath, 255);
			printf("d_path in match_string: %s\n", d_path);
			result = 1;
		} 
	}
	return 0;
}

int match_directory(const char* fpath, const struct stat *sb, int tflag){
	char *name = basename((char*)fpath);
	if(tflag == FTW_D && strcmp(name, str_to_match) == 0){
		strncpy(d_path, fpath, 255);
		printf("d_path in match_string: %s\n", d_path);
		result = 1;
	}
	return 0;
}

void delete_directories(struct Node *node){
	while(node != NULL){
		rmdir((char*)node->data);
		printf("node path: %s\n", (char*)node->data);
		node = node->next;
	}
}

int copy_directory(const char *fpath, const struct stat *sb, int tflag){
	result = 0;
	printf("path: %s\n", fpath);
	// this wall of text builds the path to which we need to restore the file/directory
	char buffer[256];
	const char *cwd = getcwd(buffer, sizeof(buffer));
	printf("string to match: %s\n", str_to_match);
	char pathBuff[strlen(fpath)];
	strncpy(pathBuff, fpath, strlen(fpath));
    chopN(pathBuff, strlen(getenv("DUMPSTER")));
	int refreshPathLength = strlen(cwd);
	refreshPathLength += strlen(pathBuff) + 4;
	char *refreshPath = malloc(sizeof(char) * refreshPathLength);
	sprintf(refreshPath, "%s%s", cwd, pathBuff);
	printf("new name: %s\n", refreshPath);
		
	//if we are walking over a directory, create a new empty dir under the dumpster
	if(tflag == FTW_D){
		struct stat st = {0};
		if(stat(refreshPath, &st) == -1){
			mkdir(refreshPath, 0700);
			push(&start, (void*)fpath, sizeof(char) * strlen(fpath) + 4);
		}
	}

	//if we are walking over a file, unlink it then link it to dumpster path
	else if(tflag == FTW_F) {
		int result;
		if(!(access(refreshPath, F_OK) != -1)) {
			char *cwd = get_current_dir_name();
			struct stat fstat;
			struct stat cwdstat;
			int fres = stat(fpath, &fstat);
			int cwdres = stat(cwd, &fstat);
			if(!fres && !cwdres && fstat.st_dev != cwdstat.st_dev) {
				// dest is a separate disk
				result = copyfile(fpath, refreshPath);
				unlink(fpath);
			} else {
				result = rename(fpath, refreshPath);
			}
			free(cwd);
		}
		printf("rename result: %d\n", result);
	}
	free(refreshPath);
	return 0;
}


int restore_files_walk(char** filenames, int numfiles) {
	printf("numfiles: %d\n", numfiles);
	for(int i =0; i < numfiles; i ++) {
		result = 0;
		printf("file: %s\n", filenames[i]);

		// thisis the bit that actually moves the file  the dumpster
		const char *dumpster = getenv("DUMPSTER");
		int dumpsterPathLength = strlen(dumpster);

		str_to_match = filenames[i];

		ftw(dumpster, match_string, 1);
		printf("d_path before copy: %s\n", d_path);
		if(result == 1) {
			// now rename the file
			// just renaming doesn't do anything if the deleted file is on a separate disk
			// so we first add the new link, and then we delete the old link
			// TODO: add check for disk change
			char buffer[256];
			const char *cwd = getcwd(buffer, sizeof(buffer));
			int refreshPathLength = strlen(cwd);
			refreshPathLength += strlen(filenames[i] + 4);
			char *refreshPath = malloc(sizeof(char) * refreshPathLength);
			sprintf(refreshPath, "%s/%s", cwd, filenames[i]);
			printf("new name: %s\n", refreshPath);
			
			// build the new string for the dumpster pathname
			//char *dumpsterPath = malloc(sizeof(char) * dumpsterPathLength);
			//sprintf(dumpsterPath, "%s/%s", dumpster, d_path);
			//printf("New Name: %s\n", dumpsterPath);
			printf("d_path: %s\n", d_path);
			// char *cwd = get_current_dir_name();
			struct stat fstat;
			struct stat cwdstat;
			int fres = stat(d_path, &fstat);
			int cwdres = stat(cwd, &fstat);
			int linkResult, unlinkResult;
			if(!fres && !cwdres && fstat.st_dev != cwdstat.st_dev) {
				// dest is a separate disk
				int linkResult = copyfile(d_path, refreshPath);
				unlink(d_path);
			} else {
				// result = rename(fpath, refreshPath);
				link(d_path, refreshPath);
				unlink(d_path);
			}
			// free(cwd);
			printf("link/unlink results: %i\t%i\n", linkResult, unlinkResult);

			//free(dumpsterPath);
			//free(filenames);		
		}
		else {
			//could not find the file
			printf("file does not exist in dumpster: %s\n", filenames[i]);
			return -1;
		}
	}
}

int restore_directory(char* name){
	result = 0;
	printf("dir: %s\n", name);

	// thisis the bit that actually moves the file  the dumpster
	const char *dumpster = getenv("DUMPSTER");
	int dumpsterPathLength = strlen(dumpster);

	str_to_match = name;

	ftw(dumpster, match_directory, 1);
	printf("d_path before copy: %s\n", d_path);
	if(result == 1){
		ftw(d_path, copy_directory, 1);
		delete_directories(start);
	}
	else{
		//could not find the file
		printf("dir does not exist in dumpster: %s\n", name);
		return -1;
	}
}


int main(int argc, char **argv) {

	char help = FALSE;
	char is_file_list = FALSE;
	// Check command line options
	char *dumpster = getenv("DUMPSTER");
	if(dumpster == NULL) {
		printf("No dumpser specified, specify dumpster to use dv");
		return -1;
	}

	char **filenames = malloc(sizeof(char *) * argc); // use this array to accumulate the filenames
	int numfiles = 0;
	for(int i = 0; i < argc; i++){
		printf("argv[%d]: %s\n", i, argv[i]);
		// if(strcmp(argv[i], "file") == 0){
		// 	is_file_list = TRUE;
		// }

		// else 
		if(strcmp(argv[i], "-h") == 0){
			help = TRUE;
		}

		else if(i > 0) {

			// since the file doesn't exist in this directory we need 
			// to check that it exists in the dumpster. For that we need
			// to figure out what the path in the dumpster would be
			// for now, lets assume that if you want a specific file
			// that isn't directly under the dumpster, you'll give the
			// path to that file relative to the directory under the dumpster
			// in which to look for it 
			char name[strlen(dumpster) + strlen(argv[i]) + 2]; // covers / and null term
			sprintf(name, "%s/%s", dumpster, argv[i]);

			struct stat fstat;
			int fres = stat(name, &fstat);

			if(fres) {
				perror(strerror(errno));
				continue;
			}

			if(S_ISREG(fstat.st_mode)) {
				printf("Regular file: %s\n", name);
				filenames[numfiles++] = argv[i];

			}
			else if (S_ISDIR(fstat.st_mode)) {
				printf("dir: %s\n", name);
				restore_directory(argv[i]);
			}
		}
	}

	if(is_file_list){
		printf("restore file list:\n");
		restore_files_walk(filenames, numfiles);
	}

}