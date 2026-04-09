#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#define PORTA_PADRAO  7070
#define NUM_RODADAS   5
#define TEMPO_RODADA  10
#define MIN_PALAVRA   5

#define TAM_BUF     512
#define TAM_NOME     64
#define TAM_PALAVRA 128

#define MSG_PREFIXO       "MSG"
#define NOME_PREFIXO      "NOME"
#define AGUARDE_PREFIXO   "AGUARDE"
#define RODADA_PREFIXO    "RODADA"
#define RESULTADO_PREFIXO "RESULTADO"
#define PLACAR_PREFIXO    "PLACAR"
#define FIM_PREFIXO       "FIM"

#define PALAVRA_PREFIXO  "PALAVRA"
#define TIMEOUT_PREFIXO  "TIMEOUT"

#define OK              0
#define ERRO           -1
#define TIMEOUT_OK      1

#endif