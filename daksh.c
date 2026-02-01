#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

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
        //reallocate memory for an additional word's pointer
        char** temporary = realloc(words, (*count + 1) * sizeof(char*));
        
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

            int count = 0;

            char** words = breakString(input, &count);

            for(int i = 0; i < count; i++) {
                printf("%s\n", words[i]);
            }
            
            free(words);
        }



    } while (strcmp(input, "exit")!= 0);

    free(input);

    return 0;
}

