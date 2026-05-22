#ifndef JOGO_H
#define JOGO_H

#include <stddef.h>

#include "protocolo.h"

typedef enum RecebimentoStatus {
    RECV_ERRO = -2,
    RECV_DESCONECTOU = -1,
    RECV_TIMEOUT = 0,
    RECV_OK = 1
} RecebimentoStatus;

typedef struct RespostaJogador {
    int respondido;
    int timeout;
    int desconectou;
    char palavra[TAM_PALAVRA];
} RespostaJogador;

char gerar_letra_aleatoria(void);
int validar_nome(const char *nome);
int validar_palavra(const char *palavra, char letra, char *motivo, size_t motivo_tam);
int palavras_repetidas(const char *palavra_a, const char *palavra_b);
void trim_nova_linha(char *texto);
void trim_espacos(char *texto);
void minusculas_ascii(const char *origem, char *destino, size_t tam);
int enviar_formatado(int fd, const char *fmt, ...);
RecebimentoStatus receber_linha_timeout(int fd, char *buffer, size_t tam, int timeout_seg);
RecebimentoStatus coletar_respostas_rodada(int fd1, RespostaJogador *j1,
                                           int fd2, RespostaJogador *j2,
                                           int tempo_seg);

#endif
