#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct Ordenado {
    int *vetor;
    int tamanho;
} Ordenado;

int quantidadeMerge = 0;
Ordenado vetoresOrdenados[2];

void error(const char *msg) {
    perror(msg);
    exit(0);
}

int *initVetor(int size) {
    int *vetorRandom = malloc(size * sizeof(int));
    srand(time(NULL));

    for (int i = 0; i < size; i++) {
        vetorRandom[i] = (rand() % 100) + 1;
    }

    return vetorRandom;
}

/* Function to print an array */
void printArray(int arr[], int size) {
    int i;
    for (i = 0; i < size; i++)
        printf("Vetor[%d]: %d. ", i, arr[i]);
    printf("\n");
}

int conectarServidor(char *ip, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    server = gethostbyname(ip);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

    puts("Conectado ao servidor.");

    return sockfd;
}

void enviarVetor(int sockfd, int **vetor, int size) {
    int controle, bytes_transfer;

    printf("Enviando tamanho do vetor: ");
    bytes_transfer = write(sockfd, &size, sizeof(size));

    if (bytes_transfer < 0) {
        error("ERROR writing to socket");
    }

    puts("Aguardando confirmacao de recebimento do tamanho do vetor.");
    bytes_transfer = read(sockfd, &controle, sizeof(int));

    if (bytes_transfer < 0) {
        error("[Erro] Sem confirmacao de recebimento do tamanho.");
    } else if (controle) {
        puts("Tamaho do vetor recebido pelo servidor.");

        puts("Enviando vetor: ");
        bytes_transfer = write(sockfd, *vetor, size * sizeof(int));

        if (bytes_transfer < 0) {
            error("Erro ao enviar vetor.");
        }

        puts("Aguardando confirmacao de recebimento do vetor.");

        bytes_transfer = read(sockfd, &controle, sizeof(int));

        if (bytes_transfer < 0 || !controle) {
            error("Sem confirmacao de recebimento do vetor.");
        } else {
            puts("Vetor recebido pelo servidor.");
        }
    }
}

void receberVetor(int sockfd, int **vetor, int size) {
    int bytes_transfer = read(sockfd, *vetor, size * sizeof(int));

    if (bytes_transfer < 0) {
        puts("Nao foi possivel receber o vetor ordenado.");
    } else {
        puts("Vetor ordenado recebido.");
        printArray(*vetor, size);
    }
}

int ordenarVetor(char *ip, int port, int **vetor, int tamanhoVetor) {
    int serversockfd = conectarServidor(ip, port);

    enviarVetor(serversockfd, &*vetor, tamanhoVetor);

    receberVetor(serversockfd, &*vetor, tamanhoVetor);

    close(serversockfd);
}

void mergeVetoresOrdenados() {
    int *vetorFinal;
    int finalSize;

    puts("Dois vetores para merge!");

    int serversockfd = conectarServidor("0.0.0.0", 9091);

    enviarVetor(serversockfd, &vetoresOrdenados[0].vetor, vetoresOrdenados[0].tamanho);
    enviarVetor(serversockfd, &vetoresOrdenados[1].vetor, vetoresOrdenados[1].tamanho);

    finalSize = vetoresOrdenados[0].tamanho + vetoresOrdenados[1].tamanho;
    vetorFinal = malloc(finalSize * sizeof(int));

    receberVetor(serversockfd, &vetorFinal, finalSize);

    close(serversockfd);
}

void addToMerge(int **vetor, int size) {
    Ordenado *ordenado = &vetoresOrdenados[quantidadeMerge++];
    ordenado->vetor = *vetor;
    ordenado->tamanho = size;

    if (quantidadeMerge == 2) {
        quantidadeMerge = 0;
        mergeVetoresOrdenados();
    }
}

int main(int argc, char *argv[]) {
    char *ip;
    int port;
    int *vetorA, *vetorB;
    int tamanhoVetor = 50;
    int controle;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    ip = argv[1];
    port = atoi(argv[2]);

    vetorA = initVetor(tamanhoVetor);
    ordenarVetor(ip, port, &vetorA, tamanhoVetor);
    addToMerge(&vetorA, tamanhoVetor);

    vetorB = initVetor(tamanhoVetor);
    ordenarVetor(ip, port, &vetorB, tamanhoVetor);
    addToMerge(&vetorB, tamanhoVetor);

    return 0;
}
