/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // defines POSIX constants
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // defines IP standard protocols
#include <arpa/inet.h> // to convert host addresses

#define BUFFER_SIZE 1024
#define TRUE 1

void exitWithFailure() {
    exit(EXIT_FAILURE);
}

void error(const char *msg) {
    perror(msg);
    exitWithFailure();
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, porta, bytes_recv;
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

    printf("\nServidor de ordenacao esperando por cliente na porta %d...", porta);
    fflush(stdout);

    while (TRUE) {
        clilen = sizeof(struct sockaddr_in);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        printf("\nRecebendo conexão de (%s , %d)", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        while (TRUE) {
            printf("\nEnvie (q ou Q para sair) : ");

            gets(send_data);

            if (strcmp(send_data, "q") == 0 || strcmp(send_data, "Q") == 0) {
                send(newsockfd, send_data, strlen(send_data), 0);
                close(newsockfd);
                break;
            } else {
                send(newsockfd, send_data, strlen(send_data), 0);
            }

            bytes_recv = recv(newsockfd, recv_data, BUFFER_SIZE, 0);
            recv_data[bytes_recv] = '\0';

            if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0) {
                close(newsockfd);
                break;
            } else {
                printf("\nDados recebidos: %s ", recv_data);
            }

            fflush(stdout);
        }
    }

    close(sockfd);
    return 0;
}