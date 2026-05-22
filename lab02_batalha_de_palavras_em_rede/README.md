# Batalha de palavras em rede

Jogo multiplayer em texto, implementado em C com sockets TCP e pthreads.

## Visao geral

Dois jogadores se conectam ao servidor e disputam 5 rodadas. Em cada rodada:

1. O servidor sorteia uma letra aleatoria
2. Os clientes recebem 10 segundos para responder
3. A palavra enviada precisa:
   - comecar com a letra indicada
   - ter no minimo 5 caracteres
   - conter apenas letras ASCII (a-z, A-Z)
4. Palavras validas valem 1 ponto
5. Palavras invalidas, timeout ou palavras repetidas nao pontuam

Ao final das 5 rodadas, vence quem tiver mais pontos.

## Estrutura do projeto

```text
batalha_de_palavras_em_rede/
├── protocolo.h
├── jogo.h
├── jogo.c
├── servidor.c
├── cliente.c
├── Makefile
└── README.md
```

## Responsabilidade dos arquivos

| Arquivo | Funcao |
|---------|--------|
| `protocolo.h` | Constantes do jogo e tipos de mensagem trocados entre cliente e servidor |
| `jogo.h` / `jogo.c` | Logica compartilhada: validacao, geracao de letra, envio/recebimento e timeout |
| `servidor.c` | Servidor concorrente com pareamento de jogadores e controle da partida |
| `cliente.c` | Cliente interativo que interpreta o protocolo e usa `select()` no teclado |
| `Makefile` | Compilacao automatizada |

## Protocolo

Todas as mensagens sao linhas de texto no formato `TIPO|campo1|campo2|...` e terminadas por `\n`.

### Servidor -> Cliente

| Tipo | Exemplo | Descricao |
|------|---------|-----------|
| `MSG` | `MSG|Bem-vindo, Alice!` | Mensagem geral |
| `NOME` | `NOME|` | Solicita o nome do jogador |
| `AGUARDE` | `AGUARDE|Esperando outro jogador...` | Coloca o cliente em espera |
| `RODADA` | `RODADA|1|M|10` | Inicio de rodada |
| `RESULTADO` | `RESULTADO|Palavra "manga" valida! +1 ponto.` | Resultado individual |
| `PLACAR` | `PLACAR|Alice|1|Bob|0` | Placar atualizado |
| `FIM` | `FIM|Alice venceu! Placar final: Alice 3 x 2 Bob` | Encerramento da partida |

### Cliente -> Servidor

| Tipo | Exemplo | Descricao |
|------|---------|-----------|
| `NOME` | `NOME|Alice` | Envia o nome do jogador |
| `PALAVRA` | `PALAVRA|manga` | Envia a palavra da rodada |
| `TIMEOUT` | `TIMEOUT|` | Informa que o tempo acabou |

## Compilacao

Na pasta do projeto:

```bash
make
```

Para remover os binarios:

```bash
make clean
```

Compilacao manual:

```bash
gcc -Wall -Wextra -pedantic -std=c11 -o servidor servidor.c jogo.c -pthread
gcc -Wall -Wextra -pedantic -std=c11 -o cliente cliente.c jogo.c
```

## Execucao

### 1. Inicie o servidor

```bash
./servidor
./servidor 9000
```

### 2. Conecte dois clientes

Em terminais separados:

```bash
./cliente
./cliente 127.0.0.1 7070
./cliente 127.0.0.1 9000
```

## Regras implementadas

- 5 rodadas por partida
- 10 segundos por rodada
- validacao case-insensitive para a letra inicial
- palavras com menos de 5 caracteres sao invalidas
- apenas letras ASCII sao aceitas
- se os dois jogadores enviarem a mesma palavra, ninguem pontua
- se um jogador desconectar no meio da partida, o outro vence por W.O.

## Conceitos tecnicos demonstrados

- sockets TCP (`socket`, `bind`, `listen`, `accept`, `connect`, `send`, `recv`)
- servidor concorrente com `pthread_create` e `pthread_detach`
- protocolo de aplicacao baseado em texto
- timeout com `select()` no cliente e no recebimento do servidor
- validacao de entrada
- modularizacao em varios arquivos `.c` e `.h`
- tratamento de sinais (`SIGINT` e `SIGPIPE`)
