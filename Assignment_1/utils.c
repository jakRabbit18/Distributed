// Everett Harding
// this file contains basic directory operations
// that are useful for multiple functions

#include "utils.h"

//return the number of files in the given directory
int getNumFiles(const char *directory, char *filename) {
	int numFiles = 0; // we don't already know if there's one of these files here
	DIR *dir;
	struct dirent *ent;
	// printf("File to count: %s\n", filename);
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

			// printf("Ent name: %s\n", ent->d_name);
			if(strcmp(name, filename) == 0 || strcmp(ent->d_name, filename) == 0) {
				// we've found another instance of this filename! increase our count
				numFiles ++;
			}
		}
	}

	// printf("Number of pre-exisitng files: %d\n", numFiles);
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

		// set dest permissions to match source 
		struct stat src_stat;
		struct stat dest_stat;
		int src_res = stat(source, &src_stat);
		int dest_res = stat(dest, &dest_stat);

		if(!src_res && !dest_res) {
			dest_res = chmod(dest, src_stat.st_mode);
		} else {
			printf("Stat ran into an issue, %s\n", strerror(errno));
			return -1;
		}

		// do the same with access times
		struct utimbuf dest_times;
		dest_times.actime = src_stat.st_atime;
		dest_times.modtime = src_stat.st_mtime;
		utime(dest, &dest_times);

		// a little clean-up
		fclose(destf);
		fclose(srcf);
		return 0;
	}
	printf("something else went wrong I guess\n");
	return -1;
}

int checkFileExt(const char *filename) {
	char *fileExt = strrchr(filename,'.');
	int ext = 0;
	if(fileExt){
		fileExt++;
		ext = atoi(fileExt);
	}
	// printf("File extention: %s\t%d\n", fileExt, ext);
	return ext;
}