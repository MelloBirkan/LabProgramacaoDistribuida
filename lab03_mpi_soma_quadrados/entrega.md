# Entrega - Lab 03: Soma de Quadrados com MPI

**Disciplina:** Laboratorio de Programacao Distribuida  
**Aluno:** Marcello Gonzatto Birkan

---

## Visao Geral

Este laboratorio implementa um programa em C com MPI para calcular a soma dos quadrados dos numeros de `1` a `40`. A ideia principal eh dividir o vetor entre os processos, calcular somas parciais em paralelo e depois juntar tudo no processo raiz para verificar se o resultado bate com a formula matematica conhecida.

---

## Estrutura do Projeto

| Arquivo | Funcao |
|---------|--------|
| `soma_quadrados.c` | Codigo principal com distribuicao, calculo local, reducao e validacao |
| `Makefile` | Compilacao com `mpicc` e execucao com `mpirun` |
| `README.md` | Resumo do laboratorio e instrucoes de uso |

---

## Trechos Importantes do Codigo

### 1. Geracao do vetor no processo raiz

Somente o processo de `rank 0` inicializa o vetor completo. Isso evita trabalho repetido e deixa claro que a distribuicao parte de um unico processo.

```c
if (rank == 0) {
    for (int i = 0; i < N; i++) {
        vector[i] = i + 1;
    }
}
```

Nesse caso, o vetor gerado eh `[1, 2, 3, ..., 40]`.

### 2. Distribuicao com `MPI_Scatter`

Depois da inicializacao, o vetor eh dividido igualmente entre os processos com `MPI_Scatter`.

```c
MPI_Scatter(vector, CHUNK, MPI_INT, local_data, CHUNK, MPI_INT, 0, MPI_COMM_WORLD);
```

Como `N = 40` e `NUM_PROCS = 4`, cada processo recebe `10` elementos:

- processo 0: `1` a `10`;
- processo 1: `11` a `20`;
- processo 2: `21` a `30`;
- processo 3: `31` a `40`.

### 3. Soma local dos quadrados

Cada processo trabalha apenas com sua parte do vetor e calcula a soma local de forma independente.

```c
for (int i = 0; i < CHUNK; i++) {
    local_sum += local_data[i] * local_data[i];
}
```

Esse passo representa a parte paralela do laboratorio, porque todos os processos podem executar esse calculo ao mesmo tempo.

### 4. Controle da ordem de impressao

Um detalhe interessante do codigo eh o uso das funcoes `wait_for_turn()` e `signal_next_rank()`. Elas passam um pequeno token entre os processos com `MPI_Recv` e `MPI_Send` para evitar que as mensagens saiam embaralhadas no terminal.

Assim, primeiro aparece o bloco de cada processo com os dados recebidos e depois o bloco com as somas locais, em ordem de rank.

### 5. Reducao e verificacao final

Depois das somas locais, o programa usa `MPI_Reduce` com `MPI_SUM` para juntar todos os resultados no processo raiz.

```c
MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
```

No final, o `rank 0` compara a soma paralela com a formula:

```c
int seq_sum = N * (N + 1) * (2 * N + 1) / 6;
```

Para `N = 40`, o valor esperado eh `22140`, que coincide com a saida do programa.

---

## Funcoes MPI Utilizadas

| Funcao | Finalidade |
|--------|------------|
| `MPI_Init` | Inicializa o ambiente MPI |
| `MPI_Comm_rank` | Descobre o rank do processo atual |
| `MPI_Comm_size` | Informa quantos processos foram criados |
| `MPI_Scatter` | Distribui o vetor em partes iguais |
| `MPI_Send` / `MPI_Recv` | Controlam a ordem de impressao no terminal |
| `MPI_Reduce` | Soma os resultados locais no processo raiz |
| `MPI_Finalize` | Encerra o ambiente MPI |

---

## Compilacao e Execucao

O `Makefile` do projeto ja automatiza a compilacao e a execucao:

```bash
make run
```

Ele equivale aos comandos:

```bash
mpicc -Wall -Wextra -pedantic -std=c11 -o soma_quadrados soma_quadrados.c
mpirun -np 4 ./soma_quadrados
```

---

## Resultado Observado

Na execucao, cada processo recebe sua faixa correta de valores e calcula a soma local correspondente:

- processo 0: `385`
- processo 1: `2485`
- processo 2: `6585`
- processo 3: `12685`

A soma final paralela eh `22140`, igual ao valor sequencial esperado. Isso mostra que a distribuicao dos dados e a reducao final foram executadas corretamente.

---

## Capturas de Tela da Execucao

### Execucao do programa

![Execucao do Lab 03](./CleanShot%202026-04-10%20at%2005.33.13@2x.png)

### Espaco para outras imagens

- Captura adicional 1:
- Captura adicional 2:


