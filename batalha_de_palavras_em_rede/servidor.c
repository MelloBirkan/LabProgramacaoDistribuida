#define _POSIX_C_SOURCE 200809L

#include "jogo.h"
#include "protocolo.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

typedef struct Jogador {
    int fd;
    char nome[TAM_NOME];
    char endereco[64];
} Jogador;

typedef struct Partida {
    int id;
    Jogador a;
    Jogador b;
} Partida;

static pthread_mutex_t g_fila_mutex = PTHREAD_MUTEX_INITIALIZER;
static Jogador *g_aguardando = NULL;
static int g_proxima_partida = 1;
static volatile sig_atomic_t g_servidor_ativo = 1;
static int g_listen_fd = -1;

static void fechar_jogador(Jogador *jogador) {
    if (jogador != NULL && jogador->fd >= 0) {
        close(jogador->fd);
        jogador->fd = -1;
    }
}

static void tratar_sigint(int sinal) {
    (void) sinal;
    g_servidor_ativo = 0;
    if (g_listen_fd >= 0) {
        close(g_listen_fd);
        g_listen_fd = -1;
    }
}

static int configurar_sinais(void) {
    struct sigaction sa_int;
    struct sigaction sa_pipe;

    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = tratar_sigint;
    sigemptyset(&sa_int.sa_mask);

    if (sigaction(SIGINT, &sa_int, NULL) != 0) {
        perror("sigaction(SIGINT)");
        return -1;
    }

    memset(&sa_pipe, 0, sizeof(sa_pipe));
    sa_pipe.sa_handler = SIG_IGN;
    sigemptyset(&sa_pipe.sa_mask);

    if (sigaction(SIGPIPE, &sa_pipe, NULL) != 0) {
        perror("sigaction(SIGPIPE)");
        return -1;
    }

    return 0;
}

static void exibir_banner(int porta) {
    printf("=============================================\n");
    printf("       BATALHA DE PALAVRAS - Servidor        \n");
    printf("  Porta: %d\n", porta);
    printf("  Aguardando jogadores (pares de 2)...\n");
    printf("=============================================\n\n");
}

static int criar_socket_servidor(int porta) {
    int fd;
    int reutilizar = 1;
    struct sockaddr_in servidor_addr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reutilizar, sizeof(reutilizar)) != 0) {
        perror("setsockopt");
        close(fd);
        return -1;
    }

    memset(&servidor_addr, 0, sizeof(servidor_addr));
    servidor_addr.sin_family = AF_INET;
    servidor_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    servidor_addr.sin_port = htons((uint16_t) porta);

    if (bind(fd, (struct sockaddr *) &servidor_addr, sizeof(servidor_addr)) != 0) {
        perror("bind");
        close(fd);
        return -1;
    }

    if (listen(fd, 16) != 0) {
        perror("listen");
        close(fd);
        return -1;
    }

    return fd;
}

static void preencher_endereco(const struct sockaddr_in *cliente_addr, char *destino,
                               size_t destino_tam) {
    char ip[INET_ADDRSTRLEN];

    if (inet_ntop(AF_INET, &cliente_addr->sin_addr, ip, sizeof(ip)) == NULL) {
        snprintf(destino, destino_tam, "desconhecido");
        return;
    }

    snprintf(destino, destino_tam, "%s:%d", ip, ntohs(cliente_addr->sin_port));
}

static int receber_nome_jogador(Jogador *jogador) {
    char linha[TAM_LINHA];

    if (enviar_formatado(jogador->fd, TIPO_NOME "|\n") != 0) {
        return -1;
    }

    if (receber_linha_timeout(jogador->fd, linha, sizeof(linha), TIMEOUT_NOME) != RECV_OK) {
        return -1;
    }

    if (strncmp(linha, TIPO_NOME "|", strlen(TIPO_NOME) + 1) != 0) {
        return -1;
    }

    strncpy(jogador->nome, linha + strlen(TIPO_NOME) + 1, sizeof(jogador->nome) - 1);
    jogador->nome[sizeof(jogador->nome) - 1] = '\0';
    trim_espacos(jogador->nome);

    if (!validar_nome(jogador->nome)) {
        return -1;
    }

    if (enviar_formatado(jogador->fd, TIPO_MSG "|Bem-vindo, %s!\n", jogador->nome) != 0) {
        return -1;
    }

    return 0;
}

