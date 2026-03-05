#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int codigo;
    char *nome;
    float preco;
    int quantidade;
} Produto;

static int ler_linha(const char *prompt, char *buffer, size_t tamanho) {
    if (prompt != NULL) {
        printf("%s", prompt);
    }

    if (fgets(buffer, (int)tamanho, stdin) == NULL) {
        return 0;
    }

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
    }

    return 1;
}

static int ler_int(const char *prompt, int *valor) {
    char linha[128];
    char *fim = NULL;
    long numero;

    while (1) {
        if (!ler_linha(prompt, linha, sizeof(linha))) {
            return 0;
        }

        errno = 0;
        numero = strtol(linha, &fim, 10);

        if (errno == 0 && fim != linha && *fim == '\0') {
            *valor = (int)numero;
            return 1;
        }

        printf("Entrada invalida. Tente novamente.\n");
    }
}

static int ler_float(const char *prompt, float *valor) {
    char linha[128];
    char *fim = NULL;
    float numero;

    while (1) {
        if (!ler_linha(prompt, linha, sizeof(linha))) {
            return 0;
        }

        errno = 0;
        numero = strtof(linha, &fim);

        if (errno == 0 && fim != linha && *fim == '\0') {
            *valor = numero;
            return 1;
        }

        printf("Entrada invalida. Tente novamente.\n");
    }
}

Produto *buscar_produto(Produto *produtos, int total, int codigo) {
    int i;
    for (i = 0; i < total; i++) {
        if (produtos[i].codigo == codigo) {
            return &produtos[i];
        }
    }
    return NULL;
}

void adicionar_produto(Produto **produtos, int *total, int *proximo_codigo) {
    char nome_buffer[256];
    char *nome_dinamico = NULL;
    Produto *novo_vetor = NULL;
    float preco;
    int quantidade;

    printf("\n--- Adicionar Produto ---\n");

    if (!ler_linha("Nome: ", nome_buffer, sizeof(nome_buffer))) {
        printf("Falha ao ler nome.\n");
        return;
    }

    if (nome_buffer[0] == '\0') {
        printf("Nome nao pode ser vazio.\n");
        return;
    }

    if (!ler_float("Preco: ", &preco)) {
        printf("Falha ao ler preco.\n");
        return;
    }
    if (preco < 0.0f) {
        printf("Preco nao pode ser negativo.\n");
        return;
    }

    if (!ler_int("Quantidade: ", &quantidade)) {
        printf("Falha ao ler quantidade.\n");
        return;
    }
    if (quantidade < 0) {
        printf("Quantidade nao pode ser negativa.\n");
        return;
    }

    nome_dinamico = (char *)malloc(strlen(nome_buffer) + 1);
    if (nome_dinamico == NULL) {
        printf("Erro de memoria ao alocar nome do produto.\n");
        return;
    }
    strcpy(nome_dinamico, nome_buffer);

    novo_vetor = (Produto *)realloc(*produtos, (size_t)(*total + 1) * sizeof(Produto));
    if (novo_vetor == NULL) {
        printf("Erro de memoria ao realocar vetor de produtos.\n");
        free(nome_dinamico);
        return;
    }

    *produtos = novo_vetor;
    (*produtos)[*total].codigo = *proximo_codigo;
    (*produtos)[*total].nome = nome_dinamico;
    (*produtos)[*total].preco = preco;
    (*produtos)[*total].quantidade = quantidade;

    (*total)++;
    (*proximo_codigo)++;

    printf("Produto adicionado com codigo %d!\n", (*produtos)[*total - 1].codigo);
}

void listar_produtos(const Produto *produtos, int total) {
    float total_estoque = 0.0f;
    int i;

    printf("\n--- Lista de Produtos ---\n");

    if (total == 0) {
        printf("Nenhum produto cadastrado.\n");
        return;
    }

    printf("+--------+--------------------------+------------+------+----------------+\n");
    printf("| Codigo | Nome                     | Preco      | Qtd  | Valor Estoque  |\n");
    printf("+--------+--------------------------+------------+------+----------------+\n");

    for (i = 0; i < total; i++) {
        float valor = produtos[i].preco * (float)produtos[i].quantidade;
        total_estoque += valor;

        printf("| %6d | %-24s | %10.2f | %4d | %14.2f |\n",
               produtos[i].codigo,
               produtos[i].nome,
               produtos[i].preco,
               produtos[i].quantidade,
               valor);
    }

    printf("+--------+--------------------------+------------+------+----------------+\n");
    printf("Valor total do estoque: R$ %.2f\n", total_estoque);
}

void exibir_produto(const Produto *produto) {
    float valor = produto->preco * (float)produto->quantidade;
    printf("Codigo: %d\n", produto->codigo);
    printf("Nome: %s\n", produto->nome);
    printf("Preco: %.2f\n", produto->preco);
    printf("Quantidade: %d\n", produto->quantidade);
    printf("Valor em estoque: %.2f\n", valor);
}

