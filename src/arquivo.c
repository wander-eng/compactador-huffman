#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "arquivo.h"
#include "huffman.h"
#include "log.h"

static int eh_folha(No *no)
{
    return no != NULL && no->esquerda == NULL && no->direita == NULL;
}

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

/*TESTES TEMPORÁRIOS DA ETAPA 3:
int main(void)
{
    const char *normal_txt = "repositorio/teste_normal.txt";
    const char *normal_huff = "repositorio/teste_normal.huff";
    const char *normal_rec = "repositorio/teste_normal_recuperado.txt";

    const char *unico_txt = "repositorio/teste_unico.txt";
    const char *unico_huff = "repositorio/teste_unico.huff";
    const char *unico_rec = "repositorio/teste_unico_recuperado.txt";

    const char *vazio_txt = "repositorio/teste_vazio.txt";
    const char *vazio_huff = "repositorio/teste_vazio.huff";

    const char *inexistente_txt = "repositorio/nao_existe.txt";
    const char *saida_inexistente = "repositorio/saida_inexistente.huff";

    FILE *a;
    FILE *b;
    int ca;
    int cb;
    int retorno;
    int ok = 1;

    remove(normal_huff);
    remove(normal_rec);
    remove(unico_huff);
    remove(unico_rec);
    remove(vazio_huff);
    remove(saida_inexistente);

    printf("=== TESTES TEMPORARIOS DA ETAPA 3 ===\n\n");

    /* ============================================================
       TESTE 1 — arquivo simples
       Conteúdo: aabbbcccc
       Esperado:
       - comprimir(...) retorna 0
       - descomprimir(...) retorna 0
       - verificar_integridade(...) retorna 1
       - comparação byte a byte retorna 1
       ============================================================ 
    retorno = comprimir(normal_txt, normal_huff);
    printf("[TESTE 1] comprimir(%s) -> %d (esperado: 0)\n", normal_txt, retorno);
    if (retorno != 0)
        ok = 0;

    if (ok)
    {
        retorno = descomprimir(normal_huff, normal_rec);
        printf("[TESTE 1] descomprimir(%s) -> %d (esperado: 0)\n", normal_huff, retorno);
        if (retorno != 0)
            ok = 0;
    }

    if (ok)
    {
        retorno = verificar_integridade(normal_huff, normal_rec);
        printf("[TESTE 1] verificar_integridade(...) -> %d (esperado: 1)\n", retorno);
        if (retorno != 1)
            ok = 0;
    }

    if (ok)
    {
        a = fopen(normal_txt, "rb");
        b = fopen(normal_rec, "rb");

        if (a == NULL || b == NULL)
        {
            printf("[TESTE 1] Falha ao abrir arquivos para comparacao byte a byte.\n");
            ok = 0;
        }
        else
        {
            while (1)
            {
                ca = fgetc(a);
                cb = fgetc(b);

                if (ca != cb)
                {
                    ok = 0;
                    break;
                }

                if (ca == EOF)
                    break;
            }

            fclose(a);
            fclose(b);

            printf("[TESTE 1] comparacao byte a byte -> %d (esperado: 1)\n", ok ? 1 : 0);
        }
    }

    printf("[TESTE 1] %s\n\n", ok ? "PASS" : "FAIL");

    /* ============================================================
       TESTE 2 — árvore de um único nó
       Conteúdo: aaaaaaaaaa
       Esperado:
       - comprimir(...) retorna 0
       - descomprimir(...) retorna 0
       - verificar_integridade(...) retorna 1
       - comparação byte a byte retorna 1
       ============================================================ 
    if (ok)
    {
        retorno = comprimir(unico_txt, unico_huff);
        printf("[TESTE 2] comprimir(%s) -> %d (esperado: 0)\n", unico_txt, retorno);
        if (retorno != 0)
            ok = 0;
    }

    if (ok)
    {
        retorno = descomprimir(unico_huff, unico_rec);
        printf("[TESTE 2] descomprimir(%s) -> %d (esperado: 0)\n", unico_huff, retorno);
        if (retorno != 0)
            ok = 0;
    }

    if (ok)
    {
        retorno = verificar_integridade(unico_huff, unico_rec);
        printf("[TESTE 2] verificar_integridade(...) -> %d (esperado: 1)\n", retorno);
        if (retorno != 1)
            ok = 0;
    }

    if (ok)
    {
        a = fopen(unico_txt, "rb");
        b = fopen(unico_rec, "rb");

        if (a == NULL || b == NULL)
        {
            printf("[TESTE 2] Falha ao abrir arquivos para comparacao byte a byte.\n");
            ok = 0;
        }
        else
        {
            while (1)
            {
                ca = fgetc(a);
                cb = fgetc(b);

                if (ca != cb)
                {
                    ok = 0;
                    break;
                }

                if (ca == EOF)
                    break;
            }

            fclose(a);
            fclose(b);

            printf("[TESTE 2] comparacao byte a byte -> %d (esperado: 1)\n", ok ? 1 : 0);
        }
    }

    printf("[TESTE 2] %s\n\n", ok ? "PASS" : "FAIL");

    /* ============================================================
       TESTE 3 — arquivo vazio
       Conteúdo: vazio
       Esperado:
       - comprimir(...) retorna -1
       ============================================================ 
    if (ok)
    {
        retorno = comprimir(vazio_txt, vazio_huff);
        printf("[TESTE 3] comprimir(%s) -> %d (esperado: -1)\n", vazio_txt, retorno);
        if (retorno != -1)
            ok = 0;

        printf("[TESTE 3] %s\n\n", (retorno == -1) ? "PASS" : "FAIL");
    }

    /* ============================================================
       TESTE 4 — caminho inexistente
       Esperado:
       - comprimir(...) retorna -1
       - descomprimir(...) retorna -1
       ============================================================ 
    if (ok)
    {
        retorno = comprimir(inexistente_txt, saida_inexistente);
        printf("[TESTE 4] comprimir(%s) -> %d (esperado: -1)\n", inexistente_txt, retorno);
        if (retorno != -1)
            ok = 0;

        retorno = descomprimir(inexistente_txt, saida_inexistente);
        printf("[TESTE 4] descomprimir(%s) -> %d (esperado: -1)\n", inexistente_txt, retorno);
        if (retorno != -1)
            ok = 0;

        printf("[TESTE 4] %s\n\n", (retorno == -1) ? "PASS" : "FAIL");
    }

    remove(normal_huff);
    remove(normal_rec);
    remove(unico_huff);
    remove(unico_rec);
    remove(vazio_huff);
    remove(saida_inexistente);

    printf("====================================\n");
    if (ok)
    {
        printf("RESULTADO FINAL: TODOS OS TESTES PASSARAM.\n");
        printf("RETORNO ESPERADO SE TUDO ESTIVER CERTO: 0\n");
        printf("\nPressione ENTER para sair...");
		getchar();
        return 0;
    }

    printf("RESULTADO FINAL: HOUVE FALHA EM PELO MENOS UM TESTE.\n");
    printf("RETORNO ESPERADO SE ALGUM TESTE FALHAR: 1\n");
    printf("\nPressione ENTER para sair...");
	getchar();
    return 1;
}*/
