#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "arquivo.h"
#include "huffman.h"
#include "log.h"

// Identifica se o nó é uma folha da árvore de Huffman.
static int eh_folha(No *no)
{
    return no != NULL && no->esquerda == NULL && no->direita == NULL;
}
// Escreve um inteiro de 32 bits no arquivo em ordem de bytes fixa.
static int escrever_uint32(FILE *arquivo, uint32_t valor)
{
    if (arquivo == NULL)
    {
        return 0;
    }

    if (fputc((int)((valor >> 24) & 0xFF), arquivo) == EOF)
    {
        return 0;
    }
    if (fputc((int)((valor >> 16) & 0xFF), arquivo) == EOF)
    {
        return 0;
    }
    if (fputc((int)((valor >> 8) & 0xFF), arquivo) == EOF)
    {
        return 0;
    }
    if (fputc((int)(valor & 0xFF), arquivo) == EOF)
    {
        return 0;
    }

    return 1;
}
// Lê um inteiro de 32 bits do arquivo em ordem de bytes fixa.
static int ler_uint32(FILE *arquivo, uint32_t *valor)
{
    int b1;
    int b2;
    int b3;
    int b4;

    if (arquivo == NULL || valor == NULL)
    {
        return 0;
    }

    b1 = fgetc(arquivo);
    b2 = fgetc(arquivo);
    b3 = fgetc(arquivo);
    b4 = fgetc(arquivo);

    if (b1 == EOF || b2 == EOF || b3 == EOF || b4 == EOF)
    {
        return 0;
    }

    *valor = ((uint32_t)b1 << 24) |
             ((uint32_t)b2 << 16) |
             ((uint32_t)b3 << 8) |
             (uint32_t)b4;

    return 1;
}
// Obtém o tamanho total de um arquivo em bytes.
static int obter_tamanho_arquivo(const char *caminho, uint32_t *tamanho)
{
    FILE *arquivo;
    long posicao;

    if (caminho == NULL || tamanho == NULL)
    {
        return 0;
    }

    arquivo = fopen(caminho, "rb");
    if (arquivo == NULL)
    {
        return 0;
    }

    if (fseek(arquivo, 0, SEEK_END) != 0)
    {
        fclose(arquivo);
        return 0;
    }

    posicao = ftell(arquivo);
    fclose(arquivo);

    if (posicao < 0)
    {
        return 0;
    }

    *tamanho = (uint32_t)posicao;
    return 1;
}
// Acumula bits em um byte e grava no arquivo quando o buffer completa.
static void escrever_bit(FILE *arquivo, int bit, unsigned char *buffer, int *contador)
{
    if (arquivo == NULL || buffer == NULL || contador == NULL)
    {
        return;
    }

    bit = bit ? 1 : 0;

    *buffer = (unsigned char)((*buffer << 1) | bit);
    (*contador)++;

    if (*contador == 8)
    {
        fputc(*buffer, arquivo);
        *buffer = 0;
        *contador = 0;
    }
}
// Escreve no arquivo o último byte parcial do buffer, completando com zeros.
static void flush_buffer(FILE *arquivo, unsigned char *buffer, int *contador)
{
    if (arquivo == NULL || buffer == NULL || contador == NULL)
    {
        return;
    }

    if (*contador > 0)
    {
        *buffer = (unsigned char)(*buffer << (8 - *contador));
        fputc(*buffer, arquivo);
        *buffer = 0;
        *contador = 0;
    }
}
// Lê o próximo bit do arquivo usando um buffer de bytes.
static int ler_bit(FILE *arquivo, unsigned char *buffer, int *contador)
{
    int byte_lido;
    int bit;

    if (arquivo == NULL || buffer == NULL || contador == NULL)
    {
        return -1;
    }

    if (*contador == 0)
    {
        byte_lido = fgetc(arquivo);
        if (byte_lido == EOF)
        {
            return -1;
        }

        *buffer = (unsigned char)byte_lido;
        *contador = 8;
    }

    bit = (*buffer & 0x80) ? 1 : 0;
    *buffer = (unsigned char)(*buffer << 1);
    (*contador)--;

    return bit;
}

