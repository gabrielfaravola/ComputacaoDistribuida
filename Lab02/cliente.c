#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "protocolo.h"
#include "jogo.h"

static int sockfd = -1;
static char meu_nome[TAM_NOME] = "";

static int ler_input(char *buf, int tam, int segundos)
{
    time_t deadline = time(NULL) + segundos;
    int maior = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

    while (1) {
        int restante = (int)(deadline - time(NULL));
        if (restante <= 0) { printf("\nTempo esgotado!\n"); return TIMEOUT_OK; }

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(sockfd, &fds);
        struct timeval tv = { .tv_sec = restante, .tv_usec = 0 };

        int r = select(maior + 1, &fds, NULL, NULL, &tv);
        if (r == 0) { printf("\nTempo esgotado!\n"); return TIMEOUT_OK; }
        if (r < 0)  return ERRO;

        if (FD_ISSET(STDIN_FILENO, &fds)) {
            if (!fgets(buf, tam, stdin)) return ERRO;
            limpar_string(buf);
            return OK;
        }
    }
}

static int processar(const char *msg)
{
    char corpo[TAM_BUF];

    if (extrair_corpo(msg, NOME_PREFIXO, corpo, sizeof(corpo))) {
        printf("Seu nome: ");
        fflush(stdout);
        fgets(meu_nome, TAM_NOME, stdin);
        limpar_string(meu_nome);
        if (!meu_nome[0]) strncpy(meu_nome, "Jogador", TAM_NOME);
        enviar_msg(sockfd, NOME_PREFIXO, meu_nome);
        printf("Bem-vindo, %s!\n\n", meu_nome);
        return OK;
    }

    if (extrair_corpo(msg, AGUARDE_PREFIXO, corpo, sizeof(corpo))) {
        printf(">> %s\n", corpo);
        return OK;
    }

    if (extrair_corpo(msg, MSG_PREFIXO, corpo, sizeof(corpo))) {
        printf("\n%s\n", corpo);
        return OK;
    }

    if (extrair_corpo(msg, RODADA_PREFIXO, corpo, sizeof(corpo))) {
        int num, tempo;
        char letra_s[4];
        if (sscanf(corpo, "%d|%1s|%d", &num, letra_s, &tempo) == 3) {
            char letra = toupper((unsigned char)letra_s[0]);
            printf("\n--- Rodada %d de %d ---\n", num, NUM_RODADAS);
            printf("Letra: [%c]  |  Tempo: %d segundos  |  Minimo: %d letras\n",
                   letra, tempo, MIN_PALAVRA);
            printf("Sua palavra: ");
            fflush(stdout);

            char palavra[TAM_PALAVRA];
            if (ler_input(palavra, sizeof(palavra), tempo) != OK || !palavra[0]) {
                enviar_msg(sockfd, TIMEOUT_PREFIXO, "");
            } else {
                enviar_msg(sockfd, PALAVRA_PREFIXO, palavra);
                printf("Enviado: \"%s\" — aguardando resultado...\n", palavra);
            }
        }
        return OK;
    }

    if (extrair_corpo(msg, RESULTADO_PREFIXO, corpo, sizeof(corpo))) {
        printf("\n%s\n", corpo);
        return OK;
    }

    if (extrair_corpo(msg, PLACAR_PREFIXO, corpo, sizeof(corpo))) {
        char n1[TAM_NOME], n2[TAM_NOME];
        int p1, p2;
        if (sscanf(corpo, "%63[^|]|%d|%63[^|]|%d", n1, &p1, n2, &p2) == 4)
            printf("Placar: %s %d x %d %s\n", n1, p1, p2, n2);
        return OK;
    }

    if (extrair_corpo(msg, FIM_PREFIXO, corpo, sizeof(corpo))) {
        printf("\n=== %s ===\n\n", corpo);
        return ERRO; /* encerra o loop principal */
    }

    return OK;
}

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);

    const char *ip    = (argc >= 2) ? argv[1] : "127.0.0.1";
    int          porta = (argc >= 3) ? atoi(argv[2]) : PORTA_PADRAO;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv = {
        .sin_family = AF_INET,
        .sin_port   = htons((uint16_t)porta)
    };
    inet_pton(AF_INET, ip, &serv.sin_addr);

    printf("Conectando a %s:%d...\n", ip, porta);
    if (connect(sockfd, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        perror("Erro ao conectar");
        return 1;
    }
    printf("Conectado!\n\n");

    char buf[TAM_BUF];
    while (1) {
        int r = receber_com_timeout(sockfd, buf, sizeof(buf), 120);
        if (r != OK) { printf("Conexao encerrada.\n"); break; }
        if (processar(buf) == ERRO) break;
    }

    close(sockfd);
    return 0;
}