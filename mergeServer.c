/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // defines POSIX constants
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // defines IP standard protocols
#include <arpa/inet.h> // to convert host addresses

#define BUFFER_SIZE 256
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

int posicoesVetor(int size, int posicao_atual) {
    return ((size < BUFFER_SIZE) || (posicao_atual + BUFFER_SIZE) > size) ? size - posicao_atual : BUFFER_SIZE;
}

int receberVetor(int clientsockfd, int **vetor) {
    int tamanhoVetor, bytes_transfer;
    int ok = 1, erro = 0;
    //free(*vetor);

    bytes_transfer = read(clientsockfd, &tamanhoVetor, sizeof(int));

    if (bytes_transfer < 0) {
        puts("Nenhum byte recebido.");
    } else {
        printf("Tamanho do vetor: %d (%d bytes).\n", tamanhoVetor, bytes_transfer);

        puts("Enviando confirmacao de recebimento do tamanho do vetor.");
        bytes_transfer = write(clientsockfd, &ok, sizeof(int));

        if (bytes_transfer < 0) {
            puts("Erro ao confirmar recebimento do tamanho do vetor.");
        } else {
            puts("Aguardando vetor.");
            *vetor = (int*) malloc(tamanhoVetor * sizeof(int));

            for (int i = 0; i < tamanhoVetor; i += BUFFER_SIZE) {
                bytes_transfer = posicoesVetor(tamanhoVetor, i);

                printf("Recebendo posicoes de %d a %d.\n", i, i + bytes_transfer);
                bytes_transfer = read(clientsockfd, *vetor + i, bytes_transfer * sizeof(int));

                if (bytes_transfer < 0) {
                    printf("Nenhum byte recebido.");
                    *vetor = NULL;
                    return 0;
                }
            }

            printf("Vetor recebido (%d posicoes).\n", tamanhoVetor);

            puts("Enviando confirmacao de recebimento do vetor.");
            bytes_transfer = write(clientsockfd, &ok, sizeof(int));

            if (bytes_transfer < 0) {
                puts("Nao foi possivel confirmar o recebimento do vetor.");
            } else {
                puts("Confirmado o recebimento do vetor.\n");
                //printArray(*vetor, tamanhoVetor);
            }
        }
    }

    return tamanhoVetor;
}

void enviarVetor(int sockfd, int **vetor, int size) {
    int bytes_transfer;

    for (int i = 0; i < size; i += BUFFER_SIZE) {

        // Calcula a quantidade para transferir
        bytes_transfer = posicoesVetor(size, i);

        printf("Enviando posicoes de %d a %d.\n", i, i + bytes_transfer);
        bytes_transfer = write(sockfd, *vetor + i, bytes_transfer * sizeof(int));

        if (bytes_transfer < 0) {
            error("Nao foi possivel enviar o vetor ordenado ao cliente..");
            return;
        }
    }
}

// Merges two subarrays of arr[].
// First subarray is arr[l..m]
// Second subarray is arr[m+1..r]
void merge(int arr[], int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 =  r - m;

    /* create temp arrays */
    int L[n1], R[n2];

    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1+ j];

    /* Merge the temp arrays back into arr[l..r]*/
    i = 0; // Initial index of first subarray
    j = 0; // Initial index of second subarray
    k = l; // Initial index of merged subarray
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    /* Copy the remaining elements of L[], if there
       are any */
    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    /* Copy the remaining elements of R[], if there
       are any */
    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, porta, bytes_transfer;
    int *vetor;
    int tamanhoVetor;
    int ok = 1, erro = 0;
    socklen_t clilen;
    char buffer[BUFFER_SIZE];
    char send_data[BUFFER_SIZE], recv_data[BUFFER_SIZE];
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr, "[Erro] informe o numero da porta como argumento.\n");
        exitWithFailure();
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error("[Erro] Nao foi possivel abrir o socket");
    }

//    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &1, sizeof(int)) == -1) {
//        error("[Erro] Setsockopt");
//    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    porta = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(porta);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("[Erro] Nao foi possivel realizar o binding");
    }

    if (listen(sockfd, 5) == -1) {
        error("[Erro] Erro ao iniciar escuta");
    }

    while (TRUE) {
        printf("\nServidor de merge esperando por cliente na porta %d (...)\n", porta);
        fflush(stdout);

        clilen = sizeof(struct sockaddr_in);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        printf("\nRecebendo conexão de (%s, %d)\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        puts("Recebendo vetor...");
        tamanhoVetor = receberVetor(newsockfd, &vetor);

        printArray(vetor, tamanhoVetor);

        merge(vetor, 0, (tamanhoVetor - 1)/2, tamanhoVetor - 1);

        printArray(vetor, tamanhoVetor);

        enviarVetor(newsockfd, &vetor, tamanhoVetor);

        close(newsockfd);
    }

    close(sockfd);
    return 0;
}