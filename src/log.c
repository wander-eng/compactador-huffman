#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

#include "log.h"

#define PASTA_LOG "logs"
#define TAMANHO_LINHA_LOG 512

static int garantir_pasta_logs(void)
{
    struct stat info;

    if (stat(PASTA_LOG, &info) == 0)
    {
        return S_ISDIR(info.st_mode);
    }

    return MKDIR(PASTA_LOG) == 0;
}

static const char *tipo_para_string(TipoOperacao tipo)
{
    switch (tipo)
    {
        case LOG_COMPRESSAO:
            return "COMPRESSAO";
        case LOG_DESCOMPRESSAO:
            return "DESCOMPRESSAO";
        case LOG_REMOCAO:
            return "REMOCAO";
        case LOG_LISTAGEM:
            return "LISTAGEM";
        default:
            return "OPERACAO";
    }
}

void registrar_operacao(TipoOperacao tipo, const char *entrada, const char *saida, const char *resultado)
{
    FILE *arquivo;
    time_t agora;
    struct tm *tm_local;
    char data_hora[32];
    const char *tipo_texto;

    if (!garantir_pasta_logs())
    {
        return;
    }

    arquivo = fopen(CAMINHO_LOG, "a");
    if (arquivo == NULL)
    {
        return;
    }

    agora = time(NULL);
    tm_local = localtime(&agora);
    if (tm_local != NULL)
    {
        strftime(data_hora, sizeof(data_hora), "%Y-%m-%d %H:%M:%S", tm_local);
    }
    else
    {
        snprintf(data_hora, sizeof(data_hora), "data_indisponivel");
    }

    tipo_texto = tipo_para_string(tipo);

    fprintf(arquivo, "[%s] %s | entrada=%s | saida=%s | resultado=%s\n",
            data_hora,
            tipo_texto,
            (entrada != NULL ? entrada : "-"),
            (saida != NULL ? saida : "-"),
            (resultado != NULL ? resultado : "-"));

    fclose(arquivo);
}

void exibir_log(void)
{
    FILE *arquivo;
    char linha[TAMANHO_LINHA_LOG];

    arquivo = fopen(CAMINHO_LOG, "r");
    if (arquivo == NULL)
    {
        printf("Nenhum log encontrado.\n");
        return;
    }

    printf("\n=== LOG DE OPERACOES ===\n");

    while (fgets(linha, sizeof(linha), arquivo) != NULL)
    {
        printf("%s", linha);
    }

    fclose(arquivo);
}

