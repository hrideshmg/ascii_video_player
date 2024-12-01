#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    FILE *fptr;
    fptr = fopen("newline-test.txt", "r");
    if (fptr == NULL) {
        printf("Failed to open the file.\n");
        return 1;
    }

    int newline_count = 0;
    int total_size = 0;
    char ch;

    //count newlines and size for string arry size
    while ((ch = fgetc(fptr)) != EOF) {
        total_size++;
        if (ch == '\n') {
            newline_count++;
        }
    }

    // Allocate memory dynamically for the character array
    char *content = malloc(total_size + 1); // +1 for null terminator
    if (content == NULL) {
        printf("Memory allocation failed.\n");
        fclose(fptr);
        return 1;
    }

    rewind(fptr);

    fread(content, 1, total_size, fptr);
    content[total_size] = '\0';

    printf("Newline count: %d\n", newline_count);
    printf("Total size: %d bytes\n\n", total_size);

    printf("%s", content);

    sleep(2);
    for (int i = 0; i < newline_count; i++) {
        printf("\033[A\33[2K\r"); //one step up and clear
    }

    // Clean up
    free(content);
    fclose(fptr);
    return 0;
}
