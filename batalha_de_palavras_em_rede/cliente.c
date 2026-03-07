#define _POSIX_C_SOURCE 200809L

#include "jogo.h"
#include "protocolo.h"

#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

static volatile sig_atomic_t g_cliente_ativo = 1;
static int g_sock = -1;

static void tratar_sigint(int sinal) {
    (void) sinal;
    g_cliente_ativo = 0;
    if (g_sock >= 0) {
        close(g_sock);
        g_sock = -1;
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

static int ler_porta(const char *texto) {
    char *fim = NULL;
    long porta = strtol(texto, &fim, 10);

    if (texto == NULL || *texto == '\0' || (fim != NULL && *fim != '\0') ||
        porta < 1 || porta > 65535) {
        return -1;
    }

    return (int) porta;
}

static void exibir_banner(const char *host, int porta) {
    printf("====================================\n");
    printf("    BATALHA DE PALAVRAS - Cliente   \n");
    printf("====================================\n");
    printf("Conectando a %s:%d...\n", host, porta);
}

static int conectar_servidor(const char *host, const char *porta_texto) {
    struct addrinfo hints;
    struct addrinfo *resultado = NULL;
    struct addrinfo *atual;
    int sock = -1;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(host, porta_texto, &hints, &resultado);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    for (atual = resultado; atual != NULL; atual = atual->ai_next) {
        sock = socket(atual->ai_family, atual->ai_socktype, atual->ai_protocol);
        if (sock < 0) {
            continue;
        }

        if (connect(sock, atual->ai_addr, atual->ai_addrlen) == 0) {
            break;
        }

        close(sock);
        sock = -1;
    }

    freeaddrinfo(resultado);
    return sock;
}

static RecebimentoStatus ler_stdin_timeout(char *buffer, size_t tam, int timeout_seg) {
    while (g_cliente_ativo) {
        int pronto;
        fd_set leitura;
        struct timeval tv;
        struct timeval *tv_ptr = NULL;

        FD_ZERO(&leitura);
        FD_SET(STDIN_FILENO, &leitura);

        if (timeout_seg >= 0) {
            tv.tv_sec = timeout_seg;
            tv.tv_usec = 0;
            tv_ptr = &tv;
        }

        pronto = select(STDIN_FILENO + 1, &leitura, NULL, NULL, tv_ptr);
        if (pronto == 0) {
            return RECV_TIMEOUT;
        }
        if (pronto < 0) {
            if (errno == EINTR && g_cliente_ativo) {
                continue;
            }
            return RECV_DESCONECTOU;
        }

        if (fgets(buffer, (int) tam, stdin) == NULL) {
            return RECV_DESCONECTOU;
        }

        trim_nova_linha(buffer);
        trim_espacos(buffer);
        return RECV_OK;
    }

    return RECV_DESCONECTOU;
}

static int responder_nome(int sock) {
    char nome[TAM_NOME];

    while (g_cliente_ativo) {
        printf("\nDigite seu nome: ");
        fflush(stdout);

        if (ler_stdin_timeout(nome, sizeof(nome), -1) != RECV_OK) {
            return -1;
        }

        if (!validar_nome(nome)) {
            printf("Nome invalido. Use entre 1 e %d caracteres e evite '|'.\n",
                   TAM_NOME - 1);
            continue;
        }

        if (enviar_formatado(sock, TIPO_NOME "|%s\n", nome) != 0) {
            return -1;
        }
        return 0;
    }

    return -1;
}

static void mostrar_placar(const char *nome1, const char *pts1,
                           const char *nome2, const char *pts2) {
    printf("\n+--------------------------------------+\n");
    printf("| PLACAR: %-12s %3s x %3s %-12s |\n", nome1, pts1, pts2, nome2);
    printf("+--------------------------------------+\n");
}

static int responder_rodada(int sock, int rodada, char letra, int tempo) {
    char palavra[TAM_PALAVRA];
    RecebimentoStatus status;

    printf("\n+--------------------------------------+\n");
    printf("| RODADA %d de %d\n", rodada, TOTAL_RODADAS);
    printf("| Letra: [%c]   Tempo: %d seg\n", letra, tempo);
    printf("| Minimo: 5 caracteres\n");
    printf("+--------------------------------------+\n");
    printf("Sua palavra: ");
    fflush(stdout);

    status = ler_stdin_timeout(palavra, sizeof(palavra), tempo);
    if (status == RECV_TIMEOUT) {
        printf("\nTempo esgotado. Enviando TIMEOUT...\n");
        return enviar_formatado(sock, TIPO_TIMEOUT "|\n");
    }
    if (status != RECV_OK) {
        return -1;
    }

    if (enviar_formatado(sock, TIPO_PALAVRA "|%s\n", palavra) != 0) {
        return -1;
    }

    printf("Enviado: \"%s\" - aguardando resultado...\n", palavra);
    return 0;
}

int main(int argc, char *argv[]) {
    const char *host = "127.0.0.1";
    int porta = PORTA_PADRAO;
    char porta_texto[16];
    char linha[TAM_LINHA];

    if (argc > 3) {
        fprintf(stderr, "Uso: %s [host] [porta]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc >= 2) {
        host = argv[1];
    }
    if (argc == 3) {
        porta = ler_porta(argv[2]);
        if (porta < 0) {
            fprintf(stderr, "Porta invalida: %s\n", argv[2]);
            return EXIT_FAILURE;
        }
    }

    if (configurar_sinais() != 0) {
        return EXIT_FAILURE;
    }

    snprintf(porta_texto, sizeof(porta_texto), "%d", porta);
    exibir_banner(host, porta);

    g_sock = conectar_servidor(host, porta_texto);
    if (g_sock < 0) {
        perror("connect");
        return EXIT_FAILURE;
    }

    printf("Conectado!\n");

    while (g_cliente_ativo) {
        char *separador;
        char *tipo;
        char *conteudo;
        RecebimentoStatus status =
            receber_linha_timeout(g_sock, linha, sizeof(linha), -1);

        if (status == RECV_DESCONECTOU) {
            printf("\nServidor encerrou a conexao.\n");
            break;
        }
        if (status != RECV_OK) {
            printf("\nFalha na comunicacao com o servidor.\n");
            break;
        }

        separador = strchr(linha, '|');
        if (separador == NULL) {
            printf("\n[PROTOCOLO] %s\n", linha);
            continue;
        }

        *separador = '\0';
        tipo = linha;
        conteudo = separador + 1;

        if (strcmp(tipo, TIPO_NOME) == 0) {
            if (responder_nome(g_sock) != 0) {
                break;
            }
            continue;
        }

        if (strcmp(tipo, TIPO_MSG) == 0) {
            printf("\n%s\n", conteudo);
            continue;
        }

        if (strcmp(tipo, TIPO_AGUARDE) == 0) {
            printf("\nAguardando: %s\n", conteudo);
            continue;
        }

        if (strcmp(tipo, TIPO_RODADA) == 0) {
            int rodada = 0;
            int tempo = 0;
            char letra = '\0';

            if (sscanf(conteudo, "%d|%c|%d", &rodada, &letra, &tempo) != 3) {
                printf("\nMensagem de rodada invalida: %s\n", conteudo);
                continue;
            }

            if (responder_rodada(g_sock, rodada, letra, tempo) != 0) {
                break;
            }
            continue;
        }

        if (strcmp(tipo, TIPO_RESULTADO) == 0) {
            printf("\nResultado: %s\n", conteudo);
            continue;
        }

        if (strcmp(tipo, TIPO_PLACAR) == 0) {
            char copia[TAM_LINHA];
            char *saveptr = NULL;
            char *nome1;
            char *pts1;
            char *nome2;
            char *pts2;

            strncpy(copia, conteudo, sizeof(copia) - 1);
            copia[sizeof(copia) - 1] = '\0';

            nome1 = strtok_r(copia, "|", &saveptr);
            pts1 = strtok_r(NULL, "|", &saveptr);
            nome2 = strtok_r(NULL, "|", &saveptr);
            pts2 = strtok_r(NULL, "|", &saveptr);

            if (nome1 != NULL && pts1 != NULL && nome2 != NULL && pts2 != NULL) {
                mostrar_placar(nome1, pts1, nome2, pts2);
            } else {
                printf("\nPlacar invalido: %s\n", conteudo);
            }
            continue;
        }

        if (strcmp(tipo, TIPO_FIM) == 0) {
            printf("\nFIM: %s\n", conteudo);
            break;
        }

        printf("\nMensagem desconhecida: %s|%s\n", tipo, conteudo);
    }

    if (g_sock >= 0) {
        close(g_sock);
    }

    printf("Cliente encerrado.\n");
    return EXIT_SUCCESS;
}
