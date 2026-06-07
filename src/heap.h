#ifndef HEAP_H
#define HEAP_H

// Dependência: No é definido em huffman.h
// Incluir huffman.h antes de heap.h, ou usar forward declaration
#include "huffman.h"

#define HEAP_CAPACIDADE_INICIAL 256

typedef struct Heap {
    No **dados;        // vetor de ponteiros para nós
    int tamanho;     // quantidade atual de elementos
    int capacidade;  // capacidade máxima atual
} Heap;

// Cria um heap vazio com a capacidade informada
Heap *heap_criar(int capacidade);

// Libera a memória do heap (não libera os nós internamente)
void heap_destruir(Heap *h);

// Insere um nó no heap mantendo a propriedade de min-heap
void heap_inserir(Heap *h, No *no);

// Remove e retorna o nó de menor frequência
No *heap_remover_min(Heap *h);

// Retorna 1 se o heap estiver vazio, 0 caso contrário
int heap_vazio(Heap *h);

#endif