static void descrever_oponente(const RespostaJogador *oponente, char *saida, size_t saida_tam) {
    if (oponente->timeout) {
        snprintf(saida, saida_tam, "Oponente sem resposta");
        return;
    }

    snprintf(saida, saida_tam, "Oponente enviou: \"%s\"", oponente->palavra);
}

static void montar_resultado(const RespostaJogador *proprio, const RespostaJogador *oponente,
                             int palavra_valida, int repetida, const char *motivo,
                             char *saida, size_t saida_tam) {
    char detalhe_oponente[128];

    descrever_oponente(oponente, detalhe_oponente, sizeof(detalhe_oponente));

    if (repetida) {
        snprintf(saida, saida_tam,
                 "Palavras repetidas (\"%s\"). Ninguem pontua nesta rodada. [%s]",
                 proprio->palavra, detalhe_oponente);
        return;
    }

    if (proprio->timeout) {
        snprintf(saida, saida_tam, "Tempo esgotado. Voce nao pontuou. [%s]", detalhe_oponente);
        return;
    }

    if (palavra_valida) {
        snprintf(saida, saida_tam, "Palavra \"%s\" valida! +1 ponto. [%s]",
                 proprio->palavra, detalhe_oponente);
        return;
    }

    snprintf(saida, saida_tam, "Palavra \"%s\" invalida: %s. [%s]",
             proprio->palavra[0] != '\0' ? proprio->palavra : "(vazia)",
             motivo, detalhe_oponente);
}

static void enviar_placar(const Partida *partida, int pontos_a, int pontos_b) {
    enviar_formatado(partida->a.fd, TIPO_PLACAR "|%s|%d|%s|%d\n",
                     partida->a.nome, pontos_a, partida->b.nome, pontos_b);
    enviar_formatado(partida->b.fd, TIPO_PLACAR "|%s|%d|%s|%d\n",
                     partida->a.nome, pontos_a, partida->b.nome, pontos_b);
}

static void finalizar_por_desconexao(Partida *partida, int desconectou_a, int desconectou_b) {
    if (desconectou_a && !desconectou_b) {
        enviar_formatado(partida->b.fd, TIPO_FIM "|Oponente desconectou. Voce venceu por W.O.\n");
        printf("[Partida #%d] %s desconectou. %s venceu por W.O.\n",
               partida->id, partida->a.nome, partida->b.nome);
    } else if (desconectou_b && !desconectou_a) {
        enviar_formatado(partida->a.fd, TIPO_FIM "|Oponente desconectou. Voce venceu por W.O.\n");
        printf("[Partida #%d] %s desconectou. %s venceu por W.O.\n",
               partida->id, partida->b.nome, partida->a.nome);
    } else {
        printf("[Partida #%d] Ambos os jogadores se desconectaram.\n", partida->id);
    }
}

