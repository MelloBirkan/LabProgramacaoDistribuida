# Entrega - Lab 05: Transformacao Paralela de Dados com MPI

**Disciplina:** Laboratorio de Programacao Distribuida  
**Aluno:** Marcello Gonzatto Birkan

---

## Visao Geral

Este laboratorio implementa um programa em C com MPI para aplicar uma transformacao matematica sobre um vetor de 100 inteiros. O processo de `rank 0` cria o vetor original com os valores de `1` a `100`, distribui partes iguais para 5 processos com `MPI_Scatter`, cada processo calcula o quadrado dos seus 20 elementos e o resultado final eh reunido no `rank 0` com `MPI_Gather`.

---

## Estrutura do Projeto

| Arquivo | Funcao |
|---------|--------|
| `transformacao_dados_mpi.c` | Codigo principal com criacao do vetor, distribuicao, transformacao local e reuniao dos resultados |
| `Makefile` | Compilacao com `mpicc` e execucao com `mpirun` |
| `README.md` | Resumo da atividade e instrucoes de uso |

---

## Trechos Importantes do Codigo

### 1. Definicao dos parametros fixos

O programa usa constantes para representar o tamanho total do vetor, a quantidade obrigatoria de processos e o tamanho da fatia recebida por cada processo.

```c
#define DATA_SIZE 100
#define NUM_PROCS 5
#define CHUNK_SIZE (DATA_SIZE / NUM_PROCS)
```

Como `DATA_SIZE = 100` e `NUM_PROCS = 5`, cada processo recebe exatamente `20` elementos.

### 2. Validacao da quantidade de processos

A atividade exige exatamente 5 processos MPI. Por isso, o programa verifica o tamanho do comunicador antes de executar a distribuicao dos dados.

```c
if (size != NUM_PROCS) {
    if (rank == 0) {
        fprintf(stderr, "Erro: execute com exatamente %d processos.\n", NUM_PROCS);
        fprintf(stderr, "Exemplo: mpirun -np %d ./transformacao_dados_mpi\n", NUM_PROCS);
    }
    MPI_Finalize();
    return 1;
}
```

### 3. Criacao do vetor original no rank 0

Somente o processo mestre inicializa o vetor completo. Os demais processos recebem apenas suas partes depois da chamada de `MPI_Scatter`.

```c
if (rank == 0) {
    for (int i = 0; i < DATA_SIZE; i++) {
        original_data[i] = i + 1;
    }

    print_vector("[Processo 0] Vetor original", original_data, DATA_SIZE);
}
```

### 4. Distribuicao com `MPI_Scatter`

O vetor original eh dividido igualmente entre os processos. Cada processo recebe 20 inteiros em seu vetor local.

```c
MPI_Scatter(original_data, CHUNK_SIZE, MPI_INT, local_data, CHUNK_SIZE, MPI_INT, 0, MPI_COMM_WORLD);
```

### 5. Transformacao local

Cada processo aplica a funcao `f(x) = x * x` apenas nos elementos que recebeu.

```c
for (int i = 0; i < CHUNK_SIZE; i++) {
    local_data[i] = local_data[i] * local_data[i];
}
```

Esse passo representa a parte paralela da atividade, pois todos os processos podem transformar suas partes ao mesmo tempo.

### 6. Reuniao com `MPI_Gather`

Depois da transformacao local, os vetores parciais sao enviados de volta para o processo de `rank 0`.

```c
MPI_Gather(local_data, CHUNK_SIZE, MPI_INT, transformed_data, CHUNK_SIZE, MPI_INT, 0, MPI_COMM_WORLD);
```

No final, o `rank 0` imprime o vetor completo ja transformado.

---

## Funcoes MPI Utilizadas

| Funcao | Finalidade |
|--------|------------|
| `MPI_Init` | Inicializa o ambiente MPI |
| `MPI_Comm_rank` | Descobre o rank do processo atual |
| `MPI_Comm_size` | Informa quantos processos foram criados |
| `MPI_Scatter` | Distribui o vetor original em partes iguais |
| `MPI_Gather` | Reune as partes transformadas no processo raiz |
| `MPI_Finalize` | Encerra o ambiente MPI |

---

## Compilacao e Execucao

O `Makefile` do projeto automatiza a compilacao e a execucao:

```bash
make run
```

Ele equivale aos comandos:

```bash
mpicc -Wall -Wextra -pedantic -std=c11 -o transformacao_dados_mpi transformacao_dados_mpi.c
mpirun -np 5 ./transformacao_dados_mpi
```

---

## Resultado Esperado

A saida mostra o vetor original e o vetor transformado no processo de `rank 0`.

```text
[Processo 0] Vetor original: [1, 2, 3, ..., 100]
[Processo 0] Vetor transformado: [1, 4, 9, ..., 10000]
```

O primeiro vetor contem os inteiros de `1` a `100`. O segundo vetor contem o quadrado de cada posicao, preservando a ordem original dos dados depois da reuniao com `MPI_Gather`.
