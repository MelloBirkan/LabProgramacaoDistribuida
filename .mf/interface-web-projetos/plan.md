# Plan: Interface Web dos Projetos

## Problem & Context

O repositório reúne três atividades da disciplina de Programação Distribuída, mas hoje a navegação e apresentação dependem de leitura direta de README e código-fonte. Isso dificulta mostrar rapidamente o valor de cada projeto, os conceitos distribuídos envolvidos e os resultados visuais já produzidos. O trabalho consiste em criar um microsite estático, em português, na raiz do repositório para apresentar os labs de forma mais clara e demonstrável. A solução precisa funcionar apenas com HTML, CSS e JavaScript puros, sem backend e sem framework. Também deve ser simples de abrir localmente no navegador, mantendo espaço para futura publicação estática.

Principais afetados: o autor do repositório, professores/colegas avaliando os projetos e qualquer pessoa que queira entender rapidamente o que foi implementado em cada lab.

Restrições principais:
- site estático com páginas `.html` separadas e links normais;
- demos no navegador serão simulações pedagógicas, não execução dos programas C;
- o conteúdo deve reaproveitar os materiais já existentes no repositório, incluindo screenshots do `lab01`;
- a implementação deve ficar na raiz do projeto e não pode depender de build step obrigatório.

## User Experience

### Fluxo 1: Entrada e visão geral

O usuário entra pela home e entende em poucos segundos que o site reúne projetos da disciplina. Ele vê uma apresentação breve da disciplina/equipe, um resumo dos três labs e chamadas claras para abrir cada projeto. A home deve destacar o tipo de problema resolvido por cada lab e a tecnologia principal usada.

### Fluxo 2: Explorar um lab específico

Ao abrir a página de um lab, o usuário encontra primeiro o contexto do projeto e o objetivo da atividade. Em seguida, vê os conceitos técnicos principais, comandos de compilação/execução e evidências do trabalho realizado, como imagens e trechos descritivos. A demo aparece como área de exploração prática para reforçar o funcionamento do projeto.

### Fluxo 3: Interagir com demos

No `lab01`, o usuário consegue cadastrar, listar, buscar, atualizar e remover produtos em uma interface simples que reflita as regras do programa original. No `lab02`, ele acompanha uma partida simulada com regras visíveis, validação de palavras e placar entre dois jogadores. No `lab03`, ele visualiza a distribuição dos números entre processos, o cálculo local e a redução final da soma.

### Hierarquia de informação

Informação crítica:
- nome e objetivo de cada projeto;
- qual conceito de programação distribuída ele demonstra;
- demo interativa;
- como compilar/executar o projeto real.

Informação secundária:
- capturas de tela;
- detalhes adicionais sobre implementação;
- contexto da disciplina e da equipe.

### Feedback e estados

Toda demo deve indicar claramente o que aconteceu após cada ação do usuário. Estados vazios, validações, erros de entrada e resultados finais precisam aparecer de forma legível e imediata. Nas páginas, links ativos e navegação atual devem ficar visíveis para evitar desorientação.

### Edge cases percebidos pelo usuário

Se uma demo receber entrada inválida, a interface informa a regra quebrada em linguagem simples. Se não houver itens cadastrados ou resultado para mostrar, a tela comunica o estado vazio em vez de parecer quebrada. Em telas com simulação por etapas, o usuário deve conseguir reiniciar facilmente o fluxo.

## Technical Approach

### Architectural Approach

A implementação será um microsite multipágina com HTML estático e assets compartilhados de CSS e JavaScript. Essa escolha atende ao requisito de simplicidade, reduz custo de manutenção e permite abrir o site localmente sem ferramentas adicionais.

Decisões principais e trade-offs:
- HTML por página em vez de SPA:
  páginas separadas simplificam navegação, publicação estática e leitura do código; em troca, há alguma repetição estrutural entre páginas.
- CSS compartilhado com identidade visual única:
  mantém consistência visual entre home, página sobre e páginas dos labs; em troca, exige disciplina para evitar folhas de estilo monolíticas.
- JavaScript modular por responsabilidade:
  um núcleo comum para navegação/comportamentos globais e scripts específicos para cada demo; isso reduz acoplamento e evita carregar lógica desnecessária em todas as páginas.
- Conteúdo baseado no repositório existente:
  descrições, comandos e imagens serão derivados dos labs já feitos; isso minimiza invenção e mantém o site fiel ao material entregue.
- Demos simuladas:
  tornam os conceitos navegáveis no browser sem backend nem compilação; em troca, precisam deixar explícito que representam o comportamento do programa original, não sua execução literal.

Resiliência esperada:
- o site deve permanecer utilizável sem servidor local sofisticado;
- cada página deve funcionar isoladamente, desde que os assets compartilhados estejam presentes;
- falhas em uma demo não devem comprometer a navegação global do site.

### Component Architecture

Os principais blocos do site serão:

- Layout base compartilhado:
  cabeçalho, navegação principal, rodapé e estrutura visual comum entre páginas.
- Home:
  hero de apresentação, resumo da disciplina/equipe, grade com os três projetos e links para exploração.
- Página Sobre:
  contexto da disciplina, objetivo do repositório e apresentação da equipe/autoria.
- Página do Lab 01:
  resumo do sistema de cadastro, comandos, galeria com screenshots existentes e demo de CRUD de produtos.
- Página do Lab 02:
  explicação do protocolo e das regras do jogo, visão resumida de cliente/servidor e demo de rodada/partida simulada.
- Página do Lab 03:
  explicação do uso de `MPI_Scatter` e `MPI_Reduce`, visão do particionamento do vetor e demo visual da soma paralela.
- Scripts de demo:
  módulos independentes por projeto, com estado local e controles próprios.

Interfaces e responsabilidades:
- o layout compartilhado fornece navegação consistente e estilo comum;
- cada página de lab é responsável por seu conteúdo estático e pela montagem da demo correspondente;
- cada demo encapsula seu próprio estado, validação e renderização;
- assets de imagem ficam referenciados diretamente a partir do que já existe no repositório, evitando duplicação quando possível.

Integração e fluxo de dados:
- dados textuais fixos podem ser definidos diretamente no HTML ou em objetos JavaScript simples;
- o estado das demos existe apenas em memória no navegador;
- interações de usuário atualizam a interface localmente, sem persistência nem comunicação externa.
