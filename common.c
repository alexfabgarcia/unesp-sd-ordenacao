//
// Created by agarcia46 on 7/2/2017.
//

#define TRUE 1
#define FALSE 0

void exitWithFailure() {
    exit(EXIT_FAILURE);
}

/* Function to print an array */
void printArray(int arr[], int size) {
    int i;
    for (i = 0; i < size; i++)
        printf("Vetor[%d]: %d. ", i, arr[i]);
    printf("\n");
}

void error(const char *msg) {
    perror(msg);
    exitWithFailure();
}