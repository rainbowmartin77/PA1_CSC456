#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>

// only error message for entire program
void eMessage(void);

void clearWords(char** words);

void exCommand(char* words[], char presentDirectory[], char** outputFile, int* f);

void breakString(char** words, char* input, ssize_t length);

void breakCommands(char** multipleCommands, char* input, ssize_t length);

void parallelCommands(char** multipleCommands, char* words[], char* input, ssize_t length, char presentDirectory[], char** outputFile, int* flag);

void redirectIncluded(char** words, char** outputFile, char* input, char presentDirectory[], int* flag);

bool checkRedirect (char* input);

int main(int argc, char* argv[]) {
    // set initial path to /bin
    setenv("PATH", "/bin", 1);
    int flagVal = 0;
    int *flag = &flagVal;

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
        char* file;
        char* outputFile[20];
        char* multipleCommands[10];
        char presentDirectory[60];
        

        getcwd(presentDirectory, 60);

        // if file entered as argument, enter batch mode
        if (argc == 2) {

            FILE *inputFile = fopen(argv[1], "r");

            if (inputFile == NULL) {
                eMessage();
                exit(1);
            }

            while ((length = getline(&input, &capacity, inputFile))!= -1) {

                if (strchr(input, '&')) {
                    parallelCommands(multipleCommands, words, input, length, presentDirectory, outputFile, flag);
                }
                else {
                    // if command has redirect
                    if (strchr(input, '>')) {
                        bool good = checkRedirect(input);

                        if (good == true) {
                            redirectIncluded(words, outputFile, input, presentDirectory, flag);
                            exCommand(words, presentDirectory, outputFile, flag);
                            clearWords(words);
                        }
                    }
                    
                    
                    else {
                        breakString(words, input, length);
                        if (strcmp(words[0], "exit") == 0) {
                            // leave loop to close file if exit is read
                            break;
                        }
                        exCommand(words, presentDirectory, outputFile, flag);
                        clearWords(words);
                    }
                }  
            }
            fclose(inputFile);
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
                parallelCommands(multipleCommands, words, input, length, presentDirectory, outputFile, flag);
            }
            else {
                if (strchr(input, '>')) {
                    bool good = checkRedirect(input);

                    if (good == true) {
                        redirectIncluded(words, outputFile, input, presentDirectory, flag);
                        exCommand(words, presentDirectory, outputFile, flag);
                        clearWords(words);
                    }
                }
                else {
                    breakString(words, input, length);
                    exCommand(words, presentDirectory, outputFile, flag);
                    clearWords(words);
                }
            }
        }

    } while (run);

    return 0;
}

void parallelCommands(char** multipleCommands, char* words[], char* input, ssize_t length, char presentDirectory[], char** outputFile, int*flag) {
    clearWords(words);
    breakCommands(multipleCommands, input, length);
    int children = 0;

    int parentPipe[2];
    int childPipe[2];

    pipe(parentPipe);
    pipe(childPipe);

    pid_t pid;

    for (int m = 0; multipleCommands[m] != NULL; m++) {
        children++;
    }

    for (int proc = 0; proc < children; proc++) {

        pid = fork();

        if (pid == 0) {
            int flags = fcntl(childPipe[0], F_GETFL, 0);
            fcntl(childPipe[0], F_SETFL, flags | O_NONBLOCK);

            // if information available in child pipe (ie another child executed cd)
            // update the current directory of this process
            ssize_t info = read(childPipe[0], presentDirectory, 60);
            close(childPipe[0]);
            if (info > 0) {
                chdir(presentDirectory);
            }

            if (proc > 0) {
                length = strlen(multipleCommands[proc]) + 1;
            }

            // one command has redirect
            if (strchr(multipleCommands[proc], '>')) {
                // if command has redirect
                redirectIncluded(words, outputFile, input, presentDirectory, flag);
                exCommand(words, presentDirectory, outputFile, flag);
                clearWords(words);
            }
            else {
                breakString(words, multipleCommands[proc], length);

                // if a child process is executing cd
                if (strcmp(words[0], "cd") == 0) {
                    exCommand(words, presentDirectory, outputFile, flag);
                    clearWords(words);
                    // write to child pipe
                    write(childPipe[1], presentDirectory, strlen(presentDirectory) + 1);
                    close (childPipe[1]);
                    // write to the parent process
                    write(parentPipe[1], presentDirectory, strlen(presentDirectory) + 1);
                    close(parentPipe[1]);
                }

                // execute command normally if not cd
                if (strcmp(words[0], "cd") != 0) {
                    exCommand(words, presentDirectory, outputFile, flag);
                    clearWords(words);
                }
            }

            _exit(0);
        }
        else if (pid < 0) {
            eMessage();
            exit(1);
        }

    }

    // wait for all child processes to end
    for (int x = 0; x < children; x++) {
        wait(NULL);
    }
    int flags = fcntl(childPipe[0], F_GETFL, 0);
    fcntl(parentPipe[0], F_SETFL, flags | O_NONBLOCK);

    // if information available in parent pipe (ie cd was executed)
    // update the current directory of this process
    ssize_t info = read(parentPipe[0], presentDirectory, 60);
    close(parentPipe[0]);
    if (info > 0) {
        chdir(presentDirectory);
    }
    
    clearWords(multipleCommands);

    return;
}