static void *thread_partida(void *arg) {
    Partida *partida = (Partida *) arg;
    int pontos_a = 0;
    int pontos_b = 0;
    int rodada;
    int jogo_completo = 1;

    printf("[Partida #%d] Jogadores: %s vs %s\n",
           partida->id, partida->a.nome, partida->b.nome);

    if (enviar_formatado(partida->a.fd, TIPO_MSG "|Batalha de Palavras! %s vs %s - %d rodadas. Boa sorte!\n",
                         partida->a.nome, partida->b.nome, TOTAL_RODADAS) != 0 ||
        enviar_formatado(partida->b.fd, TIPO_MSG "|Batalha de Palavras! %s vs %s - %d rodadas. Boa sorte!\n",
                         partida->a.nome, partida->b.nome, TOTAL_RODADAS) != 0) {
        finalizar_por_desconexao(partida, 0, 0);
        jogo_completo = 0;
    }

    for (rodada = 1; jogo_completo && rodada <= TOTAL_RODADAS; rodada++) {
        RespostaJogador resposta_a;
        RespostaJogador resposta_b;
        char letra = gerar_letra_aleatoria();
        char motivo_a[128];
        char motivo_b[128];
        char texto_a[TAM_LINHA];
        char texto_b[TAM_LINHA];
        int valida_a;
        int valida_b;
        int repetida;

        printf("  [Rodada %d] Letra: %c\n", rodada, letra);

        if (enviar_formatado(partida->a.fd, TIPO_RODADA "|%d|%c|%d\n",
                             rodada, letra, TEMPO_RODADA) != 0) {
            finalizar_por_desconexao(partida, 1, 0);
            jogo_completo = 0;
            break;
        }
        if (enviar_formatado(partida->b.fd, TIPO_RODADA "|%d|%c|%d\n",
                             rodada, letra, TEMPO_RODADA) != 0) {
            finalizar_por_desconexao(partida, 0, 1);
            jogo_completo = 0;
            break;
        }

        if (coletar_respostas_rodada(partida->a.fd, &resposta_a,
                                     partida->b.fd, &resposta_b,
                                     TEMPO_RODADA) != RECV_OK) {
            finalizar_por_desconexao(partida, resposta_a.desconectou, resposta_b.desconectou);
            jogo_completo = 0;
            break;
        }

        valida_a = !resposta_a.timeout &&
                   validar_palavra(resposta_a.palavra, letra, motivo_a, sizeof(motivo_a));
        valida_b = !resposta_b.timeout &&
                   validar_palavra(resposta_b.palavra, letra, motivo_b, sizeof(motivo_b));
        repetida = !resposta_a.timeout && !resposta_b.timeout &&
                   palavras_repetidas(resposta_a.palavra, resposta_b.palavra);

        if (!repetida) {
            if (valida_a) {
                pontos_a++;
            }
            if (valida_b) {
                pontos_b++;
            }
        }

        montar_resultado(&resposta_a, &resposta_b, valida_a, repetida, motivo_a,
                         texto_a, sizeof(texto_a));
        montar_resultado(&resposta_b, &resposta_a, valida_b, repetida, motivo_b,
                         texto_b, sizeof(texto_b));

        if (enviar_formatado(partida->a.fd, TIPO_RESULTADO "|%s\n", texto_a) != 0) {
            finalizar_por_desconexao(partida, 1, 0);
            jogo_completo = 0;
            break;
        }
        if (enviar_formatado(partida->b.fd, TIPO_RESULTADO "|%s\n", texto_b) != 0) {
            finalizar_por_desconexao(partida, 0, 1);
            jogo_completo = 0;
            break;
        }

        enviar_placar(partida, pontos_a, pontos_b);

        printf("  [Rodada %d] %s=\"%s\"(%s) | %s=\"%s\"(%s) | Placar: %d x %d\n",
               rodada,
               partida->a.nome,
               resposta_a.timeout ? "TIMEOUT" : resposta_a.palavra,
               repetida ? "repetida" : (valida_a ? "ok" : "invalida"),
               partida->b.nome,
               resposta_b.timeout ? "TIMEOUT" : resposta_b.palavra,
               repetida ? "repetida" : (valida_b ? "ok" : "invalida"),
               pontos_a, pontos_b);
    }

    if (jogo_completo) {
        char fim[TAM_LINHA];

        if (pontos_a > pontos_b) {
            snprintf(fim, sizeof(fim), "%s venceu! Placar final: %s %d x %d %s",
                     partida->a.nome, partida->a.nome, pontos_a, pontos_b, partida->b.nome);
        } else if (pontos_b > pontos_a) {
            snprintf(fim, sizeof(fim), "%s venceu! Placar final: %s %d x %d %s",
                     partida->b.nome, partida->a.nome, pontos_a, pontos_b, partida->b.nome);
        } else {
            snprintf(fim, sizeof(fim), "Empate! Placar final: %s %d x %d %s",
                     partida->a.nome, pontos_a, pontos_b, partida->b.nome);
        }

        enviar_formatado(partida->a.fd, TIPO_FIM "|%s\n", fim);
        enviar_formatado(partida->b.fd, TIPO_FIM "|%s\n", fim);
        printf("[Partida #%d] %s\n", partida->id, fim);
    }

    fechar_jogador(&partida->a);
    fechar_jogador(&partida->b);
    free(partida);
    return NULL;
}

