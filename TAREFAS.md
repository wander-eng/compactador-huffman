# Divisão de Tarefas — Compactador Huffman

## Visão Geral da Sequência

```
ETAPA 1        ETAPA 2        ETAPA 3        ETAPA 4        ETAPA 5
Fundação   →   Algoritmo  →   Arquivos   →   Interface  →   Finalização
heap.c         huffman.c      arquivo.c      main.c         testes + log.c
headers        (juntos)                                      ajustes finais
```

---

## ETAPA 1 — Fundação

### Pessoa 1 — Implementar `heap.c`
O min-heap é a base do algoritmo. Deve estar funcionando e testado antes de qualquer outra coisa.

**Tarefas:**
- [ ] Definir a struct do heap (vetor de ponteiros para `No`, tamanho, capacidade)
- [ ] Implementar `heap_inserir(Heap *h, No *no)`
- [ ] Implementar `heap_remover_min(Heap *h)` — retorna o nó de menor frequência
- [ ] Implementar `heapify_baixo(Heap *h, int i)` — usada no remover
- [ ] Implementar `heapify_cima(Heap *h, int i)` — usada no inserir
- [ ] Implementar `heap_criar(int capacidade)` e `heap_destruir(Heap *h)`
- [ ] Testar isoladamente: inserir nós com frequências variadas, verificar se `remover_min` sempre retorna o menor

### Pessoa 2 — Definir todos os headers
Enquanto o heap é construído, a Pessoa 2 define as interfaces de todos os módulos. Isso alinha os dois antes de partir para o algoritmo.

**Tarefas:**
- [ ] `heap.h` — declarar structs e funções públicas do heap
- [ ] `huffman.h` — declarar struct `No`, funções de construção da árvore, geração de códigos, serialização
- [ ] `arquivo.h` — declarar funções de compressão, descompressão, leitura e escrita de bits
- [ ] `log.h` — declarar função de registro de operação
- [ ] Criar `main.c` com o esqueleto do menu (só a estrutura, sem lógica ainda)

---

## ETAPA 2 — Algoritmo de Huffman (juntos)

Esta é a etapa mais complexa. Trabalhar juntos aqui reduz o risco de erros na lógica central.

### Ambos — Implementar `huffman.c`

**Tarefas:**
- [ ] `contar_frequencias(FILE *arquivo, int frequencias[256])` — lê o arquivo e preenche o vetor de 256 posições
- [ ] `construir_arvore(int frequencias[256])` — insere símbolos no heap e executa o algoritmo de Huffman, retorna a raiz
- [ ] `gerar_codigos(No *raiz, char tabela[256][256])` — percurso recursivo na árvore preenchendo a tabela de códigos
- [ ] `serializar_arvore(No *raiz, FILE *saida)` — grava a árvore em pré-ordem no arquivo
- [ ] `desserializar_arvore(FILE *entrada)` — reconstrói a árvore a partir do arquivo, retorna a raiz
- [ ] `destruir_arvore(No *raiz)` — libera memória recursivamente
- [ ] `tratar arquivo vazio como caso inválido para compactação` 
- [ ] `tratar arquivo com um único símbolo distinto como árvore de um nó só;`
- [ ] `validar geração de código e serialização nesse cenário.`
- [ ] `validar que a raiz possui frequência igual ao total de bytes do arquivo`

**Ponto de validação antes de continuar:**
Comprime uma string simples (`"aabbbcccc"`) e verifica se a árvore gerada e os códigos fazem sentido — folhas mais frequentes devem ter códigos mais curtos.

---

## ETAPA 3 — Leitura e Escrita de Arquivos

### Pessoa 1 — Implementar compressão em `arquivo.c`
- [ ] Implementar buffer de escrita de bits (`escrever_bit`, `flush_buffer`)
- [ ] `comprimir(const char *caminho_entrada, const char *caminho_saida)`:
  - Contar frequências
  - Construir árvore
  - Gerar tabela de códigos
  - Gravar cabeçalho (tamanho original + árvore serializada)
  - Reler arquivo original e gravar bits comprimidos
  - Fechar buffer (flush do último byte parcial com padding)

