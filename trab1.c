#include <ctype.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * EDITOR DE IMAGENS PPM
 * 
 * C�digos de erro:
 *  1 Par�metros incorretos
 *  2 Erro ao abrir arquivo
 *  3 Formato de arquivo inv�lido
 *  4 Dimens�es inv�lidas
 *  5 Falha ao alocar Imagem (struct)
 *  6 Falha ao alocar string
 *  7 Falha ao alocar matriz (linhas)
 *  8 Falha ao alocar matriz (colunas)
  */

/* estrutura de um arquivo */
typedef struct img {
    FILE *arquivo;
    char *nome;
    unsigned int alt;
    unsigned int larg;
    unsigned int prof_cor;
} Imagem;
/* estrutura de um pixel */
typedef struct pxl {
    int r;
    int g;
    int b;
} Pixel;

/*declara��es das fun��es secund�rias*/
void erro_param(char *nome);

char *cria_string(char *palavra);

void erro_pixels();

/* fun��o main */
int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "portuguese"); /*Define a codifica��o*/

    /* testa os argumentos */
    if (argc == 1)
        erro_param(argv[0]);
    if (*argv[1] == '-' || *argv[1] == '\\') /* verifica se o primeiro par�metro pode ser arquivo */
        erro_param(argv[0]);

    /* abre o arquivo */
    Imagem *imagem;
    imagem = malloc(sizeof(Imagem));
    if (imagem == NULL) {
        printf("Mem�ria insuficiente!\n\tLibere mais mem�ria e tente novamente.\n");
        exit(5);
    }
    imagem->arquivo = fopen(argv[1], "r+");
    if (imagem->arquivo == NULL) {
        printf("Falha ao abrir o arquivo %s", argv[1]);
        exit(2);
    }
    imagem->nome = cria_string(argv[1]); /* salva o nome do arquivo */
    /* verifica se � um arquivo v�lido */
    bool arq_valido = true;
    char tipo[5];
    fgets(tipo, 5, imagem->arquivo);
    if (strlen(tipo) != 3 || tipo[0] != 'P' || !isdigit(tipo[1]))
        arq_valido = false;
    if (!arq_valido) {
        printf("Formato de arquivo inv�lido\n");
        printf("Tipo: %c%c\t(%d)\n\n", tipo[0], tipo[1],
               strlen(tipo));
        exit(3);
    }
    /* l� o restante do cabe�alho */
    fscanf(imagem->arquivo, "%d%d%d", &imagem->larg, &imagem->alt, &imagem->prof_cor); // NOLINT

    /* aloca o espa�o para os pixels */
    Pixel **pixels = malloc(sizeof(Pixel *) * imagem->alt);
    if (pixels == NULL) {
        erro_pixels();
        exit(7);
    }
    for (int i = 0; i < imagem->alt; i++) {
        pixels[i] = malloc(sizeof(Pixel) * imagem->larg);
        if (pixels[i] == NULL) {
            erro_pixels();
            exit(8);
        }
    }
    for (int i = 0; i < imagem->alt; i++)
        for (int j = 0; j < imagem->larg; j++)
            fscanf(imagem->arquivo, "%d%d%d", &pixels[i][j].r, &pixels[i][j].g, &pixels[i][j].b); // NOLINT
    printf("Arquivo %s carregado com sucesso\n", imagem->nome);

    return 0;
}

/* informa do erro ao alocar os pixels e encerra o programa */
void erro_pixels() {
    printf("Erro ao alocar espa�o para carregar imagem\nLibere mais mem�ria e tente novamente.\n");
}

/* imprime instru��es e encerra o programa com c�digo 1 */
void erro_param(char *nome) {
    printf("Use %s [SA�DA] [OP��ES]\n", nome);
    printf("O primeiro argumento deve ser o nome arquivo de sa�da\n\n");
    printf("As op��es podem ser:\n");
    exit(1);
}

/*copia uma string para um novo endere�o de mem�ria dinamicamente alocado*/
char *cria_string(char *palavra) {
    char *string = malloc(sizeof(char) * strlen(palavra));
    if (string == NULL) {
        printf("Mem�ria insuficiente!\n\tLibere mais mem�ria e tente "
                       "novamente.\n");
        exit(6);
    }
    strcpy(string, palavra);
    return string;
}