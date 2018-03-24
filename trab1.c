#include <ctype.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * EDITOR DE IMAGENS PPM
 * 
 * Códigos de erro:
 *  1 Parâmetros incorretos
 *  2 Erro ao abrir arquivo
 *  3 Formato de arquivo inválido
 *  4 Dimensões inválidas
 *  5 Falha ao alocar Imagem (struct)
 *  6 Falha ao alocar string
 *  7 Falha ao alocar matriz (linhas)
 *  8 Falha ao alocar matriz (colunas)
 *
 * OPÇÕES
 *  -o [ARQUIVO]    arquivo de saída
 *  -n              Filtro negativar
 *  -e              Espelhar imagem (inverter horizontalmente)
 *  -v              Virar imagem (inverter verticalmente)
  */

/* estrutura de um pixel */
typedef struct pxl {
    unsigned int r;
    unsigned int g;
    unsigned int b;
} Pixel;
/* estrutura de um arquivo */
typedef struct img {
    FILE *arquivo;
    char *nome;
    char tipo;
    unsigned int alt;
    unsigned int larg;
    unsigned int prof_cor;
    Pixel **pixels;
} Imagem;

/*declarações das funções secundárias*/
void erro_param();
char *cria_string(char *palavra);
void erro_pixels();
bool testa_param(const char *arg);

void filtro_negativo(Imagem *imagem);

Pixel **pixels_aloca(unsigned int larg, unsigned int alt);

/* recebe uma matriz de pixels e a exclui */
void pixels_apaga(Pixel **pixels, Imagem *imagem);

void filtro_espelhar(Imagem *imagem);

void filtro_virar(Imagem *imagem);

/* função main */
int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "portuguese"); /*Define a codificação*/

    /* booleanos dos filtros */
    bool filtros = false;
    bool o = false;
    bool n = false;
    bool e = false;
    bool v = false;

    char *arq_saida = NULL;
    int i, j;

    /* testa os argumentos */
    if (argc < 3)
        erro_param();
    if (testa_param(argv[1])) /* verifica se o primeiro parâmetro pode ser arquivo */
        erro_param();

    /* verifica os parâmetros, começando pelo segundo */
    for (i = 2; i < argc; i++) {
        if (testa_param(argv[i]))
            switch (argv[i][1]) {
                case 'o': /* arquivo de saída */
                    /* verifica se o nome foi definido */
                    if (argc == i + 1 || testa_param(argv[i + 1])) {
                        printf("Arquivo de saída não definido!\n");
                        erro_param();
                    }
                    o = true;
                    arq_saida = cria_string(argv[i + 1]);
                    i++; /* pula o próximo argumento (o nome do arquivo) */
                    break;
                case 'n':
                    n = true;
                    filtros++;
                    break;
                case 'e':
                    e = true;
                    filtros++;
                    break;
                case 'v':
                    v = true;
                    filtros++;
                    break;
                default:
                    printf("Opção inválida: %s\n", argv[i]);
                    erro_param();
                    break;
            }
    }
    if (!filtros) {
        printf("Defina ao menos um filtro para aplicar na imagem!\n");
        erro_param();
    }

    /* abre o arquivo */
    Imagem *imagem;
    imagem = malloc(sizeof(Imagem));
    if (imagem == NULL) {
        printf("Memória insuficiente!\n\tLibere mais memória e tente novamente.\n");
        exit(5);
    }
    imagem->arquivo = fopen(argv[1], "r");
    if (imagem->arquivo == NULL) {
        printf("Falha ao abrir o arquivo %s", argv[1]);
        exit(2);
    }
    imagem->nome = cria_string(argv[1]); /* salva o nome do arquivo */
    /* verifica se é um arquivo válido */
    bool arq_valido = true;
    char tipo[5];
    fgets(tipo, 5, imagem->arquivo);
    if (strlen(tipo) != 3 || tipo[0] != 'P' || !isdigit(tipo[1]))
        arq_valido = false;
    if (arq_valido) {
        imagem->tipo = tipo[1];
    } else {
        printf("Formato de arquivo inválido\n");
        printf("Tipo: %c%c\t(%d)\n\n", tipo[0], tipo[1], strlen(tipo));
        exit(3);
    }
    /* lê o restante do cabeçalho */
    fscanf(imagem->arquivo, "%d%d%d", &imagem->larg, &imagem->alt, &imagem->prof_cor); /* NOLINT*/

    /* aloca e faz a leitura dos pixels */
    imagem->pixels = pixels_aloca(imagem->larg, imagem->alt);
    for (i = 0; i < imagem->alt; i++)
        for (j = 0; j < imagem->larg; j++)
            fscanf(imagem->arquivo, "%d%d%d", &imagem->pixels[i][j].r, &imagem->pixels[i][j].g, /*NOLINT*/
                   &imagem->pixels[i][j].b);
    printf("Arquivo %s carregado com sucesso\n", imagem->nome);
    fclose(imagem->arquivo);

    if (o)
        printf("O arquivo de saída está definido como %s\n\n", arq_saida);
    else {
        printf("Arquivo de saída não definido. A imagem será sobre-escrita\n\n");
        arq_saida = argv[1];
    }

    if (n) /* filtro negativo */
        filtro_negativo(imagem);
    if (e) /* espelhar */
        filtro_espelhar(imagem);
    if (v) /* virar */
        filtro_virar(imagem);

    /* salva o arquivo */
    imagem->arquivo = fopen(arq_saida, "w");
    fprintf(imagem->arquivo, "P%c\n", imagem->tipo);
    fprintf(imagem->arquivo, "%d %d\n", imagem->larg, imagem->alt);
    fprintf(imagem->arquivo, "%d\n", imagem->prof_cor);
    for (i = 0; i < imagem->alt; i++)
        for (j = 0; j < imagem->larg; j++) {
            fprintf(imagem->arquivo, "%d %d %d\n", imagem->pixels[i][j].r, imagem->pixels[i][j].g,
                    imagem->pixels[i][j].b);
        }
    fclose(imagem->arquivo);
    return 0;
}


