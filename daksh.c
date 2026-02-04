#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/wait.h>

// only error message for entire program
void eMessage(void);

void clearWords(char** words);

void exCommand(char* words[]);

void breakString(char** words, char* input, ssize_t length);

void breakCommands(char** multipleCommands, char* input, ssize_t length);

int main(int argc, char* argv[]) {
    // set initial path to /bin
    setenv("PATH", "/bin", 1);

    bool run = true;

    // program invoked with more than 2 arguments
    // exit program after error message
    if (argc > 2) {
        eMessage();
        exit(1);
    }

    do {
        char *input = NULL;
        size_t capacity = 0;
        ssize_t length = 0;
        char* words[10];
        char* multipleCommands[10];

        // if file entered as argument, enter batch mode
        if (argc == 2) {

            FILE *file = fopen(argv[1], "r");

            while ((length = getline(&input, &capacity, file))!= -1) {

                // read input
                breakString(words, input, length);
                words[length] = NULL;

                // next line exists
                // execute command
                exCommand(words);

                clearWords(words);   
            }

            fclose(file);
            exit(0);

            free(input);
        }

        // no file passed in, enter interactive mode
        else if (argc == 1) {
            // print prompt
            printf("daksh> ");

            // read input
            length = getline(&input, &capacity, stdin);

            if (strchr(input, '&')) {
                breakCommands(multipleCommands, input, length);
                //breakString(words, input, length);

                breakString(words, multipleCommands[0], strlen(multipleCommands[0]));
                
                exCommand(words);
                clearWords(words);
            }
            else {
                breakString(words, input, length);

                exCommand(words);

                clearWords(words);
            }
        }

    } while (run);

    return 0;
}

void eMessage(void) {
    char errorMessage[30] = "An error has occurred\n";
    write(STDERR_FILENO, errorMessage, strlen(errorMessage));
}

void breakString(char** words, char* input, ssize_t length) {
    // replace newline with terminator
    input[length - 1] = '\0';
    char* word;
    char del[] = {' ', '\t', '&'};

    // break the string into tokens
    int i = 0;
    while ((word = strsep(&input, del)) != 0) {
        if (strcmp(word, "") != 0) {
            words[i] = word;
            i++;
        }
    }
    // make sure list of words in null terminated
    words[i+1] = NULL;
    
    return;
}

void breakCommands(char** multipleCommands, char *input, ssize_t length) {
    // replace newline with terminator
    input[length - 1] = '\0';
    char* command;
    char del[] = {'&'};

    // break the string into tokens
    int i = 0;
    while ((command = strsep(&input, del)) != 0) {
        multipleCommands[i] = command;
        i++;
    }
    // make sure list of words in null terminated
    multipleCommands[i+1] = NULL;
    
    return;
}

void exCommand(char* words[]) {
    // "exit" entered
    if (strcmp(words[0], "exit") == 0) {
        exit(0);
    }

    // cd command
    else if (strcmp(words[0], "cd") == 0) {
        // variables to store directories
        char presentDirectory[60];
        getcwd(presentDirectory, 60);

        char previousDirectory[60];
        const char *home = "/home";
        char newDirectory[60];

        // count arguments to cd
        int c = 0;
        for(int i = 1; i < 3; i++){
            if (words[i] != NULL) {
                c++;
            }
        }

        // if there is not one argument, error
        if (c != 1) {
            eMessage();
        }

        else {
            // argument does not equal "~", go to directory entered
            if (strcmp(words[1], "~") != 0) {
                strcpy(newDirectory, words[1]);

                // make sure directory is valid
                int working = chdir(newDirectory);
                if( working != 0) {
                    eMessage();
                }
                // store current directory
                getcwd(presentDirectory, 60);
            }
            // arguemnt is "~" and redirects to home
            else if (strcmp(words[1], "~") == 0) {
                strcpy(newDirectory, "/home");
                chdir(newDirectory);
                getcwd(presentDirectory, 60);
            }
        }
    }

    // path command
    else if (strcmp(words[0], "path") == 0) {
        char* paths[20];

        // count arguments to path
        int c = 0;
        for(int i = 1; i < 10; i++){
            if (words[i] != NULL) {
                c++;
            }
        }

        // clearing PATH
        if(c == 1 && strcmp(words[1], "clear") == 0) {
            // clear the path
            setenv("PATH", "", 1);
        }

        // adding to PATH
        else {
            for (int i = 1; words[i] != NULL; i++) {
                char* presentPath = getenv("PATH");
                char path[50];
                strcpy(path, words[i]);
                char* newPath;
            
                if (strcmp(presentPath, "") != 0) {
                    size_t len = strlen(presentPath) + 1 + strlen(path) + 1;
                    newPath = malloc(len);
                    snprintf(newPath, len, "%s:%s", presentPath, path);
                    setenv("PATH", newPath, 1);
                    free(newPath);
                }
                else {
                    size_t len = strlen(presentPath) + 1 + strlen(path) + 1;
                    newPath = malloc(len);
                    snprintf(newPath, len, "%s", path);
                    setenv("PATH", newPath, 1);
                    free(newPath);
                }
            }
        }
    }

    // external commands
    else {
        pid_t pid = fork();

        if (pid == -1) {
            eMessage();
        }
        else if (pid == 0) {
            execvp(words[0], words);

            eMessage();
            _exit(0);
        }
        else {
            wait(NULL);
            clearWords(words);
        }

    }

    return;
}

void clearWords(char** words) {
    // clear words array
    for(int x = 0; words[x] != NULL; x++) {
        words[x] = NULL;
    }
    return;
}
