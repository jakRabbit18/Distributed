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
	// for every file in filenames
	// if it is a file remove it
	// if it is a directory:
	// 		obtain a list of the files in the directory
	// 		pass this list to myDump and collect the result
	//		if the result is success, continue
	// 		if the result was failure, return failure
	// if we finish the loop, we successfully removed everything (I hope)

	// then we need to obtain a list of the top level items in the dumpster

	char **collecteddirs;
	int numEntries = countDirsInDir(baseDir);

	collecteddirs = malloc(sizeof(char *) *numEntries);

	int numDirs = 0;
	DIR *dir;
	struct dirent *ent;
	// try to open the dumpster
	if((dir = opendir(baseDir))!=NULL) {
		while((ent = readdir(dir)) != NULL) {
			// here we loop over all the entries in the 
			// directory and check if they're files/directories
			struct stat entStat;
			int res = stat(ent->d_name, &entStat);
			printf("Current entry: %s\n", ent->d_name);
			if(res != 0) {
				printf("stat result: %d:", res);
				printf("%s\n", strerror(errno));
			}

			// Get rid of this entry if it is a file
			if(S_ISREG(entStat.st_mode)){
				printf("Attempt to remove %s\n", ent->d_name);
				int res = removeFile(ent->d_name);
				printf("Remove %s result: %d\n", ent->d_name, res);
			}

			// Call myDump on this if it is a directory. Once it returns, delete it
			else if(S_ISDIR(entStat.st_mode)) {
				int name_len = strlen(ent->d_name) + 1;
				char *name = malloc(sizeof(char) * name_len);
				name = strcpy(name, ent->d_name);
				printf("Directory name: %s\n", name);
				collecteddirs[numDirs] = name;
				if(strcmp(name, "..") != 0 && strcmp(name, ".") != 0){
					printf("deleting...\n");
					int success_delete = myDump(name);
					rmdir(name);
				}
			}
		}
	}

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