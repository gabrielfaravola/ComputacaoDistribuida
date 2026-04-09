#ifndef JOGO_H
#define JOGO_H

#include "protocolo.h"

typedef struct {
    int  fd;
    char nome[TAM_NOME];
    int  pontos;
    char palavra[TAM_PALAVRA];
    int  respondeu;
    int  timeout;
} Jogador;

int  validar_palavra(const char *palavra, char letra);
char gerar_letra(void);
int  enviar_msg(int fd, const char *prefixo, const char *corpo);
int  receber_com_timeout(int fd, char *buf, int tam, int segundos);
void limpar_string(char *s);
int  extrair_corpo(const char *msg, const char *prefixo, char *saida, int tam);

#endif