int comprimir(const char *caminho_entrada, const char *caminho_saida)
{
    FILE *entrada = NULL;
    FILE *saida = NULL;
    int frequencias[TAMANHO_ALFABETO];
    char tabela[TAMANHO_ALFABETO][TAMANHO_MAX_CODIGO];
    No *raiz = NULL;
    unsigned char buffer;
    int contador;
    int c;
    int i;
    uint32_t tamanho_original;
    long tamanho_compactado;
    char *codigo;
    const char *resultado_log = "OK";
    int status = -1;

    if (caminho_entrada == NULL || caminho_saida == NULL)
    {
        resultado_log = "ERRO: parametros invalidos";
        goto cleanup;
    }

    entrada = fopen(caminho_entrada, "rb");
    if (entrada == NULL)
    {
        resultado_log = "ERRO: nao foi possivel abrir arquivo de entrada";
        goto cleanup;
    }

    contar_frequencias(entrada, frequencias);

    tamanho_original = 0;
    for (i = 0; i < TAMANHO_ALFABETO; i++)
    {
        tamanho_original += (uint32_t)frequencias[i];
    }

    if (tamanho_original == 0)
    {
        resultado_log = "ERRO: arquivo vazio";
        goto cleanup;
    }

    raiz = construir_arvore(frequencias);
    if (raiz == NULL)
    {
        resultado_log = "ERRO: falha ao construir arvore";
        goto cleanup;
    }

    gerar_codigos(raiz, tabela);

    saida = fopen(caminho_saida, "wb");
    if (saida == NULL)
    {
        resultado_log = "ERRO: nao foi possivel criar arquivo de saida";
        goto cleanup;
    }

    if (!escrever_uint32(saida, tamanho_original))
    {
        resultado_log = "ERRO: falha ao escrever tamanho original";
        goto cleanup;
    }

    serializar_arvore(raiz, saida);
    if (ferror(saida))
    {
        resultado_log = "ERRO: falha ao serializar arvore";
        goto cleanup;
    }

    rewind(entrada);
    buffer = 0;
    contador = 0;

    while ((c = fgetc(entrada)) != EOF)
    {
        codigo = tabela[(unsigned char)c];
        if (codigo[0] == '\0')
        {
            resultado_log = "ERRO: codigo nao encontrado na tabela";
            goto cleanup;
        }

        for (i = 0; codigo[i] != '\0'; i++)
        {
            if (codigo[i] == '0')
            {
                escrever_bit(saida, 0, &buffer, &contador);
            }
            else if (codigo[i] == '1')
            {
                escrever_bit(saida, 1, &buffer, &contador);
            }
            else
            {
                resultado_log = "ERRO: codigo invalido";
                goto cleanup;
            }
        }
    }

    flush_buffer(saida, &buffer, &contador);
    if (ferror(saida))
    {
        resultado_log = "ERRO: falha ao gravar bits comprimidos";
        goto cleanup;
    }

    tamanho_compactado = ftell(saida);
    if (tamanho_compactado >= 0 && (uint32_t)tamanho_compactado >= tamanho_original)
    {
        printf("[AVISO] Nao houve ganho de compressao para este arquivo.\n");
    }

    status = 0;
    resultado_log = "OK";
	//rótulo para limpeza geral
	cleanup:
    if (saida != NULL)
    {
        fclose(saida);
    }

    if (entrada != NULL)
    {
        fclose(entrada);
    }

    if (raiz != NULL)
    {
        destruir_arvore(raiz);
    }

    registrar_operacao(LOG_COMPRESSAO, caminho_entrada, caminho_saida, resultado_log);
    return status;
}

