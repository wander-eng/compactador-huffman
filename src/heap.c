#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

//FUNÇÕES AUXILIARES

// Troca as posições de dois nós no vetor do heap.
static void trocar(No **a, No **b){
    No *temp = *a;
    *a = *b;
    *b = temp;
}

// Restaura a propriedade de min-heap subindo o elemento na árvore
static void heapify_cima(Heap *h, int i){
    while (i > 0)
    {
        int pai = (i - 1) / 2;

        if (h->dados[pai]->frequencia <= h->dados[i]->frequencia)
        {
            break;
        }

        trocar(&h->dados[pai], &h->dados[i]);

        i = pai;
    }
}

// Restaura a propriedade de min-heap descendo o elemento na árvore
static void heapify_baixo(Heap *h, int i){
    while (1)
    {
        int menor = i;

        int esquerda = 2 * i + 1;
        int direita = 2 * i + 2;

        if (esquerda < h->tamanho &&
            h->dados[esquerda]->frequencia <
            h->dados[menor]->frequencia)
        {
            menor = esquerda;
        }

        if (direita < h->tamanho &&
            h->dados[direita]->frequencia <
            h->dados[menor]->frequencia)
        {
            menor = direita;
        }

        if (menor == i)
        {
            break;
        }

        trocar(&h->dados[i], &h->dados[menor]);

        i = menor;
    }
}
// Cria e inicializa um heap vazio com a capacidade informada.
Heap *heap_criar(int capacidade){
	Heap *h = malloc(sizeof(Heap));
	if (h == NULL) {
		return NULL; 
	}
	h->dados = malloc(capacidade * sizeof(No *));
	if (h->dados == NULL) {
		free(h);
		return NULL;
	}
	h->tamanho = 0;
	h->capacidade = capacidade;
	return h;
}
// Libera a estrutura do heap sem liberar os nós armazenados nele.
void heap_destruir(Heap *h){
	if (h == NULL) {
		return; 
	}	
	free(h->dados);
	free(h);
}
// Verifica se o heap está vazio ou se o ponteiro recebido é nulo.
int heap_vazio(Heap *h){
	return h == NULL || h->tamanho == 0;
}
// Insere um nó no heap e ajusta a estrutura para manter a ordem mínima.
void heap_inserir(Heap *h, No *no)
{
    if (h == NULL || no == NULL)
        return;

    if (h->tamanho >= h->capacidade)
    	return;

    h->dados[h->tamanho] = no;
    h->tamanho++;

    heapify_cima(h, h->tamanho - 1);
}
// Remove e retorna o nó de menor frequência do heap.
No *heap_remover_min(Heap *h)
{
    No *minimo;

    if (h == NULL || heap_vazio(h)){
        return NULL;
    }

    minimo = h->dados[0];

    h->tamanho--;

    if (h->tamanho > 0){
        h->dados[0] = h->dados[h->tamanho];
        heapify_baixo(h, 0);
    }

    return minimo;
}
