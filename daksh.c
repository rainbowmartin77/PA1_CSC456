#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

void eMessage(void) {
    char errorMessage[30] = "An error has occurred\n";
    write(STDERR_FILENO, errorMessage, strlen(errorMessage));
}

char** breakString(char* inputString, int *count) {

    // array of pointers to each word
    char** words = NULL;
    // pointer to individual word
    char* word;
    *count = 0;

    // get the first word
    word = strtok(inputString, " ");

    // while there is another word waiting
    while (word != NULL) {
        int newCount = *count + 1;

        //reallocate memory for an additional word's pointer
        char** temporary = realloc(words, (newCount + 1) * sizeof(char*));
        
        // make the words array the newly allocated array
        words = temporary;
        // insert the word in the next open spot of the array
        words[*count] = word;
        // increment counter
        (*count)++;

        words[*count] = NULL;

        // get the next word
        word = strtok(NULL, " ");
    }

    return words;
}

int main(int argc, char* argv[]) {

    bool run = true;

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

            // break the string into tokens
            //int count = 0;
            //char** words = breakString(input, &count);
            int i = 1;
            while ((word = strsep(&input, " ")) != 0) {
                printf("Token %d is %s\n", i, word);
                i++;
            }

            // "exit" entered
            //if (strcmp(words[0], "exit") == 0) {
            //    exit(0);
            //}



            //free(words);
        }



    } while (run);

    free(input);

    return 0;
}

