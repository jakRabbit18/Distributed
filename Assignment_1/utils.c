// Everett Harding
// this file contains basic directory operations
// that are useful for multiple functions

#include "utils.h"

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

int checkFileExt(const char *filename) {
	char *fileExt = strrchr(filename,'.');
	fileExt++;
	int ext;
	ext = atoi(fileExt);
	printf("File extention: %s\t%d\n", fileExt, ext);
	return ext;
}