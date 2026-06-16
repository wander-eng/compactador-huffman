#include <stdio.h>

int main(void)
{
    int opcao;

    do
    {
        printf("\n=== COMPACTADOR HUFFMAN ===\n");
        printf("1. Compactar arquivo\n");
        printf("2. Descompactar arquivo\n");
        printf("3. Listar arquivos compactados\n");
        printf("4. Remover arquivo compactado\n");
        printf("5. Visualizar log de operacoes\n");
        printf("6. Sair\n");
        printf("\nOpcao: ");

        scanf("%d", &opcao);

        switch (opcao)
        {
            case 1:
            {
                int subopcao;

                printf("\n=== COMPACTAR ARQUIVO ===\n");
                printf("1. Selecionar arquivo da pasta do repositorio\n");
                printf("2. Digitar caminho manualmente\n");
                printf("\nOpcao: ");

                scanf("%d", &subopcao);

                switch (subopcao)
                {
                    case 1:
                        printf("Funcionalidade ainda nao implementada.\n");
                        break;

                    case 2:
                        printf("Funcionalidade ainda nao implementada.\n");
                        break;

                    default:
                        printf("Opcao invalida.\n");
                }

                break;
            }

            case 2:
                printf("Funcionalidade ainda nao implementada.\n");
                break;

            case 3:
                printf("Funcionalidade ainda nao implementada.\n");
                break;

            case 4:
                printf("Funcionalidade ainda nao implementada.\n");
                break;

            case 5:
                printf("Funcionalidade ainda nao implementada.\n");
                break;

            case 6:
                printf("Encerrando programa...\n");
                break;

            default:
                printf("Opcao invalida.\n");
        }

    } while (opcao != 6);

    return 0;
}
