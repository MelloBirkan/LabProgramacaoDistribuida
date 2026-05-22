# Soma de Quadrados com MPI

Programa em C que calcula a soma dos quadrados de 1 a 40 utilizando processamento paralelo com MPI (Message Passing Interface), distribuindo o trabalho entre 4 processos.

## Visao geral

O processo raiz (rank 0) gera um vetor com os inteiros de 1 a 40 e distribui partes iguais para cada processo usando `MPI_Scatter`. Cada processo calcula a soma dos quadrados da sua fatia local. Os resultados parciais sao agregados no processo raiz com `MPI_Reduce` e validados contra a formula analitica N(N+1)(2N+1)/6.

## Estrutura do projeto

```text
lab03_mpi_soma_quadrados/
├── soma_quadrados.c
├── Makefile
└── README.md
```

## Responsabilidade dos arquivos

| Arquivo | Funcao |
|---------|--------|
| `soma_quadrados.c` | Programa principal com geracao do vetor, distribuicao, calculo local, reducao e validacao |
| `Makefile` | Compilacao e execucao automatizadas com `mpicc` e `mpirun` |

## Compilacao

Na pasta do projeto:

```bash
make
```

Para remover o binario:

```bash
make clean
```

Compilacao manual:

```bash
mpicc -Wall -Wextra -pedantic -std=c11 -o soma_quadrados soma_quadrados.c
```

## Execucao

```bash
make run
```

Ou manualmente:

```bash
mpirun -np 4 ./soma_quadrados
```

O programa imprime os dados recebidos por cada processo, as somas locais e a comparacao entre o resultado paralelo e o sequencial.

## Conceitos tecnicos demonstrados

- `MPI_Scatter`: distribuicao de partes iguais de um vetor do processo raiz para todos os processos
- `MPI_Reduce` com `MPI_SUM`: agregacao dos resultados parciais em um unico valor no processo raiz
- `MPI_Send`/`MPI_Recv`: passagem de token ponto-a-ponto entre processos para ordenar a saida no terminal
- Validacao do resultado paralelo contra formula analitica
- Compilacao com `mpicc` e execucao com `mpirun`
