#define _POSIX_C_SOURCE 200809L

#include "jogo.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static int eh_letra_ascii(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static long long agora_em_ms(void) {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long) ts.tv_sec * 1000LL + (long long) ts.tv_nsec / 1000000LL;
}

static RecebimentoStatus receber_linha_ate_deadline(int fd, char *buffer, size_t tam,
                                                    long long deadline_ms,
                                                    int usar_deadline) {
    size_t pos = 0;

    if (tam == 0) {
        return RECV_ERRO;
    }

    while (1) {
        int pronto;
        fd_set leitura;

        FD_ZERO(&leitura);
        FD_SET(fd, &leitura);

        if (usar_deadline) {
            struct timeval tv;
            long long restante_ms = deadline_ms - agora_em_ms();

            if (restante_ms <= 0) {
                buffer[pos] = '\0';
                return RECV_TIMEOUT;
            }

            tv.tv_sec = (time_t) (restante_ms / 1000LL);
            tv.tv_usec = (suseconds_t) ((restante_ms % 1000LL) * 1000LL);
            pronto = select(fd + 1, &leitura, NULL, NULL, &tv);
        } else {
            pronto = select(fd + 1, &leitura, NULL, NULL, NULL);
        }

        if (pronto == 0) {
            buffer[pos] = '\0';
            return RECV_TIMEOUT;
        }
        if (pronto < 0) {
            if (errno == EINTR) {
                continue;
            }
            buffer[pos] = '\0';
            return RECV_ERRO;
        }

        while (FD_ISSET(fd, &leitura)) {
            char c;
            ssize_t lidos = recv(fd, &c, 1, 0);

            if (lidos == 0) {
                buffer[pos] = '\0';
                return RECV_DESCONECTOU;
            }
            if (lidos < 0) {
                if (errno == EINTR) {
                    break;
                }
                buffer[pos] = '\0';
                return RECV_ERRO;
            }

            if (c == '\r') {
                continue;
            }
            if (c == '\n') {
                buffer[pos] = '\0';
                return RECV_OK;
            }
            if (pos + 1 < tam) {
                buffer[pos++] = c;
            }
        }
    }
}

static void inicializar_resposta(RespostaJogador *resposta) {
    if (resposta == NULL) {
        return;
    }

    resposta->respondido = 0;
    resposta->timeout = 1;
    resposta->desconectou = 0;
    resposta->palavra[0] = '\0';
}

static void interpretar_resposta(const char *linha, RespostaJogador *resposta) {
    inicializar_resposta(resposta);

    if (linha == NULL || resposta == NULL) {
        return;
    }

    if (strncmp(linha, TIPO_PALAVRA "|", strlen(TIPO_PALAVRA) + 1) == 0) {
        resposta->respondido = 1;
        resposta->timeout = 0;
        strncpy(resposta->palavra, linha + strlen(TIPO_PALAVRA) + 1, TAM_PALAVRA - 1);
        resposta->palavra[TAM_PALAVRA - 1] = '\0';
        trim_nova_linha(resposta->palavra);
        trim_espacos(resposta->palavra);
        return;
    }

    if (strcmp(linha, TIPO_TIMEOUT "|") == 0 || strcmp(linha, TIPO_TIMEOUT) == 0) {
        return;
    }

    resposta->respondido = 1;
    resposta->timeout = 0;
    resposta->palavra[0] = '\0';
}

char gerar_letra_aleatoria(void) {
    return (char) ('A' + (rand() % 26));
}

int validar_nome(const char *nome) {
    size_t i;
    size_t len;

    if (nome == NULL) {
        return 0;
    }

    while (*nome != '\0' && isspace((unsigned char) *nome)) {
        nome++;
    }

    len = strlen(nome);
    while (len > 0 && isspace((unsigned char) nome[len - 1])) {
        len--;
    }

    if (len == 0 || len >= TAM_NOME) {
        return 0;
    }

    for (i = 0; i < len; i++) {
        if (nome[i] == '|' || nome[i] == '\n' || nome[i] == '\r') {
            return 0;
        }
    }

    return 1;
}

int validar_palavra(const char *palavra, char letra, char *motivo, size_t motivo_tam) {
    size_t i;
    size_t len;
    char letra_inicial;

    if (motivo != NULL && motivo_tam > 0) {
        motivo[0] = '\0';
    }

    if (palavra == NULL || palavra[0] == '\0') {
        if (motivo != NULL && motivo_tam > 0) {
            snprintf(motivo, motivo_tam, "palavra vazia");
        }
        return 0;
    }

    len = strlen(palavra);
    if (len < 5) {
        if (motivo != NULL && motivo_tam > 0) {
            snprintf(motivo, motivo_tam, "minimo de 5 caracteres");
        }
        return 0;
    }

    letra_inicial = (char) tolower((unsigned char) palavra[0]);
    if (letra_inicial != (char) tolower((unsigned char) letra)) {
        if (motivo != NULL && motivo_tam > 0) {
            snprintf(motivo, motivo_tam, "nao comeca com a letra %c", letra);
        }
        return 0;
    }

    for (i = 0; i < len; i++) {
        if (!eh_letra_ascii(palavra[i])) {
            if (motivo != NULL && motivo_tam > 0) {
                snprintf(motivo, motivo_tam, "contem caracteres invalidos");
            }
            return 0;
        }
    }

    return 1;
}

