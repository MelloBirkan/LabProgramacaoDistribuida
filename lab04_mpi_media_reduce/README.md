# Media com MPI_Reduce

Programa em C que calcula a media global de valores aleatorios distribuidos entre processos MPI. Cada processo gera um vetor local com `N` valores no intervalo `[0, 1]`, calcula sua soma e media local, e participa de uma reducao para obter a soma global no processo raiz.

## Visao geral

Cada processo executa a mesma rotina: recebe o tamanho local `N` pela linha de comando, gera seus dados aleatorios, calcula a soma local e imprime sua media. Depois, `MPI_Reduce` agrega todas as somas locais no processo de rank `0`, que calcula e exibe a media global.

## Estrutura do projeto

```text
lab04_mpi_media_reduce/
|-- media_mpi.c
|-- Makefile
|-- README.md
`-- entrega.md
```

## Responsabilidade dos arquivos

| Arquivo | Funcao |
|---------|--------|
| `media_mpi.c` | Programa principal com geracao dos vetores locais, calculo das medias e reducao global |
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
mpicc -Wall -Wextra -pedantic -std=c11 -o media_mpi media_mpi.c
```

## Execucao

```bash
make run
```

O alvo `run` usa 4 processos e `N = 1000` por padrao. Esses valores podem ser alterados:

```bash
make run NP=6 N=5000
```

Ou manualmente:

```bash
mpirun -np 4 ./media_mpi 1000
```

## Exemplo de saida

```text
[Processo 0] Soma local: 499.837, Media local: 0.4998
[Processo 1] Soma local: 498.912, Media local: 0.4989
[Processo 2] Soma local: 500.441, Media local: 0.5004
[Processo 3] Soma local: 501.322, Media local: 0.5013

[Soma global] 2000.512
[Media global] 0.5001
```

Como os valores sao aleatorios, os numeros mudam a cada execucao.

## Conceitos tecnicos demonstrados

- `MPI_Reduce` com `MPI_SUM`: soma das contribuicoes locais no processo raiz
- Geracao independente de dados em cada processo com `srand(time(NULL) + rank)`
- Calculo de soma e media local
- Calculo de media global no rank `0`
- `MPI_Send`/`MPI_Recv`: controle simples da ordem de impressao no terminal