/** FILTROS
 *  recebem um ponteiro para uma imagem e aplicam um filtro sobre ela */
void filtro_negativo(Imagem *imagem) {
    printf("Aplicando filtro negativo...\n");
    int i, j;
    for (i = 0; i < imagem->alt; i++)
        for (j = 0; j < imagem->larg; j++) {
            imagem->pixels[i][j].r = imagem->prof_cor - imagem->pixels[i][j].r;
            imagem->pixels[i][j].g = imagem->prof_cor - imagem->pixels[i][j].g;
            imagem->pixels[i][j].b = imagem->prof_cor - imagem->pixels[i][j].b;
        }
}

void filtro_espelhar(Imagem *imagem) {
    printf("Espelhando imagem...\n");
    Pixel **pont = pixels_aloca(imagem->larg, imagem->alt);
    int i, j;
    for (i = 0; i < imagem->alt; i++) {
        int k = 0;
        for (j = imagem->larg - 1; j >= 0; j--) {
            pont[i][k] = imagem->pixels[i][j];
            k++;
        }
    }
    pixels_apaga(imagem->pixels, imagem);
    imagem->pixels = pont;
}

void filtro_virar(Imagem *imagem) {
    printf("Virando imagem...\n");
    Pixel **pont = pixels_aloca(imagem->larg, imagem->alt);
    int i, j;
    int k = 0;
    for (i = imagem->alt - 1; i >= 0; i--) {
        for (j = 0; j < imagem->larg; j++)
            pont[k][j] = imagem->pixels[i][j];
        k++;
    }
    pixels_apaga(imagem->pixels, imagem);
    imagem->pixels = pont;
}

/** FUNÇÕES SECUNDÁRIAS */
/* recebe as dimensões e retorna uma matriz de pixels */
Pixel **pixels_aloca(unsigned int larg, unsigned int alt) {
    Pixel **pixels = malloc(sizeof(Pixel *) * alt);
    int i;
    if (pixels == NULL) {
        erro_pixels();
        exit(7);
    }
    for (i = 0; i < alt; i++) {
        pixels[i] = malloc(sizeof(Pixel) * larg);
        if (pixels[i] == NULL) {
            erro_pixels();
            exit(8);
        }
    }
    return pixels;
}

/* recebe uma matriz de pixels e a exclui */
void pixels_apaga(Pixel **pixels, Imagem *imagem) {
    int i;
    if (pixels == NULL || imagem == NULL)
        return;
    for (i = 0; i < imagem->larg; i++)
        free(pixels[i]);
    free(pixels);
}

/* testa se o item é um parâmetro */
bool testa_param(const char *arg) {
    if (*arg == '-' || *arg == '\\' || *arg == '/' && strlen(arg) > 1)
        return true;
    else
        return false;
}

/* informa do erro ao alocar os pixels */
void erro_pixels() {
    printf("Erro ao alocar espaço para carregar imagem\nLibere mais memória e tente novamente.\n");
}

/* imprime instruções e encerra o programa com código 1 */
void erro_param() {
    printf("Use [IMAGEM] [OPÇÕES]\n");
    printf("O primeiro argumento deve ser o nome arquivo\n\n");
    printf("As opções podem ser:\n");
    printf("\t-o ARQUIVO\t Informe o nome do arquivo para salvar\n");
    printf("\t-n\t\t Filtro Negativo\n");
    printf("\t-e\t\t Espelhar (inverter horizontalmente)\n");
    printf("\t-v\t\t Virar    (inverter verticalmente)\n");
    exit(1);
}

/*copia uma string para um novo endereço de memória dinamicamente alocado*/
char *cria_string(char *palavra) {
    char *string = malloc(sizeof(char) * strlen(palavra));
    if (string == NULL) {
        printf("Memória insuficiente!\n\tLibere mais memória e tente "
                       "novamente.\n");
        exit(6);
    }
    strcpy(string, palavra);
    return string;
}