int palavras_repetidas(const char *palavra_a, const char *palavra_b) {
    char normalizada_a[TAM_PALAVRA];
    char normalizada_b[TAM_PALAVRA];

    if (palavra_a == NULL || palavra_b == NULL) {
        return 0;
    }

    minusculas_ascii(palavra_a, normalizada_a, sizeof(normalizada_a));
    minusculas_ascii(palavra_b, normalizada_b, sizeof(normalizada_b));
    return strcmp(normalizada_a, normalizada_b) == 0;
}

void trim_nova_linha(char *texto) {
    size_t len;

    if (texto == NULL) {
        return;
    }

    len = strlen(texto);
    while (len > 0 && (texto[len - 1] == '\n' || texto[len - 1] == '\r')) {
        texto[len - 1] = '\0';
        len--;
    }
}

void trim_espacos(char *texto) {
    char *inicio;
    size_t len;

    if (texto == NULL || texto[0] == '\0') {
        return;
    }

    inicio = texto;
    while (*inicio != '\0' && isspace((unsigned char) *inicio)) {
        inicio++;
    }

    if (inicio != texto) {
        memmove(texto, inicio, strlen(inicio) + 1);
    }

    len = strlen(texto);
    while (len > 0 && isspace((unsigned char) texto[len - 1])) {
        texto[len - 1] = '\0';
        len--;
    }
}

void minusculas_ascii(const char *origem, char *destino, size_t tam) {
    size_t i = 0;

    if (destino == NULL || tam == 0) {
        return;
    }

    if (origem == NULL) {
        destino[0] = '\0';
        return;
    }

    while (origem[i] != '\0' && i + 1 < tam) {
        destino[i] = (char) tolower((unsigned char) origem[i]);
        i++;
    }
    destino[i] = '\0';
}

int enviar_formatado(int fd, const char *fmt, ...) {
    char buffer[TAM_LINHA];
    size_t total = 0;
    int escrito;
    va_list args;

    va_start(args, fmt);
    escrito = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (escrito < 0 || (size_t) escrito >= sizeof(buffer)) {
        return -1;
    }

    while (total < (size_t) escrito) {
        ssize_t enviados = send(fd, buffer + total, (size_t) escrito - total, 0);

        if (enviados < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        total += (size_t) enviados;
    }

    return 0;
}

RecebimentoStatus receber_linha_timeout(int fd, char *buffer, size_t tam, int timeout_seg) {
    if (timeout_seg < 0) {
        return receber_linha_ate_deadline(fd, buffer, tam, 0, 0);
    }

    return receber_linha_ate_deadline(fd, buffer, tam,
                                      agora_em_ms() + (long long) timeout_seg * 1000LL, 1);
}

RecebimentoStatus coletar_respostas_rodada(int fd1, RespostaJogador *j1,
                                           int fd2, RespostaJogador *j2,
                                           int tempo_seg) {
    long long deadline_ms = agora_em_ms() + (long long) tempo_seg * 1000LL;
    int pronto1 = 0;
    int pronto2 = 0;

    inicializar_resposta(j1);
    inicializar_resposta(j2);

    while (!pronto1 || !pronto2) {
        fd_set leitura;
        int max_fd;
        int pronto;
        long long restante_ms = deadline_ms - agora_em_ms();
        struct timeval tv;

        if (restante_ms <= 0) {
            break;
        }

        FD_ZERO(&leitura);
        max_fd = -1;

        if (!pronto1) {
            FD_SET(fd1, &leitura);
            if (fd1 > max_fd) {
                max_fd = fd1;
            }
        }
        if (!pronto2) {
            FD_SET(fd2, &leitura);
            if (fd2 > max_fd) {
                max_fd = fd2;
            }
        }

        if (max_fd < 0) {
            break;
        }

        tv.tv_sec = (time_t) (restante_ms / 1000LL);
        tv.tv_usec = (suseconds_t) ((restante_ms % 1000LL) * 1000LL);
        pronto = select(max_fd + 1, &leitura, NULL, NULL, &tv);

        if (pronto == 0) {
            break;
        }
        if (pronto < 0) {
            if (errno == EINTR) {
                continue;
            }
            return RECV_ERRO;
        }

        if (!pronto1 && FD_ISSET(fd1, &leitura)) {
            char linha[TAM_LINHA];
            RecebimentoStatus status =
                receber_linha_ate_deadline(fd1, linha, sizeof(linha), deadline_ms, 1);

            if (status == RECV_DESCONECTOU || status == RECV_ERRO) {
                j1->desconectou = 1;
                return status;
            }
            if (status == RECV_OK) {
                interpretar_resposta(linha, j1);
                pronto1 = 1;
            }
        }

        if (!pronto2 && FD_ISSET(fd2, &leitura)) {
            char linha[TAM_LINHA];
            RecebimentoStatus status =
                receber_linha_ate_deadline(fd2, linha, sizeof(linha), deadline_ms, 1);

            if (status == RECV_DESCONECTOU || status == RECV_ERRO) {
                j2->desconectou = 1;
                return status;
            }
            if (status == RECV_OK) {
                interpretar_resposta(linha, j2);
                pronto2 = 1;
            }
        }
    }

    return RECV_OK;
}
