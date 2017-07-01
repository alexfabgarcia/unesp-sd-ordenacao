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
#include "ruwindows.h"

#define TAMANHO_MIN_VETOR 64
#define TAMANHO_MAX_VETOR 1600
#define LENGTH_PROP_SERVER_ORD strlen("servidores.ordenacao")
#define LENGTH_PROP_SERVER_MERGE strlen("servidores.merge")
#define DELIMITADOR_SERVIDOR ";"

typedef struct Server {
    int ip;
    int port;
} Server;

Server *server;

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

void getServer(char *informacaoServidores) {
    char *tokenServer, *tokenInfo, *tofree;

    tofree = informacaoServidores = strdup(informacaoServidores);

    while (tokenServer = strsep(&informacaoServidores, DELIMITADOR_SERVIDOR)) {
        puts(tokenServer);
    }

    free(tofree);
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
        Server *servidoresOrdenacao,*servidoresMerge;
        size_t posicaoBuffer = 0;
        int tamanhoVetor;

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

        getServer(valorSevidoresOrdenacao);

        if (getline(&linhaArquivo, &posicaoBuffer, arquivoEntrada) == -1) {
            puts("[ERRO] A segunda linha deve conter a propriedade dos servidores de merge do vetor: e.g. servidores.merge=192.168.1.104:8088;192.168.1.105:8088");
            tratarArquivoInvalido(arquivoEntrada);
        }

        valorSevidoresMerge = malloc((strlen(linhaArquivo) - LENGTH_PROP_SERVER_MERGE) * sizeof(char));

        if (sscanf(linhaArquivo, "servidores.merge=%s", valorSevidoresMerge) != 1) {
            puts("[ERRO] A propriedade 'servidores.merge' deve conter os IPs e portas dos servidores de merge.");
            tratarArquivoInvalido(arquivoEntrada);
        }

        getServer(valorSevidoresMerge);

        fclose(arquivoEntrada);
    } else {
        puts("[ERRO] Eh necessario informar o caminho do arquivo de configuracao como primeiro argumento.");
    }

    return 0;
}