int descomprimir(const char *caminho_entrada, const char *caminho_saida)
{
    FILE *entrada = NULL;
    FILE *saida = NULL;
    uint32_t tamanho_original;
    uint32_t bytes_escritos;
    No *raiz = NULL;
    No *atual;
    unsigned char buffer;
    int contador;
    int bit;
    int status_integridade;
    const char *resultado_log = "OK";
    int status = -1;

    if (caminho_entrada == NULL || caminho_saida == NULL)
    {
        resultado_log = "ERRO: parametros invalidos";
        goto cleanup;
    }

    entrada = fopen(caminho_entrada, "rb");
    if (entrada == NULL)
    {
        resultado_log = "ERRO: nao foi possivel abrir arquivo de entrada";
        goto cleanup;
    }

    if (!ler_uint32(entrada, &tamanho_original))
    {
        resultado_log = "ERRO: falha ao ler tamanho original";
        goto cleanup;
    }

    raiz = desserializar_arvore(entrada);
    if (raiz == NULL)
    {
        resultado_log = "ERRO: falha ao desserializar arvore";
        goto cleanup;
    }

    saida = fopen(caminho_saida, "wb");
    if (saida == NULL)
    {
        resultado_log = "ERRO: nao foi possivel criar arquivo de saida";
        goto cleanup;
    }

    bytes_escritos = 0;

    if (eh_folha(raiz))
    {
        while (bytes_escritos < tamanho_original)
        {
            if (fputc(raiz->simbolo, saida) == EOF)
            {
                resultado_log = "ERRO: falha ao escrever arquivo recuperado";
                goto cleanup;
            }
            bytes_escritos++;
        }
    }
    else
    {
        atual = raiz;
        buffer = 0;
        contador = 0;

        while (bytes_escritos < tamanho_original)
        {
            bit = ler_bit(entrada, &buffer, &contador);
            if (bit < 0)
            {
                resultado_log = "ERRO: falha ao ler bits comprimidos";
                goto cleanup;
            }

            if (bit == 0)
            {
                atual = atual->esquerda;
            }
            else
            {
                atual = atual->direita;
            }

            if (atual == NULL)
            {
                resultado_log = "ERRO: percurso invalido na arvore";
                goto cleanup;
            }

            if (eh_folha(atual))
            {
                if (fputc(atual->simbolo, saida) == EOF)
                {
                    resultado_log = "ERRO: falha ao escrever arquivo recuperado";
                    goto cleanup;
                }

                bytes_escritos++;
                atual = raiz;
            }
        }
    }

    if (ferror(saida))
    {
        resultado_log = "ERRO: falha na escrita do arquivo recuperado";
        goto cleanup;
    }

	if (fflush(saida) != 0)
	{
	    resultado_log = "ERRO: falha ao finalizar escrita";
	    goto cleanup;
	}

    status_integridade = verificar_integridade(caminho_entrada, caminho_saida);
    if (status_integridade != 1)
    {
        resultado_log = "ERRO: integridade divergente";
        goto cleanup;
    }

    status = 0;
    resultado_log = "OK";

	cleanup:
    if (saida != NULL)
    {
        fclose(saida);
    }

    if (entrada != NULL)
    {
        fclose(entrada);
    }

    if (raiz != NULL)
    {
        destruir_arvore(raiz);
    }

    registrar_operacao(LOG_DESCOMPRESSAO, caminho_entrada, caminho_saida, resultado_log);
    return status;
}

int verificar_integridade(const char *caminho_huff, const char *caminho_recuperado)
{
    FILE *arquivo_huff;
    uint32_t tamanho_original;
    uint32_t tamanho_recuperado;

    if (caminho_huff == NULL || caminho_recuperado == NULL)
    {
        return 0;
    }

    arquivo_huff = fopen(caminho_huff, "rb");
    if (arquivo_huff == NULL)
    {
        return 0;
    }

    if (!ler_uint32(arquivo_huff, &tamanho_original))
    {
        fclose(arquivo_huff);
        return 0;
    }

    fclose(arquivo_huff);

    if (!obter_tamanho_arquivo(caminho_recuperado, &tamanho_recuperado))
    {
        return 0;
    }

    return tamanho_original == tamanho_recuperado;
}
