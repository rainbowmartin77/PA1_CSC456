#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void eMessage(void) {
    char errorMessage[30] = "An error has occurred\n";
    write(STDERR_FILENO, errorMessage, strlen(errorMessage));
}

int main(int argc, char* argv[]) {

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

        if (length < 0){
            eMessage();
        }

        else {
            input[length - 1] = '\0';
        }



    } while (strcmp(input, "exit")!= 0);

    free(input);

    return 0;
}

