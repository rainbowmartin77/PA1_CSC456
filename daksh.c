#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

void eMessage(void) {
    char errorMessage[30] = "An error has occurred\n";
    write(STDERR_FILENO, errorMessage, strlen(errorMessage));
}

int main(int argc, char* argv[]) {

    bool run = true;

    int root;

    root = chdir("/home/dsu");

    if (root != 0) {
        eMessage();
    }

    // program invoked with more than 2 arguments
    // exit program after error message
    if (argc > 2) {
        eMessage();
        exit(1);
    }

    // character array to hold input
    char *input = NULL;
    size_t capacity = 0;

    do {
        // print prompt
        printf("daksh> ");

        // read input
        ssize_t length = getline(&input, &capacity, stdin);

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

            // "exit" entered
            if (strcmp(words[0], "exit") == 0) {
                exit(0);
            }

            // cd command
            if (strcmp(words[0], "cd") == 0) {
                chdir(words[1]);
            }
        }

    } while (run);

    free(input);

    return 0;
}

