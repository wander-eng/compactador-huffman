#ifndef ARQUIVO_H
#define ARQUIVO_H

// Comprime o arquivo de entrada e salva em caminho_saida
// Retorna 0 em sucesso, -1 em erro
int comprimir(const char *caminho_entrada, const char *caminho_saida);

// Descomprime o arquivo de entrada e salva em caminho_saida
// Retorna 0 em sucesso, -1 em erro
int descomprimir(const char *caminho_entrada, const char *caminho_saida);

// Verifica integridade comparando tamanho recuperado com tamanho original
// Retorna 1 se íntegro, 0 se divergente
int verificar_integridade(const char *caminho_huff, const char *caminho_recuperado);

#endif
