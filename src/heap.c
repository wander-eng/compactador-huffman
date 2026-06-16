#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

//FUNÇÕES AUXILIARES

// Troca dois ponteiros para nós
static void trocar(No **a, No **b){
    No *temp = *a;
    *a = *b;
    *b = temp;
}

// Restaura a propriedade de min-heap subindo o elemento
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

// Restaura a propriedade de min-heap descendo o elemento
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

//CORPO DAS FUNÇÕES DECLARADA NO HEADER DO MÓDULO
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

void heap_destruir(Heap *h){
	if (h == NULL) {
		return; 
	}	
	free(h->dados);
	free(h);
}

int heap_vazio(Heap *h){
	return h == NULL || h->tamanho == 0;
}

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


int main(int argc, char *argv[]) {
    Heap *heap = heap_criar(10);

    No n1 = {'A', 10, NULL, NULL};
    No n2 = {'B', 3,  NULL, NULL};
    No n3 = {'C', 8,  NULL, NULL};
    No n4 = {'D', 1,  NULL, NULL};
    No n5 = {'E', 5,  NULL, NULL};

    heap_inserir(heap, &n1);
    heap_inserir(heap, &n2);
    heap_inserir(heap, &n3);
    heap_inserir(heap, &n4);
    heap_inserir(heap, &n5);

    int esperado[] = {1, 3, 5, 8, 10};

    for (int i = 0; i < 5; i++)
    {
        No *minimo = heap_remover_min(heap);

        if (minimo == NULL)
        {
            printf("FALHA: heap retornou NULL\n");
            heap_destruir(heap);
            return 1;
        }

        if (minimo->frequencia != esperado[i])
        {
            printf("FALHA: esperado %d, obtido %d\n",
                   esperado[i],
                   minimo->frequencia);

            heap_destruir(heap);
            return 1;
        }
    }

    printf("TESTE APROVADO\n");

    heap_destruir(heap);

    return 0;

}