### Pessoa 2 — Implementar descompressão em `arquivo.c`
- [ ] Implementar buffer de leitura de bits (`ler_bit`)
- [ ] `descomprimir(const char *caminho_entrada, const char *caminho_saida)`:
  - Ler cabeçalho (tamanho original + desserializar árvore)
  - Percorrer bits do arquivo navegando na árvore
  - Escrever símbolo ao atingir folha
  - Parar quando número de bytes escritos igualar tamanho original
  - Verificar integridade: comparar tamanho do arquivo gerado com tamanho gravado no cabeçalho

**Ponto de validação antes de continuar:**
Comprimir um `.txt` simples e descomprimir — o arquivo recuperado deve ser idêntico ao original (`diff` ou comparação byte a byte).
garantir que compressão e descompressão funcionem com árvore de um único nó. O arquivo recuperado deve ser idêntico ao original para o caso de árvore de um único nó.
garantir que o fluxo de bits respeite o tamanho original do arquivo.
tratar padding do último byte com zeros e usar o tamanho original gravado no cabeçalho como limite na descompressão.

---

## ETAPA 4 — Interface (juntos)

### Ambos — Completar `main.c`

- [ ] Menu principal com as 6 opções
- [ ] Opção 1 — Compactar:
  - Submenu: listar arquivos da pasta fixa ou digitar caminho
  - Chamar `comprimir()`
  - Confirmar sucesso para o usuário
- [ ] Opção 2 — Descompactar:
  - Listar arquivos `.huff` do repositório
  - Chamar `descomprimir()`
  - Exibir resultado da verificação de integridade
  - Tratar nome do arquivo de saída (adicionar `_recuperado` se já existir)
- [ ] Opção 3 — Listar arquivos compactados:
  - Abrir pasta do repositório com `opendir`
  - Filtrar arquivos `.huff`
  - Exibir nome e tamanho de cada um
- [ ] Opção 4 — Remover arquivo compactado:
  - Listar `.huff` disponíveis
  - Pedir confirmação
  - Chamar `remove()`
- [ ] Opção 5 — Visualizar log
- [ ] Opção 6 — Sair
- [ ] Criar pasta `repositorio/` automaticamente se não existir (`mkdir`)

---

## ETAPA 5 — Finalização

### Pessoa 1 — Implementar `log.c` e revisar `arquivo.c`
- [ ] `registrar_operacao(const char *tipo, const char *entrada, const char *saida, const char *resultado)` — grava linha formatada com timestamp em `logs/operacoes.log`
- [ ] Integrar chamadas ao log em `comprimir()` e `descomprimir()`
- [ ] Revisar tratamento de erros em `arquivo.c` (arquivo não encontrado, sem permissão, disco cheio)

### Pessoa 2 — Testes e revisão geral
- [ ] Testar com arquivos `.txt` de tamanhos variados
- [ ] Testar com arquivo já comprimido (`.jpg` ou `.zip`) — verificar comportamento com taxa de compressão negativa
- [ ] Testar remoção e listagem com repositório vazio
- [ ] Verificar vazamentos de memória (valgrind ou checagem manual de `free`)
- [ ] Revisar `main.c` — entradas inválidas no menu não devem travar o programa

---

## Resumo por Pessoa

| Etapa | Pessoa 1 | Pessoa 2 |
|---|---|---|
| 1 | `heap.c` | Todos os headers + esqueleto `main.c` |
| 2 | `huffman.c` (juntos) | `huffman.c` (juntos) |
| 3 | Compressão em `arquivo.c` | Descompressão em `arquivo.c` |
| 4 | `main.c` (juntos) | `main.c` (juntos) |
| 5 | `log.c` + revisão `arquivo.c` | Testes + revisão geral |

---

## Pontos de Validação Obrigatórios

Não avançar para a próxima etapa sem validar:

- **Após Etapa 1:** heap insere e remove na ordem correta
- **Após Etapa 2:** árvore gerada e códigos fazem sentido para entrada simples
- **Após Etapa 3:** ciclo comprimir → descomprimir produz arquivo idêntico ao original
- **Após Etapa 4:** todas as opções do menu funcionam sem travar
- **Após Etapa 5:** testes com tipos de arquivo variados passam sem vazamento de memória
