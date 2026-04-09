#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include "jogo.h"

int validar_palavra(const char *palavra, char letra)
{
    if (!palavra || !*palavra) return 0;
    if (tolower((unsigned char)palavra[0]) != tolower((unsigned char)letra)) return 0;

    int len = 0;
    for (const char *p = palavra; *p; p++, len++)
        if (!isalpha((unsigned char)*p)) return 0;

    return len >= MIN_PALAVRA;
}

char gerar_letra(void)
{
    static int init = 0;
    if (!init) { srand((unsigned)time(NULL)); init = 1; }
    return (char)('A' + rand() % 26);
}

int enviar_msg(int fd, const char *prefixo, const char *corpo)
{
    char buf[TAM_BUF];
    if (corpo && *corpo)
        snprintf(buf, sizeof(buf), "%s|%s\n", prefixo, corpo);
    else
        snprintf(buf, sizeof(buf), "%s|\n", prefixo);

    int total = (int)strlen(buf), enviado = 0, n;
    while (enviado < total) {
        n = (int)write(fd, buf + enviado, (size_t)(total - enviado));
        if (n <= 0) return ERRO;
        enviado += n;
    }
    return enviado;
}

int receber_com_timeout(int fd, char *buf, int tam, int segundos)
{
    int i = 0;
    char c;
    while (i < tam - 1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        struct timeval tv = { .tv_sec = segundos, .tv_usec = 0 };

        int r = select(fd + 1, &fds, NULL, NULL, &tv);
        if (r == 0) return TIMEOUT_OK;
        if (r < 0)  return ERRO;

        if (read(fd, &c, 1) <= 0) return ERRO;
        if (c == '\n') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return OK;
}

void limpar_string(char *s)
{
    if (!s) return;
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' || s[len-1] == ' '))
        s[--len] = '\0';
}

int extrair_corpo(const char *msg, const char *prefixo, char *saida, int tam)
{
    size_t plen = strlen(prefixo);
    if (strncmp(msg, prefixo, plen) != 0 || msg[plen] != '|') return 0;
    strncpy(saida, msg + plen + 1, (size_t)(tam - 1));
    saida[tam - 1] = '\0';
    limpar_string(saida);
    return 1;
}