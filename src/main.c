#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

#include "arquivo.h"
#include "log.h"

#define REPOSITORIO_DIR "repositorio"
#define MAX_LINHA 512
#define MAX_CAMINHO 512
#define MAX_NOME_ARQUIVO 260
#define MAX_ARQUIVOS 256

//==Utilidades gerais de interface==
// Remove a quebra de linha final de uma string lida do teclado.
static void remover_quebra_linha(char *texto)
{
    size_t len;

    if (texto == NULL)
    {
        return;
    }

    len = strlen(texto);
    if (len > 0 && texto[len - 1] == '\n')
    {
        texto[len - 1] = '\0';
    }
}
// Lê uma linha da entrada padrão com uma mensagem de prompt.
static int ler_linha(const char *prompt, char *buffer, size_t tamanho)
{
    if (prompt != NULL)
    {
        printf("%s", prompt);
    }

    if (fgets(buffer, (int)tamanho, stdin) == NULL)
    {
        return 0;
    }

    remover_quebra_linha(buffer);
    return 1;
}
// Lê uma opção inteira informada pelo usuário.
static int ler_opcao(const char *prompt, int *valor)
{
    char linha[MAX_LINHA];
    char *fim;
    long numero;

    if (valor == NULL)
    {
        return 0;
    }

    if (!ler_linha(prompt, linha, sizeof(linha)))
    {
        return 0;
    }

    numero = strtol(linha, &fim, 10);
    while (*fim != '\0' && isspace((unsigned char)*fim))
    {
        fim++;
    }

    if (fim == linha || *fim != '\0')
    {
        *valor = -1;
        return 1;
    }

    *valor = (int)numero;
    return 1;
}
// Limpa a tela do terminal e reposiciona o cursor no topo.
static void limpar_tela(void)
{
    printf("\x1b[2J\x1b[H");
    fflush(stdout);
}
// Aguarda o usuário pressionar ENTER antes de continuar.
static void pausar(void)
{
    printf("\nPressione ENTER para continuar...");
    getchar();
}
// Verifica se uma string termina com o sufixo informado.
static int termina_com(const char *texto, const char *sufixo)
{
    size_t len_texto;
    size_t len_sufixo;

    if (texto == NULL || sufixo == NULL)
    {
        return 0;
    }

    len_texto = strlen(texto);
    len_sufixo = strlen(sufixo);

    if (len_sufixo > len_texto)
    {
        return 0;
    }

    return strcmp(texto + (len_texto - len_sufixo), sufixo) == 0;
}
// Verifica se um caminho aponta para um arquivo existente.
static int arquivo_existe(const char *caminho)
{
    struct stat info;
    return caminho != NULL && stat(caminho, &info) == 0;
}
// Garante que a pasta do repositório exista antes do uso.
static int garantir_repositorio(void)
{
    struct stat info;

    if (stat(REPOSITORIO_DIR, &info) == 0)
    {
        return S_ISDIR(info.st_mode);
    }

    return MKDIR(REPOSITORIO_DIR) == 0;
}
// Remove aspas do início e do fim de um caminho copiado.
static void remover_aspas(char *texto)
{
    size_t len;

    if (texto == NULL)
    {
        return;
    }

    len = strlen(texto);

    if (len >= 2 &&
        texto[0] == '"' &&
        texto[len - 1] == '"')
    {
        memmove(texto, texto + 1, len - 2);
        texto[len - 2] = '\0';
    }
}
// Extrai apenas o nome final de um caminho completo.
static void obter_basename(const char *caminho, char *saida, size_t tamanho_saida)
{
    const char *ultimo_slash;
    const char *ultimo_backslash;
    const char *base;

    if (saida == NULL || tamanho_saida == 0)
    {
        return;
    }

    saida[0] = '\0';
    if (caminho == NULL)
    {
        return;
    }

    ultimo_slash = strrchr(caminho, '/');
    ultimo_backslash = strrchr(caminho, '\\');

    base = caminho;
    if (ultimo_slash != NULL && ultimo_slash > base)
    {
        base = ultimo_slash + 1;
    }
    if (ultimo_backslash != NULL && ultimo_backslash > base)
    {
        base = ultimo_backslash + 1;
    }

    snprintf(saida, tamanho_saida, "%s", base);
}
// Remove a extensão .huff do nome de um arquivo compactado.
static void obter_base_sem_huff(const char *nome_huff, char *saida, size_t tamanho_saida)
{
    char base[MAX_NOME_ARQUIVO];
    size_t len;

    if (saida == NULL || tamanho_saida == 0)
    {
        return;
    }

    saida[0] = '\0';
    obter_basename(nome_huff, base, sizeof(base));

    len = strlen(base);
    if (len >= 5 && termina_com(base, ".huff"))
    {
        base[len - 5] = '\0';
    }

    snprintf(saida, tamanho_saida, "%s", base);
}
// Monta o caminho de saída do arquivo compactado no repositório.
static void montar_saida_compactado(const char *entrada, char *saida, size_t tamanho_saida)
{
    char nome_base[MAX_NOME_ARQUIVO];

    if (saida == NULL || tamanho_saida == 0)
    {
        return;
    }

    saida[0] = '\0';
    obter_basename(entrada, nome_base, sizeof(nome_base));
    snprintf(saida, tamanho_saida, "%s/%s.huff", REPOSITORIO_DIR, nome_base);
}
// Monta o caminho de saída do arquivo recuperado.
static void montar_saida_descompactado(const char *entrada_huff, char *saida, size_t tamanho_saida)
{
    char nome_arquivo[MAX_NOME_ARQUIVO];
    char nome_sem_huff[MAX_NOME_ARQUIVO];
    char raiz[MAX_NOME_ARQUIVO];
    char extensao[MAX_NOME_ARQUIVO];
    char candidato[MAX_CAMINHO];
    char candidato_final[MAX_CAMINHO];
    char *ponto;
    int contador;

    if (saida == NULL || tamanho_saida == 0)
    {
        return;
    }

    saida[0] = '\0';

    if (entrada_huff == NULL)
    {
        return;
    }

    obter_basename(entrada_huff, nome_arquivo, sizeof(nome_arquivo));
    snprintf(nome_sem_huff, sizeof(nome_sem_huff), "%s", nome_arquivo);

    if (!termina_com(nome_sem_huff, ".huff"))
    {
        snprintf(candidato, sizeof(candidato), "%s/%s_recuperado", REPOSITORIO_DIR, nome_sem_huff);
        snprintf(saida, tamanho_saida, "%s", candidato);
        return;
    }

    nome_sem_huff[strlen(nome_sem_huff) - 5] = '\0'; /* remove .huff */

    ponto = strrchr(nome_sem_huff, '.');
    if (ponto != NULL)
    {
        snprintf(extensao, sizeof(extensao), "%s", ponto + 1);
        *ponto = '\0';
        snprintf(raiz, sizeof(raiz), "%s", nome_sem_huff);
    }
    else
    {
        extensao[0] = '\0';
        snprintf(raiz, sizeof(raiz), "%s", nome_sem_huff);
    }

    if (extensao[0] != '\0')
    {
        snprintf(candidato, sizeof(candidato), "%s/%s_recuperado.%s", REPOSITORIO_DIR, raiz, extensao);
    }
    else
    {
        snprintf(candidato, sizeof(candidato), "%s/%s_recuperado", REPOSITORIO_DIR, raiz);
    }

    // Evita sobrescrever um arquivo já existente
    if (arquivo_existe(candidato))
    {
        contador = 1;
        do
        {
            if (extensao[0] != '\0')
            {
                snprintf(candidato_final, sizeof(candidato_final),
                         "%s/%s_recuperado_%d.%s", REPOSITORIO_DIR, raiz, contador, extensao);
            }
            else
            {
                snprintf(candidato_final, sizeof(candidato_final),
                         "%s/%s_recuperado_%d", REPOSITORIO_DIR, raiz, contador);
            }

            contador++;
        } while (arquivo_existe(candidato_final));

        snprintf(saida, tamanho_saida, "%s", candidato_final);
        return;
    }

    snprintf(saida, tamanho_saida, "%s", candidato);
}

