#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "protocolo.h"
#include "jogo.h"

static int fila[64];
static int fila_total = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;

static void fila_push(int fd)
{
    pthread_mutex_lock(&mutex);
    fila[fila_total++] = fd;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

static int pegar_nome(Jogador *j)
{
    char buf[TAM_BUF];
    enviar_msg(j->fd, NOME_PREFIXO, "");
    if (receber_com_timeout(j->fd, buf, sizeof(buf), 30) != OK) return ERRO;
    if (!extrair_corpo(buf, NOME_PREFIXO, j->nome, TAM_NOME) || !j->nome[0])
        strncpy(j->nome, "Jogador", TAM_NOME);
    return OK;
}

static void rodada(Jogador *j, int num)
{
    char buf[TAM_BUF], corpo[TAM_BUF];
    char letra = gerar_letra();

    snprintf(corpo, sizeof(corpo), "%d|%c|%d", num, letra, TEMPO_RODADA);
    enviar_msg(j[0].fd, RODADA_PREFIXO, corpo);
    enviar_msg(j[1].fd, RODADA_PREFIXO, corpo);

    for (int i = 0; i < 2; i++) {
        j[i].palavra[0] = '\0';
        j[i].respondeu = j[i].timeout = 0;
    }

    for (int i = 0; i < 2; i++) {
        if (receber_com_timeout(j[i].fd, buf, sizeof(buf), TEMPO_RODADA + 2) != OK) {
            j[i].timeout = 1;
        } else if (extrair_corpo(buf, PALAVRA_PREFIXO, j[i].palavra, TAM_PALAVRA)) {
            j[i].respondeu = 1;
        } else {
            j[i].timeout = 1;
        }
    }

    int ok[2] = {0, 0};
    for (int i = 0; i < 2; i++)
        if (j[i].respondeu)
            ok[i] = validar_palavra(j[i].palavra, letra);

    /* Mesma palavra: ninguém pontua */
    if (ok[0] && ok[1] && strcasecmp(j[0].palavra, j[1].palavra) == 0)
        ok[0] = ok[1] = 0;

    for (int i = 0; i < 2; i++)
        if (ok[i]) j[i].pontos++;

    for (int i = 0; i < 2; i++) {
        int op = 1 - i;
        char res[TAM_BUF];
        if (j[i].timeout)
            snprintf(res, sizeof(res), "Tempo esgotado. 0 pontos. (%s disse: %s)",
                     j[op].nome, j[op].respondeu ? j[op].palavra : "nada");
        else if (!ok[i])
            snprintf(res, sizeof(res), "Palavra '%s' invalida. 0 pontos. (%s disse: %s)",
                     j[i].palavra, j[op].nome, j[op].respondeu ? j[op].palavra : "nada");
        else
            snprintf(res, sizeof(res), "Palavra '%s' valida! +1 ponto. (%s disse: %s)",
                     j[i].palavra, j[op].nome, j[op].palavra);
        enviar_msg(j[i].fd, RESULTADO_PREFIXO, res);
    }

    snprintf(corpo, sizeof(corpo), "%s|%d|%s|%d",
             j[0].nome, j[0].pontos, j[1].nome, j[1].pontos);
    enviar_msg(j[0].fd, PLACAR_PREFIXO, corpo);
    enviar_msg(j[1].fd, PLACAR_PREFIXO, corpo);

    printf("  Rodada %d [%c]: %s=%s(%s) | %s=%s(%s) | %d x %d\n",
           num, letra,
           j[0].nome, j[0].respondeu ? j[0].palavra : "-", ok[0] ? "ok" : "x",
           j[1].nome, j[1].respondeu ? j[1].palavra : "-", ok[1] ? "ok" : "x",
           j[0].pontos, j[1].pontos);
}

static void *partida(void *arg)
{
    int *fds = (int *)arg;
    Jogador j[2];
    memset(j, 0, sizeof(j));
    j[0].fd = fds[0];
    j[1].fd = fds[1];
    free(fds);

    if (pegar_nome(&j[0]) == ERRO || pegar_nome(&j[1]) == ERRO) goto fim;

    printf("\nPartida: %s vs %s\n", j[0].nome, j[1].nome);

    char aviso[TAM_BUF];
    snprintf(aviso, sizeof(aviso), "%s vs %s — %d rodadas. Boa sorte!",
             j[0].nome, j[1].nome, NUM_RODADAS);
    enviar_msg(j[0].fd, MSG_PREFIXO, aviso);
    enviar_msg(j[1].fd, MSG_PREFIXO, aviso);

    for (int r = 1; r <= NUM_RODADAS; r++)
        rodada(j, r);

    char fim[TAM_BUF];
    if (j[0].pontos > j[1].pontos)
        snprintf(fim, sizeof(fim), "%s venceu! %d x %d", j[0].nome, j[0].pontos, j[1].pontos);
    else if (j[1].pontos > j[0].pontos)
        snprintf(fim, sizeof(fim), "%s venceu! %d x %d", j[1].nome, j[1].pontos, j[0].pontos);
    else
        snprintf(fim, sizeof(fim), "Empate! %d x %d", j[0].pontos, j[1].pontos);

    enviar_msg(j[0].fd, FIM_PREFIXO, fim);
    enviar_msg(j[1].fd, FIM_PREFIXO, fim);
    printf("Resultado: %s\n", fim);

fim:
    close(j[0].fd);
    close(j[1].fd);
    return NULL;
}

static void *pairing(void *arg)
{
    (void)arg;
    while (1) {
        pthread_mutex_lock(&mutex);
        while (fila_total < 2)
            pthread_cond_wait(&cond, &mutex);

        int *fds = malloc(2 * sizeof(int));
        fds[0] = fila[0];
        fds[1] = fila[1];
        memmove(fila, fila + 2, (size_t)(fila_total - 2) * sizeof(int));
        fila_total -= 2;
        pthread_mutex_unlock(&mutex);

        pthread_t tid;
        pthread_create(&tid, NULL, partida, fds);
        pthread_detach(tid);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);

    int porta = (argc >= 2) ? atoi(argv[1]) : PORTA_PADRAO;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port   = htons((uint16_t)porta),
        .sin_addr.s_addr = INADDR_ANY
    };
    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    listen(sockfd, 10);

    printf("Servidor rodando na porta %d. Aguardando jogadores...\n", porta);

    pthread_t tid;
    pthread_create(&tid, NULL, pairing, NULL);
    pthread_detach(tid);

    while (1) {
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);
        int cfd = accept(sockfd, (struct sockaddr *)&cli, &len);
        if (cfd < 0) continue;

        printf("[+] Conectou: %s (fd=%d) — %d na fila\n",
               inet_ntoa(cli.sin_addr), cfd, fila_total + 1);
        enviar_msg(cfd, AGUARDE_PREFIXO, "Aguardando outro jogador...");
        fila_push(cfd);
    }
}