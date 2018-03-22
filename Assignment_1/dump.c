/* Everett Harding
 * CS 4513 Distributed Systems
 * Assignment 1
 * 
 * this file contains the code to dump all the contents
 * of the dumpster
 * 
 * TODO: 
 * TESTS:
 * 	- basic dump of one-level files
 *  - basic dump with a directory present
 *  - basic dump with mult-level directories present
 *  - basic dump with multiple hardlinks to a specific file present
 * 		- not sure this will be a thing, if it's in the dumpster, it should only have one
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include "utils.h"

int countEntriesInDir(const char *dirname) {
	int numEntries = 0;
	DIR *dir;
	struct dirent *ent;
	// try to open the directory
	if((dir = opendir(dirname))!=NULL) {
		//we made it! now loop over all the items
		while((ent = readdir(dir)) != NULL) {
			numEntries ++;
		}
	}
	return numEntries;
}

int countDirsInDir(const char *dirname) {
	int numDirs = 0;
	DIR *dir;
	struct dirent *ent;
	// try to open the directory
	if((dir = opendir(dirname))!=NULL) {
		//we made it! now loop over all the items
		while((ent = readdir(dir)) != NULL) {
			struct stat entStat;
			stat(ent->d_name, &entStat);
			if(S_ISDIR(entStat.st_mode)) {
				numDirs ++;
			}
		}
	}
	return numDirs;
}

int removeFile(char *file) {
	int res = remove(file);
	return res;
}

int myDump(char *baseDir){
	printf("removing files from %s\n", baseDir);

	DIR *dir;
	struct dirent *ent;

	// try to open the dumpster
	if((dir = opendir(baseDir))!=NULL) {
		while((ent = readdir(dir)) != NULL) {
			// here we loop over all the entries in the 
			// directory and check if they're files/directories

			// make the filename friendly for playing with stat
			int ext = checkFileExt(ent->d_name);
			// char *name = malloc((sizeof(char) * strlen(ent->d_name)) + (sizeof(char) * strlen(baseDir)) + 1);
			char name[strlen(ent->d_name) + strlen(baseDir) + 2];
			sprintf(name, "%s/%s", baseDir, ent->d_name);
			// printf("full name: %s\n", name);
			if(ext) {
				int len = strlen(name);
				name[len-2] = '\0';
			}
			// printf("checked for ext: %s\n", name);
			struct stat entStat;
			int res = stat(name, &entStat);
			if(res != 0) {
				printf("%s\n", strerror(errno));
			}

			// Get rid of this entry if it is a file
			if(S_ISREG(entStat.st_mode)){
				int res = removeFile(name);
				// printf("Remove %s result: %d\n", name, res);
			}

			// call mydump on the entry if it is a directory
			else if(S_ISDIR(entStat.st_mode)) {
				printf("Directory name: %s\n", name);
				if(strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0) {
					int delete_error = myDump(name);
					if(delete_error) {
						return delete_error;
					}
					printf("deleting %s\n", name);
					rmdir(name);
				}
			}
			// free(name);
		}
		// closedir(dir);
	}

	// if((dir = opendir(baseDir))!=NULL) {
	// 	while((ent = readdir(dir)) != NULL) {
	// 		// here we loop over all the entries in the 
	// 		// directory and check if they're files/directories
	// 		printf("Current Entry: %s\n", ent->d_name);

	// 		// make the filename friendly for playing with stat
	// 		int ext = checkFileExt(ent->d_name);
	// 		char *name = malloc((sizeof(char) * strlen(ent->d_name)) + (sizeof(char) * strlen(baseDir)) + 1);
	// 		sprintf(name, "%s/%s", baseDir, ent->d_name);
	// 		printf("full name: %s\n", name);
	// 		if(ext) {
	// 			int len = strlen(name);
	// 			name[len-2] = '\0';
	// 		}

	// 		printf("checked for ext: %s\n", name);
	// 		struct stat entStat;
	// 		int res = stat(name, &entStat);

	// 		if(res != 0) {
	// 			printf("stat result: %d:", res);
	// 			printf("%s\n", strerror(errno));
	// 		}

	// 		if(S_ISDIR(entStat.st_mode)) {
	// 			printf("Directory name: %s\n", name);
	// 			if(strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0) {
	// 				printf("deleting...\n");
	// 				int success_delete = myDump(name);
	// 				rmdir(name);
	// 			}
	// 		}
	// 		free(name);
	// 	}
	// }

	return 0;
	
}

int main(int argc, char ** argv){

	// first we need to find the dumpster
	char *dumpster = getenv("DUMPSTER");
	printf("Dumpster path: %s\n", dumpster);

	// then we kick off the deletion of everything
	int res = myDump(dumpster);

	return 0;

}