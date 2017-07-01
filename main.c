//
// Created by agarcia46 on 7/1/2017.
//
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMANHO_MIN_VETOR 64
#define TAMANHO_MAX_VETOR 1600

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
 * Função principal, que realiza a leitura do arquivo de configuracao.
 */
int main (int argc, char *argv[]) {
    if (argc > 1) {
        char *nomeArquivo = argv[1];
        FILE *arquivoEntrada = fopen(nomeArquivo, "r");
        char *linhaArquivo = NULL;
        size_t posicaoBuffer = 0;
        int tamanhoVetor;

        if (arquivoEntrada == NULL) {
            printf("[ERRO] O arquivo '%s' nao foi encontrado para leitura das informacoes.\n", nomeArquivo);
            tratarArquivoInvalido(arquivoEntrada);
        }

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

        printf("Inicio da leitura do arquivo de configuracao '%s'...\n\n", nomeArquivo);

        fclose(arquivoEntrada);
    } else {
        puts("[ERRO] Eh necessario informar o caminho do arquivo de configuracao como primeiro argumento.");
    }

    return 0;
}