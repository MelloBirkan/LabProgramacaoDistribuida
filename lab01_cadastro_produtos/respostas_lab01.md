# Atividade 01 - Respostas Teóricas

## 1) Por que usamos `char*` para o nome em vez de `char nome[50]`?

`char nome[50]` reserva 50 bytes sempre. "Mouse" tem 5 caracteres — os outros 45 ficam parados sem fazer nada. Com `char*` e `malloc`, alocamos só o que o nome precisa, e ainda não ficamos presos num limite arbitrário de 49 caracteres.

O custo é gerenciar memória na mão: `malloc`, `realloc`, `free`. Um `free` esquecido vira leak. Mas para nomes de tamanho imprevisível, o array fixo cria mais problema do que resolve.

## 2) Diferença entre `Produto*` e `Produto**` no contexto do código

`Produto*` aponta para um produto ou para o início do vetor. Serve para ler e alterar campos.

`Produto**` aponta para o próprio ponteiro do vetor — necessário quando a função pode realocar o vetor e mudar seu endereço. Sem ele, a função recebe uma cópia do ponteiro, redireciona internamente, e o `main` nunca sabe. O vetor some.

## 3) Na adição, por que precisamos de `realloc`?

Porque não sabemos na compilação quantos produtos o usuário vai cadastrar.

`realloc` estica o bloco existente para `total + 1`, mantendo os dados. Com `malloc` puro, faríamos isso na mão: novo bloco, cópia manual, `free` do antigo. `realloc` comprime tudo numa chamada — quando consegue alocar.

## 4) Na remoção, por que a ordem de liberação importa?

Os endereços dos `nome`s estão guardados dentro do vetor. Se liberarmos o vetor primeiro, perdemos esses endereços — não tem mais como chegar nos nomes para liberar. Memória perdida.

Libera cada `produto.nome`, depois libera o vetor.

## 5) Diagrama de memória após adicionar 3 produtos

```text
produtos (Produto*)
   |
   v
+----------------+----------------+----------------+
| [0] codigo=1   | [1] codigo=2   | [2] codigo=3   |
| nome ----+     | nome ----+     | nome ----+     |
| preco    |     | preco    |     | preco    |     |
| qtd      |     | qtd      |     | qtd      |     |
+----------|-----+----------|-----+----------|-----+
           |                |                |
           v                v                v
      "Notebook"          "Mouse"        "Teclado"
        (heap)             (heap)          (heap)
```

Um bloco contínuo para o vetor de `Produto`. Cada `nome` aponta para outro bloco separado no heap.

## 6) O que é memory leak e onde poderia ocorrer

Memória alocada que o programa não consegue mais acessar — e portanto nunca libera.

Aqui os pontos críticos são três: remover produto sem `free(produto.nome)`, encerrar sem liberar nomes e vetor, e `realloc` falhar na adição — se o ponteiro virar `NULL`, o `nome_dinamico` já alocado some junto.

## 7) Por que `buscar_produto` retorna `Produto*` e não `Produto` por valor?

`NULL` é a resposta para "não encontrado". Retornando `Produto` por valor, não existe equivalente natural para ausência — teria que inventar um código de erro ou flag extra.

O ponteiro também permite alterar o produto direto no vetor. Útil para atualizar estoque sem fazer cópia nenhuma.

## 8) Por que usar passagem por referência na atualização de estoque?

C passa struct por valor. A função recebe uma cópia, muda a cópia, a cópia some. O produto no vetor não muda.

Com `Produto*`, a função acessa o produto real. A mudança fica.
