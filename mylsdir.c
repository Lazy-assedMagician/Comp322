/*************************
 * Name: Nereida Herrera
 * Class: Comp 322 Sp 16
 * Project 5 - mylsdir.c
 * Last Mod: 5/18/16
 * 
 * Summary: personal implementation of
 *         the ls command. For a given directory
 *         on the command line all visible subdirectories
 *         and their subdirectories are recursively
 *         displayed.
 *
 * Note: only takes in one directory as an argument.
 */

#include <stdio.h>
#include <stdlib.h>    // for using exit()
#include <sys/types.h>
#include <dirent.h>    // for directory functionality
#include <unistd.h>    
#include <errno.h>     // for error codes like EXIT_FAILURE
#include <limits.h>    // defines max length for a pathname (PATH_MAX)

char *cmdName; // command name

/********* Function Prototypes **********/
void listDir(const char *dirPath, int level);

int main(int argc, char *argv[]){
  cmdName = argv[0]; // get the command name
  
  // check for valid number of command arguments
  if(argc == 1){
    fprintf(stderr, "%s: Need at least one directory pathname\n", cmdName);
    exit(EXIT_FAILURE);
  }
  
  const char *dirName; // directory provided as command argument
  
  int i; // loop counter
  for(i = 1; i < argc; i++){
    dirName = argv[i];
    listDir(dirName, 0); // list subdirectories
  }
  
  return 0;
}

/********* Function Definitions **********/
void listDir(const char *dirPath, int level){
  struct dirent *currDir; // structure of entities in the directory stream
  DIR *dirStream;         // directory stream
  
  // attempt to open directory and read it
  if(!(dirStream = opendir(dirPath)))
    return;
  if(!(currDir = readdir(dirStream)))
    return;
    
  do{
    if(currDir->d_type == DT_DIR){ // is this a directory?
      // it's a directory so we make a new path name
      char path[PATH_MAX]; // max size for a pathname
      int len = snprintf(path, sizeof(path)-1, "%s/%s", dirStream, currDir->d_name); // store next path name as a C-string
      // length of new path is sizeof(path)-1 because we don't count the null terminator here
      path[len] = 0;
      if(strcmp(currDir->d_name, ".") == 0 || strcmp(currDir->d_name, "..") == 0) // check that we're not in the current directory or parent directory
        continue; // start a new loop iteration      
      printf("%*s%s\n", level*2, "", currDir->d_name);
      listDir(path, level+1);
    }
    else
      printf("%*s%s\n", level*2, "", currDir->d_name);
  }while(currDir = readdir(dirStream));
  
  closedir(dirStream);
}