// Lista os arquivos do repositório de acordo com o modo informado.
// modo: 0 para arquivos comuns, 1 para arquivos .huff.
static int listar_arquivos_repositorio(int modo, char nomes[][MAX_NOME_ARQUIVO], long tamanhos[], int max_arquivos)
{
    DIR *dir;
    struct dirent *ent;
    struct stat info;
    char caminho[MAX_CAMINHO];
    int quantidade;

    dir = opendir(REPOSITORIO_DIR);
    if (dir == NULL)
    {
        printf("Nao foi possivel abrir a pasta '%s'.\n", REPOSITORIO_DIR);
        return -1;
    }

    quantidade = 0;

    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }
        
        if (strcmp(ent->d_name, ".gitkeep") == 0)
		{
		    continue;
		}

        if (modo == 1)
        {
            if (!termina_com(ent->d_name, ".huff"))
            {
                continue;
            }
        }
        else
        {
            if (termina_com(ent->d_name, ".huff"))
            {
                continue;
            }
        }

        snprintf(caminho, sizeof(caminho), "%s/%s", REPOSITORIO_DIR, ent->d_name);
        if (stat(caminho, &info) != 0 || !S_ISREG(info.st_mode))
        {
            continue;
        }

        if (quantidade < max_arquivos)
        {
            if (nomes != NULL)
            {
                snprintf(nomes[quantidade], MAX_NOME_ARQUIVO, "%s", ent->d_name);
            }
            if (tamanhos != NULL)
            {
                tamanhos[quantidade] = (long)info.st_size;
            }
        }

        printf("%d. %s (%ld bytes)\n", quantidade + 1, ent->d_name, (long)info.st_size);
        quantidade++;
    }

    closedir(dir);

    if (quantidade == 0)
    {
        printf("Nenhum arquivo encontrado em '%s'.\n", REPOSITORIO_DIR);
    }

    return quantidade;
}
// Permite selecionar um arquivo listado no repositório.
// modo: 0 para arquivos comuns, 1 para arquivos .huff.
static int selecionar_arquivo_repositorio(int modo, char *selecionado, size_t tamanho_selecionado)
{
    char nomes[MAX_ARQUIVOS][MAX_NOME_ARQUIVO];
    long tamanhos[MAX_ARQUIVOS];
    int quantidade;
    int escolha;

    quantidade = listar_arquivos_repositorio(modo, nomes, tamanhos, MAX_ARQUIVOS);
    if (quantidade <= 0)
    {
        return 0;
    }

    if (!ler_opcao("Escolha um arquivo (0 para cancelar): ", &escolha))
    {
        return 0;
    }

    if (escolha == 0)
    {
        return 0;
    }

    if (escolha < 1 || escolha > quantidade)
    {
        printf("Opcao invalida.\n");
        return 0;
    }

    snprintf(selecionado, tamanho_selecionado, "%s/%s", REPOSITORIO_DIR, nomes[escolha - 1]);
    return 1;
}

