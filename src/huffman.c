#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "huffman.h"
#include "heap.h"

static int eh_folha(No *no)
{
    return no != NULL && no->esquerda == NULL && no->direita == NULL;
}

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

/*Main temporária de testes!*/
int main(void)
{
    /* ============================================================
       TESTE 1 — no_criar() e no_criar_interno()
       Esperado:
       - folha com simbolo 'X', frequencia 7, sem filhos
       - no interno com frequencia 5 (2 + 3)
       ============================================================ */
    {
        No *folha = no_criar('X', 7);
        No *esquerda = no_criar('A', 2);
        No *direita = no_criar('B', 3);
        No *interno = no_criar_interno(esquerda, direita);

        if (folha == NULL || esquerda == NULL || direita == NULL || interno == NULL)
        {
            printf("[FAIL] Construtores de No retornaram NULL.\n");
            destruir_arvore(folha);
            destruir_arvore(interno);
            return 1;
        }

        if (folha->simbolo != 'X' ||
            folha->frequencia != 7 ||
            folha->esquerda != NULL ||
            folha->direita != NULL)
        {
            printf("[FAIL] no_criar nao preencheu corretamente a folha.\n");
            destruir_arvore(folha);
            destruir_arvore(interno);
            return 1;
        }

        if (interno->esquerda != esquerda ||
            interno->direita != direita ||
            interno->frequencia != 5)
        {
            printf("[FAIL] no_criar_interno nao ligou os filhos ou nao somou frequencias corretamente.\n");
            destruir_arvore(folha);
            destruir_arvore(interno);
            return 1;
        }

        printf("[PASS] no_criar e no_criar_interno.\n");
        destruir_arvore(folha);
        destruir_arvore(interno);
    }

    /* ============================================================
       TESTE 2 — arquivo vazio
       Esperado:
       - contar_frequencias zera o vetor
       - construir_arvore retorna NULL
       ============================================================ */
    {
        FILE *arquivo;
        int frequencias[TAMANHO_ALFABETO] = {0};
        No *raiz;

        arquivo = fopen("teste_vazio.txt", "wb");
        if (arquivo == NULL)
        {
            printf("[FAIL] Nao foi possivel criar teste_vazio.txt.\n");
            return 1;
        }
        fclose(arquivo);

        arquivo = fopen("teste_vazio.txt", "rb");
        if (arquivo == NULL)
        {
            printf("[FAIL] Nao foi possivel abrir teste_vazio.txt.\n");
            return 1;
        }

        contar_frequencias(arquivo, frequencias);
        fclose(arquivo);

        raiz = construir_arvore(frequencias);
        if (raiz != NULL)
        {
            printf("[FAIL] construir_arvore deveria retornar NULL para arquivo vazio.\n");
            destruir_arvore(raiz);
            return 1;
        }

        printf("[PASS] Arquivo vazio tratado corretamente.\n");
        remove("teste_vazio.txt");
    }

    /* ============================================================
       TESTE 3 — um único símbolo
       Esperado:
       - raiz com frequencia 6
       - codigo de 'a' = "0"
       - serializacao / desserializacao funcionando
       ============================================================ */
    {
        FILE *arquivo;
        FILE *saida1;
        FILE *entrada;
        FILE *saida2;
        int frequencias[TAMANHO_ALFABETO] = {0};
        No *raiz;
        No *raiz2;
        char tabela[TAMANHO_ALFABETO][TAMANHO_MAX_CODIGO];

        arquivo = fopen("teste_um_simbolo.txt", "wb");
        if (arquivo == NULL)
        {
            printf("[FAIL] Nao foi possivel criar teste_um_simbolo.txt.\n");
            return 1;
        }
        fputs("aaaaaa", arquivo);
        fclose(arquivo);

        arquivo = fopen("teste_um_simbolo.txt", "rb");
        if (arquivo == NULL)
        {
            printf("[FAIL] Nao foi possivel abrir teste_um_simbolo.txt.\n");
            return 1;
        }

        contar_frequencias(arquivo, frequencias);
        fclose(arquivo);

        raiz = construir_arvore(frequencias);
        if (raiz == NULL)
        {
            printf("[FAIL] construir_arvore retornou NULL para um unico simbolo.\n");
            remove("teste_um_simbolo.txt");
            return 1;
        }

        if (raiz->frequencia != 6)
        {
            printf("[FAIL] Frequencia da raiz esperada = 6, obtida = %d.\n", raiz->frequencia);
            destruir_arvore(raiz);
            remove("teste_um_simbolo.txt");
            return 1;
        }

        gerar_codigos(raiz, tabela);

        if (strcmp(tabela[(unsigned char)'a'], "0") != 0)
        {
            printf("[FAIL] Codigo esperado para 'a' em arvore de um no: \"0\". Obtido: \"%s\"\n",
                   tabela[(unsigned char)'a']);
            destruir_arvore(raiz);
            remove("teste_um_simbolo.txt");
            return 1;
        }

        saida1 = fopen("arvore_um_simbolo_1.bin", "wb");
        if (saida1 == NULL)
        {
            printf("[FAIL] Nao foi possivel criar arvore_um_simbolo_1.bin.\n");
            destruir_arvore(raiz);
            remove("teste_um_simbolo.txt");
            return 1;
        }

        serializar_arvore(raiz, saida1);
        fclose(saida1);

        entrada = fopen("arvore_um_simbolo_1.bin", "rb");
        if (entrada == NULL)
        {
            printf("[FAIL] Nao foi possivel reabrir arvore_um_simbolo_1.bin.\n");
            destruir_arvore(raiz);
            remove("teste_um_simbolo.txt");
            return 1;
        }

        raiz2 = desserializar_arvore(entrada);
        fclose(entrada);

        if (raiz2 == NULL)
        {
            printf("[FAIL] desserializar_arvore retornou NULL no caso de um unico simbolo.\n");
            destruir_arvore(raiz);
            remove("teste_um_simbolo.txt");
            return 1;
        }

        saida2 = fopen("arvore_um_simbolo_2.bin", "wb");
        if (saida2 == NULL)
        {
            printf("[FAIL] Nao foi possivel criar arvore_um_simbolo_2.bin.\n");
            destruir_arvore(raiz);
            destruir_arvore(raiz2);
            remove("teste_um_simbolo.txt");
            return 1;
        }

        serializar_arvore(raiz2, saida2);
        fclose(saida2);

        if (raiz2 == NULL)
        {
            printf("[FAIL] desserializacao falhou no caso de um unico simbolo.\n");
            destruir_arvore(raiz);
            remove("teste_um_simbolo.txt");
            return 1;
        }

        printf("[PASS] Um unico simbolo tratado corretamente.\n");

        destruir_arvore(raiz);
        destruir_arvore(raiz2);
        remove("teste_um_simbolo.txt");
        remove("arvore_um_simbolo_1.bin");
        remove("arvore_um_simbolo_2.bin");
    }

    /* ============================================================
       TESTE 4 — string "aabbbcccc"
       Esperado:
       - frequencias: a=2, b=3, c=4
       - raiz com frequencia 9
       - códigos coerentes
       - serialização / desserialização 
       ============================================================ */
    {
        FILE *arquivo;
        FILE *saida1;
        FILE *entrada;
        FILE *saida2;
        int frequencias[TAMANHO_ALFABETO] = {0};
        No *raiz;
        No *raiz2;
        char tabela[TAMANHO_ALFABETO][TAMANHO_MAX_CODIGO];

        arquivo = fopen("teste_aabbbcccc.txt", "wb");
        if (arquivo == NULL)
        {
            printf("[FAIL] Nao foi possivel criar teste_aabbbcccc.txt.\n");
            return 1;
        }
        fputs("aabbbcccc", arquivo);
        fclose(arquivo);

        arquivo = fopen("teste_aabbbcccc.txt", "rb");
        if (arquivo == NULL)
        {
            printf("[FAIL] Nao foi possivel abrir teste_aabbbcccc.txt.\n");
            return 1;
        }

        contar_frequencias(arquivo, frequencias);
        fclose(arquivo);

        if (frequencias[(unsigned char)'a'] != 2 ||
            frequencias[(unsigned char)'b'] != 3 ||
            frequencias[(unsigned char)'c'] != 4)
        {
            printf("[FAIL] Frequencias esperadas: a=2, b=3, c=4.\n");
            printf("       Obtido: a=%d, b=%d, c=%d\n",
                   frequencias[(unsigned char)'a'],
                   frequencias[(unsigned char)'b'],
                   frequencias[(unsigned char)'c']);
            remove("teste_aabbbcccc.txt");
            return 1;
        }

        raiz = construir_arvore(frequencias);
        if (raiz == NULL)
        {
            printf("[FAIL] construir_arvore retornou NULL para aabbbcccc.\n");
            remove("teste_aabbbcccc.txt");
            return 1;
        }

        if (raiz->frequencia != 9)
        {
            printf("[FAIL] Frequencia da raiz esperada = 9, obtida = %d.\n", raiz->frequencia);
            destruir_arvore(raiz);
            remove("teste_aabbbcccc.txt");
            return 1;
        }

        gerar_codigos(raiz, tabela);

        if (strcmp(tabela[(unsigned char)'a'], "10") != 0 ||
            strcmp(tabela[(unsigned char)'b'], "11") != 0 ||
            strcmp(tabela[(unsigned char)'c'], "0") != 0)
        {
            printf("[FAIL] Codigos inesperados para aabbbcccc.\n");
            printf("       a='%s' b='%s' c='%s'\n",
                   tabela[(unsigned char)'a'],
                   tabela[(unsigned char)'b'],
                   tabela[(unsigned char)'c']);
            destruir_arvore(raiz);
            remove("teste_aabbbcccc.txt");
            return 1;
        }

        saida1 = fopen("arvore_1.bin", "wb");
        if (saida1 == NULL)
        {
            printf("[FAIL] Nao foi possivel criar arvore_1.bin.\n");
            destruir_arvore(raiz);
            remove("teste_aabbbcccc.txt");
            return 1;
        }

        serializar_arvore(raiz, saida1);
        fclose(saida1);

        entrada = fopen("arvore_1.bin", "rb");
        if (entrada == NULL)
        {
            printf("[FAIL] Nao foi possivel reabrir arvore_1.bin.\n");
            destruir_arvore(raiz);
            remove("teste_aabbbcccc.txt");
            return 1;
        }

        raiz2 = desserializar_arvore(entrada);
        fclose(entrada);

        if (raiz2 == NULL)
        {
            printf("[FAIL] desserializar_arvore retornou NULL para aabbbcccc.\n");
            destruir_arvore(raiz);
            remove("teste_aabbbcccc.txt");
            return 1;
        }

        saida2 = fopen("arvore_2.bin", "wb");
        if (saida2 == NULL)
        {
            printf("[FAIL] Nao foi possivel criar arvore_2.bin.\n");
            destruir_arvore(raiz);
            destruir_arvore(raiz2);
            remove("teste_aabbbcccc.txt");
            return 1;
        }

        serializar_arvore(raiz2, saida2);
        fclose(saida2);

        printf("[PASS] aabbbcccc validado corretamente.\n");
        printf("       Raiz = %d, a=%s, b=%s, c=%s\n",
               raiz->frequencia,
               tabela[(unsigned char)'a'],
               tabela[(unsigned char)'b'],
               tabela[(unsigned char)'c']);

        destruir_arvore(raiz);
        destruir_arvore(raiz2);
        remove("teste_aabbbcccc.txt");
        remove("arvore_1.bin");
        remove("arvore_2.bin");
    }

    printf("\nRESULTADO FINAL: TODOS OS TESTES PASSARAM.\n");
    printf("\nPressione ENTER para sair...");
	getchar();
    return 0;
}

