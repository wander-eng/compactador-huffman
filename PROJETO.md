# Projeto AED — Compactador Huffman em C

## Visão Geral

Implementação de um compactador e descompactador de arquivos usando o algoritmo de Huffman, desenvolvido em C sem dependências externas além da biblioteca padrão. O programa opera sobre um repositório local de arquivos compactados e oferece uma interface de menu para interação com o usuário.

---

## Menu Principal

```
1. Compactar arquivo
2. Descompactar arquivo
3. Listar arquivos compactados
4. Remover arquivo compactado
5. Visualizar log de operações
6. Sair
```

### Submenu — Compactar arquivo

```
1. Selecionar arquivo da pasta do repositório (lista arquivos disponíveis)
2. Digitar caminho manualmente
```

---

## Funcionalidades

### 1. Compactar arquivo
- Aceita qualquer tipo de arquivo (operação byte a byte)
- O arquivo comprimido é salvo na pasta fixa do repositório com extensão `.huff`
- O arquivo original não é removido
- Grava no cabeçalho do `.huff`: tamanho original em bytes (para verificação de integridade), árvore de Huffman serializada
- Registra a operação no log

### 2. Descompactar arquivo
- Lista os arquivos `.huff` da pasta do repositório para seleção
- Reconstrói a árvore de Huffman a partir do cabeçalho
- Salva o arquivo recuperado na mesma pasta do arquivo `.huff`
- Se o arquivo de destino já existir, adiciona sufixo `_recuperado` automaticamente (ex: `relatorio_recuperado.txt`)
- Verifica integridade: compara o tamanho do arquivo recuperado com o tamanho original gravado no cabeçalho — exibe aviso em caso de divergência
- Registra a operação no log

### 3. Listar arquivos compactados
- Exibe os arquivos `.huff` presentes na pasta do repositório
- Mostra para cada arquivo: nome, tamanho em bytes

### 4. Remover arquivo compactado
- Lista os arquivos `.huff` para seleção
- Solicita confirmação antes de remover
- Registra a operação no log

### 5. Visualizar log de operações
- Exibe o conteúdo do arquivo de log na tela
- Formato de cada entrada:
  ```
  [YYYY-MM-DD HH:MM] | OPERACAO | arquivo_entrada → arquivo_saida | resultado
  ```

---

## Estruturas de Dados

### Nó da árvore de Huffman
```c
typedef struct No {
    unsigned char simbolo;
    int frequencia;
    struct No *esquerda;
    struct No *direita;
} No;
```

### Min-Heap (fila de prioridade)
- Usado na construção da árvore de Huffman
- Implementado como vetor com operações de heapify
- Cada elemento é um ponteiro para `No`

### Tabela de códigos
- Vetor de 256 posições indexado pelo valor do byte
- Cada posição armazena a string de bits correspondente (ex: `"101"`)

### Cabeçalho do arquivo `.huff`
```
[4 bytes] tamanho original do arquivo em bytes
[N bytes] árvore serializada (percurso pré-ordem)
[dados  ] bits comprimidos
```

---

## Algoritmo de Huffman — Fases

### Compressão
1. **Contagem de frequências**: lê o arquivo byte a byte, incrementa vetor de 256 posições
2. **Construção do heap**: insere todos os símbolos com frequência > 0 no min-heap
3. **Construção da árvore**: remove dois nós de menor frequência, cria nó pai com soma das frequências, reinsere — repete até restar um nó (raiz)
4. **Geração dos códigos**: percurso recursivo na árvore acumulando 0 (esquerda) e 1 (direita); ao atingir folha, armazena o código na tabela
5. **Escrita do arquivo**: grava cabeçalho com tamanho original e árvore serializada, relê o arquivo original e substitui cada byte pelo código binário correspondente

### Descompressão
1. **Leitura do cabeçalho**: extrai tamanho original e reconstrói a árvore serializada
2. **Decodificação**: percorre os bits do arquivo navegando na árvore — ao atingir folha, escreve o símbolo no arquivo de saída — repete até o número de bytes escritos igualar o tamanho original

### Serialização da árvore (pré-ordem)
- Nó folha: grava byte `1` seguido do símbolo (`1 byte`)
- Nó interno: grava byte `0`, serializa subárvore esquerda, serializa subárvore direita

---

## Estrutura de Arquivos do Projeto

```
huffman/
├── src/
│   ├── main.c          — menu e fluxo principal
│   ├── huffman.c       — algoritmo: contagem, árvore, codificação, decodificação
│   ├── huffman.h
│   ├── heap.c          — min-heap: insert, remove_min, heapify
│   ├── heap.h
│   ├── arquivo.c       — leitura/escrita de arquivos, cabeçalho, serialização
│   ├── arquivo.h
│   ├── log.c           — registro de operações
│   └── log.h
├── repositorio/        — pasta fixa onde ficam os .huff
├── logs/
│   └── operacoes.log
├── Makefile
└── PROJETO.md
```

---

## Fora do Escopo

- Compactação de múltiplos arquivos em um único `.huff`
- Interface gráfica
- Compatibilidade com formatos externos (gzip, zip, etc.)
- Compactação de diretórios
- Criptografia ou proteção por senha

---

## Observações de Implementação

- Arquivos já comprimidos (`.jpg`, `.mp3`, `.zip`) tendem a crescer após compressão com Huffman — comportamento esperado, não é bug
- O último byte dos dados comprimidos pode ser completado com zeros (padding); a descompressão deve encerrar quando atingir o tamanho original armazenado no cabeçalho
- Arquivo vazio deve ser rejeitado na compactação com mensagem de erro clara
- Arquivo com um único símbolo distinto deve ser suportado através de uma árvore composta por um único nó folha
- A pasta `repositorio/` deve ser criada automaticamente pelo programa se não existir

---

## Referências

- CORMEN, T. H. et al. *Algoritmos: teoria e prática*. 3. ed. Rio de Janeiro: Elsevier, 2012. (Cap. 16.3 — Códigos de Huffman)
- SEDGEWICK, Robert. *Algorithms in C: Parts 1-4*. 3. ed. Addison-Wesley, 1998.
