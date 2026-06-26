# Compactador Huffman em C

Implementação do algoritmo de Huffman para compressão e descompressão de arquivos, desenvolvida em linguagem C como projeto da disciplina de **Algoritmos e Estruturas de Dados (AED)** — Engenharia de Computação, UNIVASF.

O projeto realiza compressão sem perdas (*lossless*), construindo uma árvore de Huffman a partir da frequência dos bytes do arquivo de entrada e gerando códigos binários de tamanho variável. Arquivos de maior frequência recebem códigos mais curtos, resultando em redução do tamanho total.

---

## Funcionalidades

- Compactação de qualquer tipo de arquivo com codificação de Huffman
- Descompactação de arquivos `.huff` com reconstrução da árvore a partir do cabeçalho
- Verificação de integridade do arquivo recuperado
- Listagem e remoção de arquivos compactados
- Registro de operações em arquivo de log

---

## Estruturas de dados utilizadas

| Estrutura | Papel no algoritmo |
|---|---|
| Vetor de 256 posições | Contagem de frequências dos bytes |
| Min-Heap (fila de prioridade) | Construção eficiente da árvore de Huffman |
| Árvore binária | Representação hierárquica dos códigos |
| Tabela de códigos (matriz 256×256) | Mapeamento byte → string de bits |

---

## Estrutura do projeto

```
compactador-huffman/
├── src/
│   ├── main.c        — interface de menu e fluxo principal
│   ├── huffman.c/h   — algoritmo: frequências, árvore, codificação
│   ├── heap.c/h      — min-heap com heapify
│   ├── arquivo.c/h   — compressão, descompressão, buffer de bits
│   └── log.c/h       — registro de operações
├── docs/
│   └── PROJETO.md    — documentação técnica completa
├── repositorio/       — pasta operacional do programa
├── logs/              — logs de execução
├── Makefile
└── LICENSE
```

---

## Compilação

Requer GCC e Make.

```bash
make
```

Para especificar o compilador manualmente:

```bash
make CC=caminho/para/gcc
```

Para limpar os arquivos gerados:

```bash
make clean
```

---

## Execução

```bash
make run
```

Ou diretamente após compilar:

```bash
./Menu.exe
```

O programa abrirá um menu interativo com as opções disponíveis. A pasta `repositorio/` é criada automaticamente na primeira execução.

---

## Documentação técnica

A documentação completa está em [`docs/PROJETO.md`](docs/PROJETO.md) e inclui:

- Algoritmo de Huffman — fases de compressão e descompressão
- Formato do arquivo `.huff` (cabeçalho e serialização da árvore)
- Decisões de implementação e casos de borda tratados
- Referências bibliográficas

---

## Observações

- Arquivos já comprimidos (`.jpg`, `.mp3`, `.zip`) tendem a crescer após compressão com Huffman — comportamento esperado, pois sua distribuição de bytes já é aproximadamente uniforme
- Arquivos vazios são rejeitados com mensagem de erro
- Arquivos com um único símbolo distinto são suportados

---

## Possíveis evoluções

- **Reorganização do repositório:** separar `repositorio/` em subpastas `original/`, `huff/` e `recuperado/` para melhor organização dos arquivos:
```
repositorio/
├── original/
├── huff/
└── recuperado/
```

---

## Licença

Este projeto está licenciado sob a **MIT License**. Consulte o arquivo [`LICENSE`](LICENSE) para mais informações.