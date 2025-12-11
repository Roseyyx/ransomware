#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    printf("Processing file: %s\n", entryPoint->d_name);

    char *filePath = malloc(strlen(directoryPath) + strlen(entryPoint->d_name) +
                            2); // idek what i cooked here

    if (filePath == NULL) {
      printf("Failed to allocate filePath.\n");
      continue;
    }

    sprintf(filePath, "%s/%s", directoryPath, entryPoint->d_name);

    // we open tha file
    FILE *filePointer = fopen(
        filePath, "rb"); // rb for nissan rb engine (no its for read binary lol)
    if (filePointer == NULL) {
      printf("Unable to open file: %s\n", filePath);
      free(filePath);
      continue;
    }

    fclose(filePointer);
    printf("Finished processing file: %s\n", entryPoint->d_name);

    free(filePath);
  }

  closedir(directoryPointer);
  return EXIT_SUCCESS;
}
