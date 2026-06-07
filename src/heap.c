#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

Heap *heap_criar(int capacidade){
	Heap *h = malloc(sizeof(Heap));
	h->dados = malloc(capacidade * sizeof(No *));
	h->tamanho = 0;
	h->capacidade = capacidade;
	return h;
}

int main(int argc, char *argv[]) {
	
	return 0;
}
