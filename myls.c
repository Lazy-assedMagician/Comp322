/******************
 * Name: Nereida Herrera
 * Class: Comp 322 T/Th 2pm
 * Project 4 - myls.c
 * Last Mod: 5/18/16
 * Summary: Personal implementation of the UNIX command ls -l.
 *          User's permissions are displayed for each file
 *          on the command line in the order read, write, execute, 
 *          followed by the user's name.
 *
 * NOTE:   This program needs filenames or file pathnames to files in
 *         order to display the desired results. Will not work if 
 *         no filenames or pathnames are specified. 
 *         Also will not work with directories, only files.s
 *
 * Remark: the function fstatat() from sys/stat.h is used because
 *         it enables us to interpret a relative pathname, relative 
 *         to the path defined by the directory file descriptor
 *         argument of the function.
 *****************************/

#include <stdio.h>
#include <sys/stat.h>  // access to the stat() function to display file status
#include <sys/types.h> // includes definitions for various types, including those for user id, group id, etc.
#include <pwd.h>       // includes functions and data structures for accessing the system user database
#include <unistd.h>    // used for getting the current user's user ID
#include <dirent.h>    // library for format of directory entries
#include <errno.h>     // contains error code macros; used for deducing fstatat errors
#include <stdlib.h>    // for use of exit() when an error occurs
#include <fcntl.h>     // contains definitions for file access modes used with open()


/********** Function Prototypes **********/
void printPermissions(struct stat currFile, struct passwd user);

int main(int argc, char *argv[]){
  char *cmdName = argv[0]; // command name
  
/********** Check that there is at least one file **********/ 
  if(argc == 1){
    fprintf(stderr, "%s: Need at least one file to read from\n", cmdName);
    exit(EXIT_FAILURE);
  } 
/********** Variables to hold user and file info **********/
  struct stat fileStat;   // holds information about a file based on its filepath
  
  char* userDir;          // holds home directory of user
  struct passwd *user;    // holds information about a user
  uid_t userID;           // holds current user ID
  char *userName;         // holds name of user

/********** Get user info **********/  
  userID = getuid();      // gets real user ID of the calling process
  user = getpwuid(userID);// find the user and get their info
  userDir = user->pw_dir;     // get user home directory
  userName = user->pw_name;  

/********** Get file descriptor of user home directory **********/ 
/* Remark: this is a necessary step in order to use fstatat() 
*          correctly. Once we have the file descriptor of the
*          user's home directory we'll be able to interpret 
*          relative pathnames as paths relative to the 
*          user's home directory.
*/
  int homeFD;      // user home directory file descriptor
  DIR* homeStream; // directory stream for user home directory
  
  homeStream = opendir(userDir); // get the directory stream
  
  if(homeStream != NULL)
    homeFD = dirfd(homeStream); // get file descriptor for user home directory
  else{
    fprintf(stderr,"%s: unable to open user home directory %s!\n", cmdName, userDir); 
    return 1;
  }
  
/********** Start processing command arguments (files) **********/
  int i;
  int fstatatErr; // for error checking fstatat
  //int currFD;     // file descriptor for the current file we're looking at
 
  for(i = 1; i < argc; i++){
    fstatatErr = fstatat(homeFD, argv[i], &fileStat, 0); // get file status of current file
    
    // error checking results of fstatat()
    switch(fstatatErr){
      case EBADF:
        fprintf(stderr,"%s: dirfd is not a valid file descriptor\n", cmdName);
        closedir(homeStream);   // close user home directory
        exit(EXIT_FAILURE);
        break;
      case EINVAL:
        fprintf(stderr, "%s: invalid flag specified in flags\n", cmdName);
        closedir(homeStream);
        exit(EXIT_FAILURE);
        break;
      case ENOTDIR:
        fprintf(stderr, "%s: %s is a relative path and dirfd is a file descriptor referring to a file other than a directory\n", cmdName, argv[i]);
        closedir(homeStream);
        exit(EXIT_FAILURE);
        break;
      case -1:
        fprintf(stderr, "%s: relative pathnames are treated as relative to user's home directory: %s\n", cmdName, userDir);
        closedir(homeStream);
        exit(EXIT_FAILURE);
        break;
      default:
        break; // file status obtained successfully
    }    
    printPermissions(fileStat, *user); // print user's permissions
    printf(" %s %s\n", userName, argv[i]);          // print current file's name
  }
 
  closedir(homeStream); // close user home directory
  return 0;
}

/********** Function Definitions **********/

/* Remark: bit masking is used to decipher the mode bits of the current file*/
void printPermissions(struct stat currFile, struct passwd user){
/* File mode bits are as follows:
 *
 * S_IRUSR = read by owner
 * S_IWUSR = write by owner
 * S_IXUSR = execute/search by owner
 *
 * S_IRGRP = read by group
 * S_IWGRP = write by group
 * S_IXGRP = execute/search by group
 * 
 * S_IROTH = read by others
 * S_IWOTH = write by others
 * S_IXOTH = execute/search by others
 */

  mode_t fileMode = currFile.st_mode; // get the current file's mode (can derive permission bits from this)
  
  // user and owner IDs
  uid_t ownerID = currFile.st_uid;
  uid_t userID = user.pw_uid;
  
  // user and owner group IDs
  gid_t ownerGroup = currFile.st_gid;
  gid_t userGroup = user.pw_gid;
 
  // is the user the owner?
  if(userID == ownerID){
    printf(fileMode & S_IRUSR ? "r" : "-"); // decipher owner read bit
    printf(fileMode & S_IWUSR ? "w" : "-"); // decipher owner write bit
    printf(fileMode & S_IXUSR ? "x" : "-"); // decipher owner execute/search bit
  } 
  // is the user part of the group of the owner?
  else if(userGroup == ownerGroup){
    printf(fileMode & S_IRGRP ? "r" : "-"); // decipher group read bit
    printf(fileMode & S_IWGRP ? "w" : "-"); // decipher group write bit
    printf(fileMode & S_IXGRP ? "x" : "-"); // decipher group execute/search bit
  }
  else{ // user is part of 'others'
    printf(fileMode & S_IROTH ? "r" : "-"); // decipher others read bit
    printf(fileMode & S_IWOTH ? "w" : "-"); // decipher others write bit
    printf(fileMode & S_IXOTH ? "x" : "-"); // decipher others execute/search bit
  }  
}
