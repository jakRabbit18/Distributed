// Everett Harding
// header file for utility functions
#ifndef UTILS_H
#define UTILS_H

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <ftw.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

// extern char *dumpster;
int getNumFiles(const char *directory, char *filename);
int checkFileExt(const char *filename);

#endif /*!UTILS H*/