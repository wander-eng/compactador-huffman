#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "huffman.h"
#include "heap.h"

// Identifica se o nó é uma folha da árvore.
static int eh_folha(No *no)
{
    return no != NULL && no->esquerda == NULL && no->direita == NULL;
}
// Percorre a árvore e monta os códigos binários de cada símbolo.
static void gerar_codigos_rec(No *raiz, char tabela[TAMANHO_ALFABETO][TAMANHO_MAX_CODIGO],char codigo[TAMANHO_MAX_CODIGO], int profundidade)
{
    if (raiz == NULL)
	{
        return;
    }

    if (eh_folha(raiz))
    {
        if (profundidade == 0)
        {
            /* Caso especial: árvore com um único nó.
               O código precisa ser não vazio para a compressão funcionar. */
            tabela[raiz->simbolo][0] = '0';
            tabela[raiz->simbolo][1] = '\0';
            return;
        }

        codigo[profundidade] = '\0';
        for (int i = 0; i <= profundidade; i++)
        {
            tabela[raiz->simbolo][i] = codigo[i];
        }
        return;
    }

    if (profundidade + 1 >= TAMANHO_MAX_CODIGO)
    {
        /* Proteção simples contra estouro da tabela de códigos. */
        return;
    }

    codigo[profundidade] = '0';
    gerar_codigos_rec(raiz->esquerda, tabela, codigo, profundidade + 1);

    codigo[profundidade] = '1';
    gerar_codigos_rec(raiz->direita, tabela, codigo, profundidade + 1);
}
// Reconstrói recursivamente a árvore a partir da serialização em pré-ordem.
static No *desserializar_rec(FILE *entrada)
{
    int marcador;

    if (entrada == NULL)
    {
        return NULL;
    }

    marcador = fgetc(entrada);
    if (marcador == EOF)
    {
        return NULL;
    }

    if (marcador == 1)
    {
        int simbolo = fgetc(entrada);
        if (simbolo == EOF)
        {
            return NULL;
        }
        return no_criar((unsigned char)simbolo, 0);
    }

    if (marcador == 0)
    {
        No *esquerda = desserializar_rec(entrada);
        No *direita = desserializar_rec(entrada);

        if (esquerda == NULL || direita == NULL)
        {
            destruir_arvore(esquerda);
            destruir_arvore(direita);
            return NULL;
        }

        return no_criar_interno(esquerda, direita);
    }

    return NULL;
}

No *no_criar(unsigned char simbolo, int frequencia)
{
    No *no = (No *)malloc(sizeof(No));
    if (no == NULL)
    {
        return NULL;
    }

    no->simbolo = simbolo;
    no->frequencia = frequencia;
    no->esquerda = NULL;
    no->direita = NULL;

    return no;
}

No *no_criar_interno(No *esquerda, No *direita)
{
    No *no = (No *)malloc(sizeof(No));
    if (no == NULL)
    {
        return NULL;
    }

    no->simbolo = 0;
    no->esquerda = esquerda;
    no->direita = direita;
    no->frequencia = 0;

    if (esquerda != NULL)
    {
        no->frequencia += esquerda->frequencia;
    }
    if (direita != NULL)
    {
        no->frequencia += direita->frequencia;
    }

    return no;
}

void contar_frequencias(FILE *arquivo, int frequencias[TAMANHO_ALFABETO])
{
    int c;

    if (frequencias == NULL)
    {
        return;
    }

    for (int i = 0; i < TAMANHO_ALFABETO; i++)
    {
        frequencias[i] = 0;
    }

    if (arquivo == NULL)
    {
        return;
    }

    rewind(arquivo);

    while ((c = fgetc(arquivo)) != EOF)
    {
        frequencias[(unsigned char)c]++;
    }

    rewind(arquivo);
}

No *construir_arvore(int frequencias[TAMANHO_ALFABETO])
{
    Heap *heap;
    No *esquerda;
    No *direita;
    No *pai;

    if (frequencias == NULL)
    {
        return NULL;
    }

    heap = heap_criar(HEAP_CAPACIDADE_INICIAL);
    if (heap == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < TAMANHO_ALFABETO; i++)
    {
        if (frequencias[i] > 0)
        {
            No *folha = no_criar((unsigned char)i, frequencias[i]);
            if (folha == NULL)
            {
                heap_destruir(heap);
                return NULL;
            }
            heap_inserir(heap, folha);
        }
    }

    if (heap_vazio(heap))
    {
        heap_destruir(heap);
        return NULL;
    }

    if (heap->tamanho == 1)
    {
        No *raiz = heap_remover_min(heap);
        heap_destruir(heap);
        return raiz;
    }

    while (heap->tamanho > 1)
    {
        esquerda = heap_remover_min(heap);
        direita = heap_remover_min(heap);

        pai = no_criar_interno(esquerda, direita);
        if (pai == NULL)
        {
            destruir_arvore(esquerda);
            destruir_arvore(direita);
            heap_destruir(heap);
            return NULL;
        }

        heap_inserir(heap, pai);
    }

    pai = heap_remover_min(heap);
    heap_destruir(heap);
    return pai;
}

void gerar_codigos(No *raiz, char tabela[TAMANHO_ALFABETO][TAMANHO_MAX_CODIGO])
{
    char codigo[TAMANHO_MAX_CODIGO];

    if (tabela == NULL)
    {
        return;
    }

    for (int i = 0; i < TAMANHO_ALFABETO; i++)
    {
        tabela[i][0] = '\0';
    }

    if (raiz == NULL)
    {
        return;
    }

    gerar_codigos_rec(raiz, tabela, codigo, 0);
}

void serializar_arvore(No *raiz, FILE *saida)
{
    if (raiz == NULL || saida == NULL)
    {
        return;
    }

    if (eh_folha(raiz))
    {
        fputc(1, saida);
        fputc(raiz->simbolo, saida);
        return;
    }

    fputc(0, saida);
    serializar_arvore(raiz->esquerda, saida);
    serializar_arvore(raiz->direita, saida);
}

No *desserializar_arvore(FILE *entrada)
{
    return desserializar_rec(entrada);
}

void destruir_arvore(No *raiz)
{
    if (raiz == NULL)
    {
        return;
    }

    destruir_arvore(raiz->esquerda);
    destruir_arvore(raiz->direita);
    free(raiz);
}
