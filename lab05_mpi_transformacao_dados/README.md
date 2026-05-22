# Transformacao Paralela de Dados com MPI

Programa em C que aplica uma transformacao matematica em um vetor de 100 inteiros utilizando processamento paralelo com MPI (Message Passing Interface). O vetor original eh distribuido igualmente entre 5 processos, cada processo calcula o quadrado dos seus elementos e os resultados sao reunidos no processo raiz.

## Visao geral

O processo raiz (rank 0) cria um vetor com os inteiros de 1 a 100 e imprime o vetor original. Em seguida, o vetor eh dividido em partes iguais com `MPI_Scatter`, enviando 20 elementos para cada processo. Cada processo aplica localmente a transformacao `x = x * x`. Por fim, `MPI_Gather` reune as partes transformadas no rank 0, que imprime o vetor final.

## Estrutura do projeto

```text
lab05_mpi_transformacao_dados/
|-- transformacao_dados_mpi.c
|-- Makefile
|-- README.md
`-- entrega.md
```

## Responsabilidade dos arquivos

| Arquivo | Funcao |
|---------|--------|
| `transformacao_dados_mpi.c` | Programa principal com criacao do vetor, distribuicao, transformacao local e reuniao dos resultados |
| `Makefile` | Compilacao e execucao automatizadas com `mpicc` e `mpirun` |
| `entrega.md` | Explicacao da solucao implementada |

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
mpicc -Wall -Wextra -pedantic -std=c11 -o transformacao_dados_mpi transformacao_dados_mpi.c
```

## Execucao

```bash
make run
```

Ou manualmente:

```bash
mpirun -np 5 ./transformacao_dados_mpi
```

O programa deve ser executado com exatamente 5 processos. Caso outro numero de processos seja usado, uma mensagem de erro sera exibida.

## Exemplo de saida

```text
[Processo 0] Vetor original: [1, 2, 3, ..., 100]
[Processo 0] Vetor transformado: [1, 4, 9, ..., 10000]
```

## Conceitos tecnicos demonstrados

- `MPI_Scatter`: distribuicao de partes iguais do vetor original para todos os processos
- Transformacao local independente em cada processo
- `MPI_Gather`: reuniao das partes transformadas no processo raiz
- Validacao do numero exato de processos MPI
- Compilacao com `mpicc` e execucao com `mpirun`
