# Entrega - Lab 02: Batalha de Palavras em Rede

**Disciplina:** Laboratorio de Programacao Distribuida  
**Aluno:** Marcello Gonzatto Birkan

---

## Visao Geral

Este laboratorio implementa um jogo cliente-servidor em C no qual dois jogadores disputam rodadas com palavras. Em cada rodada, o servidor sorteia uma letra e os clientes precisam responder com uma palavra valida que comece com essa letra. A aplicacao usa sockets TCP para a comunicacao e threads POSIX para permitir partidas simultaneas.

---

## Organizacao do Projeto

| Arquivo | Papel no projeto |
|---------|------------------|
| `protocolo.h` | Define os tipos de mensagem usados na comunicacao |
| `jogo.h` | Reune as assinaturas compartilhadas entre cliente e servidor |
| `jogo.c` | Implementa a logica principal da partida |
| `servidor.c` | Aceita conexoes, organiza jogadores e inicia partidas |
| `cliente.c` | Recebe eventos do servidor e envia as respostas do usuario |
| `Makefile` | Automatiza compilacao e execucao |

---

## Trechos Importantes do Codigo

### 1. Protocolo de mensagens

O protocolo foi montado em formato textual, seguindo o padrao `TIPO|conteudo\n`. Isso simplifica o envio e a leitura das mensagens, porque tanto cliente quanto servidor conseguem identificar a acao apenas olhando o prefixo.

Alguns tipos definidos em `protocolo.h` sao:

```c
#define TIPO_MSG       "MSG"
#define TIPO_RODADA    "RODADA"
#define TIPO_RESULTADO "RESULTADO"
#define TIPO_PLACAR    "PLACAR"
#define TIPO_PALAVRA   "PALAVRA"
#define TIPO_TIMEOUT   "TIMEOUT"
```

### 2. Emparelhamento dos jogadores

No servidor, o primeiro cliente fica aguardando ate a chegada de um segundo jogador. Quando a dupla esta formada, a partida pode comecar em uma thread separada. A variavel global usada para guardar o jogador em espera precisa ser protegida por mutex para evitar erro quando varias conexoes chegam ao mesmo tempo.

```c
if (g_aguardando == NULL) {
    g_aguardando = novo;
} else {
    Jogador *primeiro = g_aguardando;
    g_aguardando = NULL;
    iniciar_partida(primeiro, novo);
}
```

### 3. Coleta simultanea das respostas

Um ponto importante do laboratorio e o uso de `select()` para esperar dados dos dois jogadores dentro do mesmo intervalo de tempo. Assim, o servidor consegue controlar o deadline da rodada sem bloquear em apenas um socket.

```c
while (!pronto1 || !pronto2) {
    FD_ZERO(&leitura);
    if (!pronto1) FD_SET(fd1, &leitura);
    if (!pronto2) FD_SET(fd2, &leitura);
}
```

Essa parte eh importante porque garante justica entre os jogadores: os dois respondem sob o mesmo limite de tempo.

### 4. Cliente com timeout no teclado

No cliente, a leitura da palavra digitada tambem pode usar `select()`, agora no `stdin`. Isso permite detectar quando o tempo acabou e enviar automaticamente uma mensagem de timeout, sem travar o programa esperando a entrada do usuario.

### 5. Validacao das palavras

A validacao verifica se a palavra:

- nao esta vazia;
- tem tamanho minimo;
- comeca com a letra sorteada;
- contem apenas caracteres aceitos.

Tambem existe a comparacao entre as respostas dos dois jogadores para impedir que palavras iguais pontuem ao mesmo tempo.

---

## Compilacao e Execucao

```bash
make
./servidor
./cliente
```

Com isso, o servidor fica aguardando conexoes e os clientes podem entrar na partida.

---

## Observacoes Finais

Este laboratorio exercita conceitos importantes de computacao distribuida e redes, como protocolo de aplicacao, sincronizacao entre threads, multiplexacao com `select()` e tratamento de conexoes concorrentes.

---

## Capturas de Tela da Execucao

Adicione aqui as imagens de execucao do servidor e dos clientes.

- Captura 1:
- Captura 2:


