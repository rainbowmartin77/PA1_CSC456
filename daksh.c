#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/wait.h>

void eMessage(void) {
    char errorMessage[30] = "An error has occurred\n";
    write(STDERR_FILENO, errorMessage, strlen(errorMessage));
}

void exCommand(char* words[]) {
    // "exit" entered
    if (strcmp(words[0], "exit") == 0) {
        exit(0);
    }

    // cd command
    if (strcmp(words[0], "cd") == 0) {
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
                    perror("chdir");
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

        if(c == 1 && strcmp(words[1], "clear") == 0) {
            // clear the path
            setenv("PATH", "", 1);
        }

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
            }
        }
        printf("%s\n", getenv("PATH"));
    }

    // external commands
    else {
        pid_t pid = fork();

        if (pid == -1) {
            eMessage();
        }
        else if (pid == 0) {
            execvp(words[0], words);

            exit(0);
        }
        else {
            wait(NULL);
        }

    }
}

int main(int argc, char* argv[]) {

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
        ssize_t length;

        if (argc == 2) {

            FILE *file = fopen(argv[1], "r");

            // read input
            length = getline(&input, &capacity, file);
            printf("%s\n", input);

            fclose(file);
        }

        else if ( argc == 1) {
            // print prompt
            printf("daksh> ");

            // read input
            length = getline(&input, &capacity, stdin);
        }

        // error: EOF reached without reading characters
        if (length < 0){
            eMessage();
        }

        // empty string entered
        else if (strcmp(input, "\n") == 0) {
            // do nothing, start loop over
        }

        // string entered
        else {
            // replace newline with terminator
            input[length - 1] = '\0';
            char* word;
            char* words[10];

            // break the string into tokens
            int i = 0;
            while ((word = strsep(&input, " ")) != 0) {
                words[i] = word;
                i++;
            }

            exCommand(words);

            for(int j = 0; j < 10; j++) {
                words[j] = '\0';
            }

            free(input);
        }

        if (argc == 2) {
            argc = 1;
        }

    } while (run);

    return 0;
}

