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
 *
 * OP��ES
 *  -o [ARQUIVO]    arquivo de sa�da
 *  -n              Filtro negativar
  */

/* estrutura de um arquivo */
typedef struct img {
    FILE *arquivo;
    char *nome;
    char tipo;
    unsigned int alt;
    unsigned int larg;
    unsigned int prof_cor;
} Imagem;
/* estrutura de um pixel */
typedef struct pxl {
    unsigned int r;
    unsigned int g;
    unsigned int b;
} Pixel;

/*declara��es das fun��es secund�rias*/
void erro_param();

char *cria_string(char *palavra);

void erro_pixels();

bool testa_param(const char *arg);

/* fun��o main */
int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "portuguese"); /*Define a codifica��o*/

    bool filtro_definido = false;
    bool o = false;
    bool n = false;
    char *arq_saida = NULL;

    /* testa os argumentos */
    if (argc < 3)
        erro_param();
    if (testa_param(argv[1])) /* verifica se o primeiro par�metro pode ser arquivo */
        erro_param();

    /* verifica os par�metros, come�ando pelo segundo */
    for (int i = 2; i < argc; i++) {
        if (testa_param(argv[i]))
            switch (argv[i][1]) {
                case 'o': /* arquivo de sa�da */
                    /* verifica se o nome foi definido */
                    if (argc == i + 1 || testa_param(argv[i + 1])) {
                        printf("Arquivo de sa�da n�o definido!\n");
                        erro_param();
                    }
                    o = true;
                    arq_saida = cria_string(argv[i + 1]);
                    i++; /* pula o pr�ximo argumento (o nome do arquivo) */
                    break;
                case 'n':
                    n = true;
                    filtro_definido = true;
                    break;
                default:
                    printf("Op��o inv�lida: %s\n", argv[i]);
                    erro_param();
                    break;
            }
    }
    if (!filtro_definido) {
        printf("Defina ao menos um filtro para aplicar na imagem!\n");
        erro_param();
    }

    /* abre o arquivo */
    Imagem *imagem;
    imagem = malloc(sizeof(Imagem));
    if (imagem == NULL) {
        printf("Mem�ria insuficiente!\n\tLibere mais mem�ria e tente novamente.\n");
        exit(5);
    }
    imagem->arquivo = fopen(argv[1], "r");
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
    if (arq_valido) {
        imagem->tipo = tipo[1];
    } else {
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
    fclose(imagem->arquivo);

    if (o)
        printf("O arquivo de sa�da est� definido como %s\n", arq_saida);
    else {
        printf("Arquivo de sa�da n�o definido. A imagem ser� sobre-escrita\n");
        arq_saida = argv[1];
    }
    /* filtro negativo */
    printf("Aplicando filtro negativo...");
    if (n) {
        for (int i = 0; i < imagem->alt; i++)
            for (int j = 0; j < imagem->larg; j++) {
                pixels[i][j].r = imagem->prof_cor - pixels[i][j].r;
                pixels[i][j].g = imagem->prof_cor - pixels[i][j].g;
                pixels[i][j].b = imagem->prof_cor - pixels[i][j].b;
            }
    }

    /* salva o arquivo */
    imagem->arquivo = fopen(arq_saida, "w");
    fprintf(imagem->arquivo, "P%c\n", imagem->tipo);
    fprintf(imagem->arquivo, "%d %d\n", imagem->larg, imagem->alt);
    fprintf(imagem->arquivo, "%d\n", imagem->prof_cor);
    for (int i = 0; i < imagem->alt; i++)
        for (int j = 0; j < imagem->larg; j++) {
            fprintf(imagem->arquivo, "%d %d %d\n", pixels[i][j].r, pixels[i][j].g, pixels[i][j].b);
        }
    fclose(imagem->arquivo);
    return 0;
}

/* testa se o item � um par�metro */
bool testa_param(const char *arg) {
    if (*arg == '-' || *arg == '\\' || *arg == '/' && strlen(arg) > 1)
        return true;
    else
        return false;
}

/* informa do erro ao alocar os pixels e encerra o programa */
void erro_pixels() {
    printf("Erro ao alocar espa�o para carregar imagem\nLibere mais mem�ria e tente novamente.\n");
}

/* imprime instru��es e encerra o programa com c�digo 1 */
void erro_param() {
    printf("Use [IMAGEM] [OP��ES]\n");
    printf("O primeiro argumento deve ser o nome arquivo\n\n");
    printf("As op��es podem ser:\n");
    printf("-o ARQUIVO: Informe o nome do arquivo para salvar\n");
    printf("-n: Filtro Negativo\n");
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