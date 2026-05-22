# Entrega - Lab 04: Media com MPI_Reduce

**Disciplina:** Laboratorio de Programacao Distribuida  
**Aluno:** Marcello Gonzatto Birkan

---

## Visao Geral

Este laboratorio implementa um programa em C com MPI para calcular a media global de valores aleatorios distribuidos entre varios processos. Cada processo cria um vetor local de tamanho `N`, calcula sua soma e media local, e depois a soma global eh obtida no processo de rank `0` com `MPI_Reduce`.

---

## Estrutura do Projeto

| Arquivo | Funcao |
|---------|--------|
| `media_mpi.c` | Codigo principal com geracao dos valores, calculo local, impressao ordenada e reducao global |
| `Makefile` | Compilacao com `mpicc` e execucao com `mpirun` |
| `README.md` | Resumo da atividade e instrucoes de uso |

---

## Trechos Importantes do Codigo

### 1. Leitura e validacao do tamanho local

O programa recebe um unico argumento pela linha de comando: `N`, o tamanho do vetor local criado por cada processo. Esse valor precisa ser um inteiro positivo.

```c
if (argc != 2 || !parse_local_size(argv[1], &local_size)) {
    if (rank == 0) {
        fprintf(stderr, "Uso: mpirun -np <processos> ./media_mpi <N>\n");
        fprintf(stderr, "N deve ser um inteiro positivo.\n");
    }
    MPI_Finalize();
    return 1;
}
```

### 2. Geracao do vetor local

Cada processo aloca seu proprio vetor e usa uma semente baseada no tempo atual somado ao `rank`. Assim, os processos tendem a gerar sequencias diferentes de valores aleatorios.

```c
srand((unsigned int)time(NULL) + (unsigned int)rank);

for (int i = 0; i < local_size; i++) {
    local_values[i] = rand() / (float)RAND_MAX;
    local_sum += local_values[i];
}
```

Os valores gerados estao no intervalo `[0, 1]`.

### 3. Soma e media local

Depois de gerar os dados, cada processo calcula sua media local dividindo a soma local pelo tamanho do vetor:

```c
local_avg = local_sum / local_size;
```

Esses calculos sao independentes, pois cada processo trabalha apenas com o seu vetor local.

### 4. Controle da ordem de impressao

Para evitar que as mensagens dos processos aparecam misturadas no terminal, o programa usa uma passagem simples de token com `MPI_Recv` e `MPI_Send`.

```c
wait_for_turn(rank);
printf("[Processo %d] Soma local: %.3f, Media local: %.4f\n", rank, local_sum, local_avg);
fflush(stdout);
signal_next_rank(rank, size);
```

Com isso, os processos imprimem seus resultados locais em ordem de rank.

### 5. Reducao com `MPI_Reduce`

A soma global eh calculada no processo `0` com `MPI_Reduce`, usando a operacao `MPI_SUM`.

```c
MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
```

Depois da reducao, o rank `0` divide a soma global pela quantidade total de valores, que eh `N * quantidade_de_processos`.

```c
double global_avg = global_sum / ((double)local_size * size);
```

---

## Funcoes MPI Utilizadas

| Funcao | Finalidade |
|--------|------------|
| `MPI_Init` | Inicializa o ambiente MPI |
| `MPI_Comm_rank` | Descobre o rank do processo atual |
| `MPI_Comm_size` | Informa quantos processos foram criados |
| `MPI_Allreduce` | Verifica se algum processo falhou ao alocar memoria |
| `MPI_Send` / `MPI_Recv` | Controlam a ordem de impressao no terminal |
| `MPI_Reduce` | Soma os resultados locais no processo raiz |
| `MPI_Finalize` | Encerra o ambiente MPI |

---

## Compilacao e Execucao

O `Makefile` do projeto automatiza a compilacao e a execucao:

```bash
make run
```

Ele equivale, por padrao, aos comandos:

```bash
mpicc -Wall -Wextra -pedantic -std=c11 -o media_mpi media_mpi.c
mpirun -np 4 ./media_mpi 1000
```

Tambem eh possivel alterar o numero de processos e o tamanho local:

```bash
make run NP=4 N=1000
```

---

## Resultado Esperado

A saida mostra a soma e a media local de cada processo, seguidas da soma global e da media global calculadas pelo rank `0`.

```text
[Processo 0] Soma local: 499.837, Media local: 0.4998
[Processo 1] Soma local: 498.912, Media local: 0.4989
[Processo 2] Soma local: 500.441, Media local: 0.5004
[Processo 3] Soma local: 501.322, Media local: 0.5013

[Soma global] 2000.512
[Media global] 0.5001
```

Como os valores sao aleatorios, a saida muda em cada execucao. Mesmo assim, as medias locais e global devem ficar proximas de `0.5`, pois os valores sao gerados uniformemente no intervalo `[0, 1]`.