//==Ações do menu==
// Exibe o submenu de compactação e encaminha a operação escolhida.
static void menu_compactar(void)
{	
    int subopcao;
    char origem[MAX_CAMINHO];
    char destino[MAX_CAMINHO];
    char caminho_manual[MAX_CAMINHO];
	
	limpar_tela();
    printf("\n=== COMPACTAR ARQUIVO ===\n");
    printf("1. Selecionar arquivo da pasta do repositorio\n");
    printf("2. Digitar caminho manualmente\n");
    printf("0. Retornar ao menu principal\n");
    
    if (!ler_opcao("Opcao: ", &subopcao))
    {	pausar();
        return;
    }

    if (subopcao == 0)
    {
        return;
    }
    else if (subopcao == 1)
    {
        if (!selecionar_arquivo_repositorio(0, origem, sizeof(origem)))
        {	pausar();
            return;
        }
    }
    else if (subopcao == 2)
    {
        if (!ler_linha("Digite o caminho do arquivo de entrada: ",
                       caminho_manual,
                       sizeof(caminho_manual)))
        {	pausar();
            return;
        }
		remover_aspas(caminho_manual);
        if (caminho_manual[0] == '\0')
        {
            printf("Caminho invalido.\n");
            pausar();
            return;
        }

        snprintf(origem, sizeof(origem), "%s", caminho_manual);
    }
    else
    {
        printf("Opcao invalida.\n");
        pausar();
        return;
    }

    montar_saida_compactado(origem, destino, sizeof(destino));
    
    if (comprimir(origem, destino) == 0)
    {
        printf("Arquivo compactado com sucesso.\n");
        printf("Saida: %s\n", destino);
        pausar();
    }
    else
    {
        printf("Falha ao compactar o arquivo.\n");
        pausar();
    }
}
// Exibe o submenu de descompactação e executa a recuperação do arquivo.
static void menu_descompactar(void)
{
    char origem[MAX_CAMINHO];
    char destino[MAX_CAMINHO];
    
	limpar_tela();
    printf("\n=== DESCOMPACTAR ARQUIVO ===\n");

    if (!selecionar_arquivo_repositorio(1, origem, sizeof(origem)))
    {
    	pausar();
        return;
    }

    montar_saida_descompactado(origem, destino, sizeof(destino));

    if (descomprimir(origem, destino) == 0)
    {
        printf("Arquivo descompactado com sucesso.\n");
        printf("Saida: %s\n", destino);
        printf("Integridade: OK\n");
        pausar();
    }
    else
    {
        printf("Falha ao descompactar o arquivo.\n");
        pausar();
    }
}
// Exibe os arquivos compactados disponíveis no repositório.
static void listar_compactados(void)
{
	limpar_tela();
    printf("\n=== ARQUIVOS COMPACTADOS ===\n");
    (void)listar_arquivos_repositorio(1, NULL, NULL, 0);
    pausar();
}
// Exibe o submenu de remoção de arquivos compactados.
static void remover_compactado(void)
{	
    char caminho[MAX_CAMINHO];
    char resposta[16];

    int opcao;
	
	limpar_tela();
    printf("\n=== REMOVER ARQUIVO COMPACTADO ===\n");
    printf("1. Remover um arquivo\n");
    printf("2. Remover TODOS os arquivos .huff\n");
    printf("0. Retornar ao menu principal\n");

    if (!ler_opcao("Opcao: ", &opcao))
    {	pausar();
        return;
    }

	if (opcao == 0)
    {
        return;
    }
    else if (opcao == 1)
    {
        if (!selecionar_arquivo_repositorio(1, caminho, sizeof(caminho)))
        {	pausar();
            return;
        }

        if (!ler_linha("Confirma a exclusao? (s/n): ",
                       resposta,
                       sizeof(resposta)))
        {
            return;
        }

        if (resposta[0] != 's' && resposta[0] != 'S')
        {
            printf("Operacao cancelada.\n");
            pausar();
            return;
        }
		if (remove(caminho) == 0)
		{
		    printf("Arquivo removido com sucesso.\n");
			pausar();
		    registrar_operacao(
		        LOG_REMOCAO,
		        caminho,
		        NULL,
		        "OK");
		}
		else
		{
		    printf("Falha ao remover o arquivo.\n");
			pausar();
		    registrar_operacao(
		        LOG_REMOCAO,
		        caminho,
		        NULL,
		        "ERRO");
		}
    }
    else if (opcao == 2)
    {
        DIR *dir;
        struct dirent *ent;
		char resultado[64];
		
        int removidos = 0;

        if (!ler_linha(
                "Tem certeza que deseja remover TODOS os .huff? (s/n): ",
                resposta,
                sizeof(resposta)))
        {
            return;
        }

        if (resposta[0] != 's' && resposta[0] != 'S')
        {
            printf("Operacao cancelada.\n");
            pausar();
            return;
        }

        dir = opendir(REPOSITORIO_DIR);

        if (dir == NULL)
        {
            printf("Nao foi possivel abrir o repositorio.\n");
            pausar();
            return;
        }

        while ((ent = readdir(dir)) != NULL)
        {
            char arquivo[MAX_CAMINHO];

            if (!termina_com(ent->d_name, ".huff"))
            {
                continue;
            }

            snprintf(
                arquivo,
                sizeof(arquivo),
                "%s/%s",
                REPOSITORIO_DIR,
                ent->d_name);

            if (remove(arquivo) == 0)
            {
                removidos++;
            }
        }

        closedir(dir);
		
		printf("%d arquivo(s) removido(s).\n", removidos);
		pausar();
		
		snprintf(
		    resultado,
		    sizeof(resultado),
		    "%d arquivo(s) removido(s)",
		    removidos);
		
		registrar_operacao(
		    LOG_REMOCAO,
		    "TODOS",
		    NULL,
		    resultado);
    }
    else
    {
        printf("Opcao invalida.\n");
        pausar();
    }
}
// Exibe o conteúdo do log de operações.
static void visualizar_log(void)
{	limpar_tela();
    printf("\n=== VISUALIZAR LOG ===\n");

    exibir_log();

    pausar();
}

//==Main==
// Exibe o menu principal e coordena as operações do programa.
int main(void)
{
    int opcao;

    if (!garantir_repositorio())
    {
        printf("Nao foi possivel garantir a pasta '%s'.\n", REPOSITORIO_DIR);
        return 1;
    }

    do
    {	limpar_tela();
		printf("\n=== COMPACTADOR HUFFMAN ===\n");
        printf("1. Compactar arquivo\n");
        printf("2. Descompactar arquivo\n");
        printf("3. Listar arquivos compactados\n");
        printf("4. Remover arquivo compactado\n");
        printf("5. Visualizar log de operacoes\n");
        printf("6. Sair\n");

        if (!ler_opcao("\nOpcao: ", &opcao))
        {
            break;
        }

        switch (opcao)
        {
            case 1:
                menu_compactar();
                break;

            case 2:
                menu_descompactar();
                break;

            case 3:
                listar_compactados();
                break;

            case 4:
                remover_compactado();
                break;

            case 5:
                visualizar_log();
                break;

            case 6:
				printf("Encerrando.");
                break;

            default:
                printf("Opcao invalida.\n");
                pausar();
                break;
        }

    } while (opcao != 6);

    return 0;
}
