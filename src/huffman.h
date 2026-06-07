#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>

#define TAMANHO_ALFABETO 256
#define TAMANHO_MAX_CODIGO 256  // tamanho máximo de um código binário em caracteres

// Nó da árvore de Huffman
typedef struct No {
    unsigned char simbolo;   // byte representado (válido apenas em folhas)
    int frequencia;          // frequência do símbolo ou soma dos filhos
    struct No *esquerda;
    struct No *direita;
} No;

// Conta a frequência de cada byte no arquivo
// Preenche o vetor frequencias[256] com as contagens
void contar_frequencias(FILE *arquivo, int frequencias[TAMANHO_ALFABETO]);

// Constrói a árvore de Huffman a partir do vetor de frequências
// Retorna o ponteiro para a raiz da árvore
No *construir_arvore(int frequencias[TAMANHO_ALFABETO]);

// Percorre a árvore recursivamente e preenche a tabela de códigos
// tabela[i] será a string de bits do símbolo i (ex: "101")
void gerar_codigos(No *raiz, char tabela[TAMANHO_ALFABETO][TAMANHO_MAX_CODIGO]);

// Serializa a árvore em pré-ordem no arquivo de saída
// Formato: '0' para nó interno, '1' + simbolo para folha
void serializar_arvore(No *raiz, FILE *saida);

// Reconstrói a árvore a partir da serialização no arquivo
// Retorna o ponteiro para a raiz reconstruída
No *desserializar_arvore(FILE *entrada);

// Libera toda a memória da árvore recursivamente
void destruir_arvore(No *raiz);

// Cria e retorna um novo nó folha
No *no_criar(unsigned char simbolo, int frequencia);

// Cria e retorna um nó interno com dois filhos
No *no_criar_interno(No *esquerda, No *direita);

#endif