void atualizar_quantidade(Produto *produto, int nova_quantidade) {
    produto->quantidade = nova_quantidade;
}

void atualizar_estoque(Produto *produtos, int total) {
    int codigo;
    int nova_quantidade;
    Produto *produto = NULL;

    printf("\n--- Atualizar Estoque ---\n");

    if (total == 0) {
        printf("Nenhum produto cadastrado.\n");
        return;
    }

    if (!ler_int("Codigo do produto: ", &codigo)) {
        printf("Falha ao ler codigo.\n");
        return;
    }

    produto = buscar_produto(produtos, total, codigo);
    if (produto == NULL) {
        printf("Produto com codigo %d nao encontrado.\n", codigo);
        return;
    }

    if (!ler_int("Nova quantidade: ", &nova_quantidade)) {
        printf("Falha ao ler nova quantidade.\n");
        return;
    }
    if (nova_quantidade < 0) {
        printf("Quantidade nao pode ser negativa.\n");
        return;
    }

    atualizar_quantidade(produto, nova_quantidade);
    printf("Estoque atualizado com sucesso!\n");
}

void buscar_produto_menu(Produto *produtos, int total) {
    int codigo;
    Produto *encontrado;

    printf("\n--- Buscar Produto ---\n");
    if (total == 0) {
        printf("Nenhum produto cadastrado.\n");
        return;
    }

    if (!ler_int("Codigo do produto: ", &codigo)) {
        printf("Falha ao ler codigo.\n");
        return;
    }

    encontrado = buscar_produto(produtos, total, codigo);
    if (encontrado == NULL) {
        printf("Produto com codigo %d nao encontrado.\n", codigo);
        return;
    }

    exibir_produto(encontrado);
}

void remover_produto(Produto **produtos, int *total) {
    int codigo;
    int indice = -1;
    int i;

    printf("\n--- Remover Produto ---\n");

    if (*total == 0) {
        printf("Nenhum produto cadastrado.\n");
        return;
    }

    if (!ler_int("Codigo do produto: ", &codigo)) {
        printf("Falha ao ler codigo.\n");
        return;
    }

    for (i = 0; i < *total; i++) {
        if ((*produtos)[i].codigo == codigo) {
            indice = i;
            break;
        }
    }

    if (indice == -1) {
        printf("Produto com codigo %d nao encontrado.\n", codigo);
        return;
    }

    printf("Produto \"%s\" removido com sucesso!\n", (*produtos)[indice].nome);

    free((*produtos)[indice].nome);
    (*produtos)[indice].nome = NULL;

    for (i = indice; i < *total - 1; i++) {
        (*produtos)[i] = (*produtos)[i + 1];
    }

    (*total)--;

    if (*total == 0) {
        free(*produtos);
        *produtos = NULL;
        return;
    }

    Produto *novo_vetor = (Produto *)realloc(*produtos, (size_t)(*total) * sizeof(Produto));
    if (novo_vetor != NULL) {
        *produtos = novo_vetor;
    } else {
        printf("Aviso: nao foi possivel reduzir o vetor agora (dados permanecem validos).\n");
    }
}

void liberar_memoria(Produto **produtos, int *total) {
    int i;

    printf("\nLiberando memoria...\n");

    for (i = 0; i < *total; i++) {
        printf("Memoria do produto \"%s\" liberada.\n", (*produtos)[i].nome);
        free((*produtos)[i].nome);
        (*produtos)[i].nome = NULL;
    }

    free(*produtos);
    *produtos = NULL;
    *total = 0;

    printf("Vetor de produtos liberado.\n");
    printf("Programa encerrado.\n");
}

int main(void) {
    Produto *produtos = NULL;
    int total_produtos = 0;
    int proximo_codigo = 1;
    int opcao;

    while (1) {
        printf("\n========================================\n");
        printf("    SISTEMA DE CADASTRO DE PRODUTOS\n");
        printf("========================================\n");
        printf("1. Adicionar produto\n");
        printf("2. Listar produtos\n");
        printf("3. Buscar produto por codigo\n");
        printf("4. Atualizar estoque\n");
        printf("5. Remover produto\n");
        printf("6. Sair\n");

        if (!ler_int("Opcao: ", &opcao)) {
            printf("Falha de leitura. Encerrando.\n");
            break;
        }

        switch (opcao) {
            case 1:
                adicionar_produto(&produtos, &total_produtos, &proximo_codigo);
                break;

            case 2:
                listar_produtos(produtos, total_produtos);
                break;

            case 3:
                buscar_produto_menu(produtos, total_produtos);
                break;

            case 4:
                atualizar_estoque(produtos, total_produtos);
                break;

            case 5:
                remover_produto(&produtos, &total_produtos);
                break;

            case 6:
                liberar_memoria(&produtos, &total_produtos);
                return 0;

            default:
                printf("Opcao invalida. Tente novamente.\n");
                break;
        }
    }

    liberar_memoria(&produtos, &total_produtos);
    return 0;
}