void eMessage(void) {
    char errorMessage[30] = "An error has occurred\n";
    write(STDERR_FILENO, errorMessage, strlen(errorMessage));
}

void redirectIncluded(char** words, char** outputFile, char* input, char presentDirectory[], int* flag) {
    char* redirectCommand;
    char* redirectLines[3];
    char* lastFile = NULL;

    if(outputFile != NULL) {
        lastFile = outputFile[0];
    }

    char del[] = {">"};
    // if command has redirect
    if (strchr(input, '>')){
        // separate command from output file
        int i = 0;
        while ((redirectCommand = strsep(&input, del))!= 0) {
            char* first = redirectCommand;
            while(isspace((unsigned char)*first)){
                first++;
            }
            redirectCommand = first;
            redirectLines[i] = redirectCommand;
            i++;
        }
        redirectLines[2] = NULL;

        breakString(words, redirectLines[0], strlen(redirectLines[0]));
        breakString(outputFile, redirectLines[1], strlen(redirectLines[1]));

        if(lastFile != NULL) {
            if(strcmp(lastFile, outputFile[0]) != 0) {
                *flag = 0;
            }
        }

    }
}

bool checkRedirect (char* input) {
    char* redir;
    char del[] = {">"};
    bool good = true;
    // if command has redirect
    // separate command from output file
    int i = 0;
    while ((redir = strsep(&input, del))!= 0) {
        i++;
    }
    if (i > 2) {
        eMessage();
        good = false;
    }

    return good;
}

void breakString(char** words, char* input, ssize_t length) {
    // replace newline with terminator
    input[length - 1] = '\0';
    char* word;
    char del[] = {' ', '\t'};

    // break the string into tokens
    int i = 0;
    while ((word = strsep(&input, del)) != 0) {
        if ((strcmp(word, "") != 0)) {
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
        char* first = command;
        while(isspace((unsigned char)*first)){
            first++;
        }
        command = first;
        multipleCommands[i] = command;
        i++;
    }
    // make sure list of words in null terminated
    multipleCommands[i+1] = NULL;
    
    return;
}

void exCommand(char* words[],  char presentDirectory[], char** outputFile, int* f) {

    // "exit" entered
    if (strcmp(words[0], "exit") == 0) {
        
        exit(0);
    }

    // cd command
    else if (strcmp(words[0], "cd") == 0) {
        // variables to store directories
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
        int flagPipe[2];
        pipe(flagPipe);

        pid_t pid = fork();
        int output;

        if (pid == -1) {
            eMessage();
        }
        else if (pid == 0) {
            close(flagPipe[0]);

            if (outputFile[0] != NULL && *f == 0) {

                output = open(outputFile[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);

                *f = 1;
                write(flagPipe[1], f, sizeof(int));
                close(flagPipe[1]);

                if (output == -1) {
                    eMessage();
                    _exit(0);
                }

                if (dup2(output, STDOUT_FILENO) == -1) {
                    eMessage();
                    _exit(0);
                }

                close(output);
                execvp(words[0], words);
                eMessage();
                _exit(0);
            }

            /*else if (outputFile[0] != NULL && *f == 1) {
                printf("test\n");
            }*/

            else {
                execvp(words[0], words);
                eMessage();
                _exit(0);
            }
        }
        else {
            wait(NULL);
            clearWords(words);

            int flags = fcntl(flagPipe[0], F_GETFL, 0);
            fcntl(flagPipe[0], F_SETFL, flags | O_NONBLOCK);

            // if information available in parent pipe (ie cd was executed)
            // update the current directory of this process
            ssize_t info = read(flagPipe[0], f, sizeof(int));
            close(flagPipe[0]);
            
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
