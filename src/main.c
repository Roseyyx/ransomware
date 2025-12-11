#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  // struct pointer to dirent header file
  struct dirent *entryPoint;
  // Directory pointer
  DIR *directoryPointer;

  // Directory path (. = current directory)
  const char *directoryPath = ".";

  directoryPointer = opendir(directoryPath);
  if (directoryPointer == NULL) {
    printf("Failed to open directory : %s\n", directoryPath);
    return EXIT_FAILURE;
  }

  while ((entryPoint = readdir(directoryPointer)) != NULL) {
    // print all entries
    printf("File found: %s\n", entryPoint->d_name);
  }

  closedir(directoryPointer);
  return EXIT_SUCCESS;
}
