#ifndef LOG_H
#define LOG_H

#define CAMINHO_LOG "logs/operacoes.log"

// Tipos de operação para o log
typedef enum {
    LOG_COMPRESSAO,
    LOG_DESCOMPRESSAO,
    LOG_REMOCAO,
    LOG_LISTAGEM
} TipoOperacao;

// Registra uma operação no arquivo de log com timestamp automático
// tipo:      tipo da operação (enum acima)
// entrada:   nome/caminho do arquivo de entrada (pode ser NULL)
// saida:     nome/caminho do arquivo de saída (pode ser NULL)
// resultado: descrição do resultado ("OK", "ERRO: arquivo não encontrado", etc.)
void registrar_operacao(TipoOperacao tipo, const char *entrada, const char *saida, const char *resultado);

// Exibe o conteúdo do log na tela
void exibir_log(void);

#endif
