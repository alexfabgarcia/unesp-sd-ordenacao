/**
 * Projeto Ordenação Merge-Sort com Sockets.
 * Autor: Alex Fabiano Garcia
 * Disciplina: Sistemas Distribuídos
 * Unesp - IBILCE - 2017
 * Código-fonte do cliente do projeto.
 * Este arquivo é reponsável por ler as propriedades do arquivo de configuração informado por parâmetro,
 * gerar um vetor de inteiros aleariamente e enviar para os servidores informados.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/**
 * Constantes.
 */
#define TAMANHO_PARTICAO_VETOR 32;
#define TAMANHO_MIN_VETOR 64
#define TAMANHO_MAX_VETOR 1600
#define LENGTH_PROP_SERVER_ORD strlen("servidores.ordenacao")
#define LENGTH_PROP_SERVER_MERGE strlen("servidores.merge")
#define DELIMITADOR_SERVIDORES ";"
#define DELIMITADOR_IP_PORTA ":"

/**
 * Estrutura do servidor.
 */
typedef struct Server {
    char *ip;
    int port;
} Server;

/**
 * Variaveis globais.
 */
Server *servidoresOrdenacao = NULL, *servidoresMerge = NULL;
int amountSortServer, amountMergeServer;
int tamanhoVetor;
int *vetorInicial;

void error(const char *msg) {
    perror(msg);
    exit(0);
}

/**
 * Função responsável por imprimir um vetor.
 */
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

/**
 * Função invocada quando o arquivo possui formato inválido. Assim, ele é fechado - se existir - e o programa é finalizado.
 */
void tratarArquivoInvalido(FILE *arquivo) {
    if (arquivo == NULL) {
        puts("Fechando arquivo...");
        fclose(arquivo);
    }
    exit(EXIT_FAILURE);
}

/**
 * Inicia a struct que representa um servidor a partir da String de configuração.
 * @param originalInfo String com a informação do servidor. Ip e porta são separados por ":".
 * @return Struct que representa o servidor.
 */
Server *getServer(char *originalInfo) {
    char *copyInfo, *token;
    Server *server;

    // Cria cópia para manter a informação original
    copyInfo = malloc(strlen(originalInfo) * sizeof(char));
    strcpy(copyInfo, originalInfo);

    token = strtok(copyInfo, DELIMITADOR_IP_PORTA);

    server = malloc(sizeof(Server));
    server->ip = token;
    token = strtok(NULL, DELIMITADOR_IP_PORTA);
    sscanf(token, "%d", &server->port);
    return server;
}

/**
 * Retorna a quantidade de servidores a partir da String de configuração.
 * @param informacaoServidores String de configuração de servidores, separando-os por ";".
 * @return A quantidade de servidores.
 */
int amountServers(char *informacaoServidores) {
    int tamanhoInformacao = strlen(informacaoServidores);
    int quantidadeServidores = 1;

    for (int i = 0; i < tamanhoInformacao; i++) {
        if (informacaoServidores[i] == ';') {
            quantidadeServidores++;
        }
    }

    printf("A quantidade de servidor informada eh %d:\n", quantidadeServidores);
    return quantidadeServidores;
}

/**
 * Recupera um vetor com as informações dos servidores a partir da String do arquivo de configuração.
 * @param informacaoServidores String que contém as informações dos servidores separando-os por ";".
 * @param amount A quantidade de servidores.
 * @return Vetor com informações dos servidores.
 */
Server *initServers(char *informacaoServidores, int amount) {
    char *token;
    int index = 0;
    Server *servidores;
    Server *serverAux;

    servidores = malloc(amount * sizeof(Server));

    while((token = strsep(&informacaoServidores, DELIMITADOR_SERVIDORES)) != NULL) {
        serverAux = getServer(token);
        servidores[index].ip = serverAux->ip;
        servidores[index].port = serverAux->port;
        printf("%d - IP %s porta %d.\n", index + 1, servidores[index].ip, servidores[index].port);
        index++;
    }

    return servidores;
}

/**
 * Inicia o vetor com as os números inteiros aleatórios.
 */
void initVetor() {
    vetorInicial = malloc(tamanhoVetor * sizeof(int));
    srand(time(NULL));

    for (int i = 0; i < tamanhoVetor; i++) {
        vetorInicial[i] = rand();
    }
}

