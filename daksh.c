#include <stdio.h>
#include <string.h>


int main(void) {

    // character array to hold input
    char input[50];

    do {
        // print prompt
        printf("daksh> ");

        // read input
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

    } while (strcmp(input, "exit")!= 0);

    return 0;
}