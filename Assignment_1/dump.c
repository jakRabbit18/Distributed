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

int myDump(int numNames, char ** filenames){
	// for every file in filenames
	// if it is a file remove it
	// if it is a directory:
	// 		obtain a list of the files in the directory
	// 		pass this list to myDump and collect the result
	//		if the result is success, continue
	// 		if the result was failure, return failure
	// if we finish the loop, we successfully removed everything (I hope)

}

int main(int argc, char ** argv){

	// first we need to find the dumpster
	// then we need to obtain a list of the top level items in the dumpster
	// we pass this list to the myDump function, which takes care of the rest



}