void initSort() {
    int tamanhoParticao = (tamanhoVetor + 32 - 1) / 32;
    int *abc[32];
    int serverIndex = 0;

    for (int i = 0; i < 32; i++) {
        abc[i] = malloc(tamanhoParticao * sizeof(int));
        memcpy(abc[i], vetorInicial + (tamanhoParticao * i), tamanhoParticao * sizeof(int));

        if (serverIndex == amountSortServer) {
            serverIndex = 0;
        }

        printf("\nEnviando vetor %d para ordenação...\n", i + 1);
        ordenarVetor(servidoresOrdenacao[serverIndex].ip, servidoresOrdenacao[serverIndex].port,
                     &abc[i], tamanhoParticao);
        serverIndex++;
    }
}

/**
 * Função principal, que realiza a leitura do arquivo de configuracao.
 */
int main (int argc, char *argv[]) {
    if (argc > 1) {
        char *nomeArquivo = argv[1];
        FILE *arquivoEntrada = fopen(nomeArquivo, "r");
        char *linhaArquivo = NULL;
        char *valorSevidoresOrdenacao = NULL;
        char *valorSevidoresMerge = NULL;
        size_t posicaoBuffer = 0;

        if (arquivoEntrada == NULL) {
            printf("[ERRO] O arquivo '%s' nao foi encontrado para leitura das informacoes.\n", nomeArquivo);
            tratarArquivoInvalido(arquivoEntrada);
        }

        printf("Inicio da leitura do arquivo de configuracao '%s'...\n\n", nomeArquivo);

        if (getline(&linhaArquivo, &posicaoBuffer, arquivoEntrada) == -1) {
            puts("[ERRO] A primeira linha do arquivo deve conter a propriedade do tamanho do vetor: e.g. tamanho.vetor=400");
            tratarArquivoInvalido(arquivoEntrada);
        }

        if (sscanf(linhaArquivo, "tamanho.vetor=%d", &tamanhoVetor) != 1 || tamanhoVetor < TAMANHO_MIN_VETOR
            || tamanhoVetor > TAMANHO_MAX_VETOR) {
            printf("[ERRO] A propriedade 'tamanho.vetor' deve ter valor entre %d e %d.\n", TAMANHO_MIN_VETOR, TAMANHO_MAX_VETOR);
            tratarArquivoInvalido(arquivoEntrada);
        }

        printf("Tamanho do vetor: %d.\n", tamanhoVetor);

        if (getline(&linhaArquivo, &posicaoBuffer, arquivoEntrada) == -1) {
            puts("[ERRO] A segunda linha deve conter a propriedade dos servidores de ordenacao do vetor: e.g. servidores.ordenacao=192.168.1.102:8088;192.168.1.103:8088");
            tratarArquivoInvalido(arquivoEntrada);
        }

        valorSevidoresOrdenacao = malloc((strlen(linhaArquivo) - LENGTH_PROP_SERVER_ORD) * sizeof(char));

        if (sscanf(linhaArquivo, "servidores.ordenacao=%s", valorSevidoresOrdenacao) != 1) {
            puts("[ERRO] A propriedade 'servidores.ordenacao' deve conter os IPs e portas dos servidores de ordenacao.");
            tratarArquivoInvalido(arquivoEntrada);
        }

        puts("\nObtendo configuração de servidores de ordenacao...");
        amountSortServer = amountServers(valorSevidoresOrdenacao);
        servidoresOrdenacao = initServers(valorSevidoresOrdenacao, amountSortServer);

        if (getline(&linhaArquivo, &posicaoBuffer, arquivoEntrada) == -1) {
            puts("[ERRO] A segunda linha deve conter a propriedade dos servidores de merge do vetor: e.g. servidores.merge=192.168.1.104:8088;192.168.1.105:8088");
            tratarArquivoInvalido(arquivoEntrada);
        }

        valorSevidoresMerge = malloc((strlen(linhaArquivo) - LENGTH_PROP_SERVER_MERGE) * sizeof(char));

        if (sscanf(linhaArquivo, "servidores.merge=%s", valorSevidoresMerge) != 1) {
            puts("[ERRO] A propriedade 'servidores.merge' deve conter os IPs e portas dos servidores de merge.");
            tratarArquivoInvalido(arquivoEntrada);
        }

        puts("\nObtendo configuração de servidores de merge...");
        amountMergeServer = amountServers(valorSevidoresMerge);
        servidoresMerge = initServers(valorSevidoresMerge, amountMergeServer);

        fclose(arquivoEntrada);

        initVetor();

        initSort();
    } else {
        puts("[ERRO] Eh necessario informar o caminho do arquivo de configuracao como primeiro argumento.");
    }

    return 0;
}