static int iniciar_partida(Jogador *primeiro, Jogador *segundo) {
    Partida *partida;
    pthread_t thread;

    partida = (Partida *) malloc(sizeof(Partida));
    if (partida == NULL) {
        perror("malloc");
        return -1;
    }

    partida->id = g_proxima_partida++;
    partida->a = *primeiro;
    partida->b = *segundo;

    if (pthread_create(&thread, NULL, thread_partida, partida) != 0) {
        perror("pthread_create");
        free(partida);
        return -1;
    }

    pthread_detach(thread);
    return 0;
}

static int ler_porta(const char *texto) {
    char *fim = NULL;
    long porta = strtol(texto, &fim, 10);

    if (texto == NULL || *texto == '\0' || (fim != NULL && *fim != '\0') ||
        porta < 1 || porta > 65535) {
        return -1;
    }

    return (int) porta;
}

int main(int argc, char *argv[]) {
    int porta = PORTA_PADRAO;

    if (argc > 2) {
        fprintf(stderr, "Uso: %s [porta]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        porta = ler_porta(argv[1]);
        if (porta < 0) {
            fprintf(stderr, "Porta invalida: %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    }

    if (configurar_sinais() != 0) {
        return EXIT_FAILURE;
    }

    srand((unsigned int) time(NULL));

    g_listen_fd = criar_socket_servidor(porta);
    if (g_listen_fd < 0) {
        return EXIT_FAILURE;
    }

    exibir_banner(porta);

    while (g_servidor_ativo) {
        int cliente_fd;
        struct sockaddr_in cliente_addr;
        socklen_t cliente_len = sizeof(cliente_addr);
        Jogador *novo;

        cliente_fd = accept(g_listen_fd, (struct sockaddr *) &cliente_addr, &cliente_len);
        if (cliente_fd < 0) {
            if (!g_servidor_ativo) {
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            continue;
        }

        novo = (Jogador *) calloc(1, sizeof(Jogador));
        if (novo == NULL) {
            perror("calloc");
            close(cliente_fd);
            continue;
        }

        novo->fd = cliente_fd;
        preencher_endereco(&cliente_addr, novo->endereco, sizeof(novo->endereco));
        printf("[+] Jogador conectou: %s (fd=%d)\n", novo->endereco, novo->fd);

        if (receber_nome_jogador(novo) != 0) {
            printf("[-] Falha ao cadastrar jogador %s. Conexao encerrada.\n", novo->endereco);
            fechar_jogador(novo);
            free(novo);
            continue;
        }

        pthread_mutex_lock(&g_fila_mutex);
        if (g_aguardando == NULL) {
            if (enviar_formatado(novo->fd,
                                 TIPO_AGUARDE "|Esperando outro jogador para iniciar a partida...\n") != 0) {
                pthread_mutex_unlock(&g_fila_mutex);
                fechar_jogador(novo);
                free(novo);
                continue;
            }
            g_aguardando = novo;
            printf("[*] Aguardando mais 1 jogador(es)...\n");
            pthread_mutex_unlock(&g_fila_mutex);
        } else {
            Jogador *primeiro = g_aguardando;
            g_aguardando = NULL;

            if (iniciar_partida(primeiro, novo) != 0) {
                enviar_formatado(primeiro->fd, TIPO_FIM "|Servidor indisponivel para iniciar a partida.\n");
                enviar_formatado(novo->fd, TIPO_FIM "|Servidor indisponivel para iniciar a partida.\n");
                fechar_jogador(primeiro);
                fechar_jogador(novo);
            }

            free(primeiro);
            free(novo);
            pthread_mutex_unlock(&g_fila_mutex);
        }
    }

    pthread_mutex_lock(&g_fila_mutex);
    if (g_aguardando != NULL) {
        enviar_formatado(g_aguardando->fd, TIPO_FIM "|Servidor encerrado antes do inicio da partida.\n");
        fechar_jogador(g_aguardando);
        free(g_aguardando);
        g_aguardando = NULL;
    }
    pthread_mutex_unlock(&g_fila_mutex);

    if (g_listen_fd >= 0) {
        close(g_listen_fd);
    }

    printf("\nServidor encerrado.\n");
    return EXIT_SUCCESS;
}
