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
 *  9 Erro ao salvar o arquivo
 *
 * OPÇÕES
 *  -o [ARQUIVO]    arquivo de saída
 *  -n              Filtro negativar
 *  -b [BRILHO]     define o brilho da imagem
 *  -e              Espelhar imagem (inverter horizontalmente)
 *  -v              Virar imagem (inverter verticalmente)
 *  -g [GRAUS]      Girar imagem (90, 180, 270, -90, -180, -270)
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

/*<editor-fold desc="previous declarations">*/
/* filtros */
void filtro_negativo(Imagem *imagem);
void filtro_espelhar(Imagem *imagem);
void filtro_virar(Imagem *imagem);
void filtro_brilho(Imagem *imagem, float brilho);
void filtro_girar(Imagem *imagem, int graus);

/* alocação e exclusão de matrizes */
Pixel **pixels_aloca(unsigned int larg, unsigned int alt);
void pixels_apaga(Pixel **pixels, Imagem *imagem);
/* manipulação do tipo Imagem */
Imagem *imagem_carrega(char *caminho);
void imagem_salva(Imagem *imagem, const char *arq_saida);
/*  funções secundárias */
bool testa_param(const char *arg);
void erro_param();
void erro_pixels();
char *cria_string(char *palavra);
/*</editor-fold>*/

/* função main */
int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "portuguese"); /*Define a codificação*/

    /* booleanos dos filtros */
    bool filtros = false;
    bool o = false, n = false, e = false, v = false, b = false, g = false;
    int brilho = 0, graus = 0;
    char *arq_saida = NULL;
    int i;

    /* testa os argumentos */
    if (argc < 3)
        erro_param();
    if (testa_param(argv[1])) /* verifica se o primeiro parâmetro pode ser um arquivo */
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
                case 'b':
                    /* verifica se a porcentagem foi definida */
                    if (argc == i + 1 || testa_param(argv[i + 1])) {
                        printf("Porcentagem do brilho não definida ou inválida!\n");
                        erro_param();
                    }
                    brilho = atoi(argv[i + 1]); /* NOLINT*/
                    i++; /* pula o próximo argumento (a porcentagem) */
                    b = true;
                    filtros++;
                    break;
                case 'g':
                    /* verifica se os graus foram definidos e são válidos */
                    if (argc == i + 1 || testa_param(argv[i + 1])) {
                        printf("Graus para rotacionar não definidos ou inválidos!\n");
                        erro_param();
                    }
                    graus = atoi(argv[i + 1]); /* NOLINT */
                    if (!(graus == 90 || graus == 180 || graus == 270)) {
                        printf("Os graus devem ser 90, 180 ou 270!\n");
                        erro_param();
                    }
                    i++; /* pula o próximo argumento (os graus) */
                    g = true;
                    filtros++;
                    break;
                default:
                opcao_invalida:
                    printf("Opção inválida: %s\n", argv[i]);
                    erro_param();
                    break;
            }
        else /* executa o default do switch acima */
            goto opcao_invalida;
    }
    if (!filtros) {
        printf("Defina ao menos um filtro para aplicar na imagem!\n");
        erro_param();
    }

    /* abre o arquivo e define o arquivo de saída */
    Imagem *imagem = imagem_carrega(argv[1]);
    if (o)
        printf("O arquivo de saída está definido como %s\n\n", arq_saida);
    else {
        printf("Arquivo de saída não definido. A imagem será sobre-escrita\n\n");
        arq_saida = imagem->nome;
    }

    if (n) /* filtro negativo */
        filtro_negativo(imagem);
    if (b) /* brilho */
        filtro_brilho(imagem, brilho);
    if (e) /* espelhar */
        filtro_espelhar(imagem);
    if (v) /* virar */
        filtro_virar(imagem);
    if (g) /* girar */
        filtro_girar(imagem, graus);

    printf("\n");
    /* salva o arquivo */
    imagem_salva(imagem, arq_saida);
    return 0;
}

/** MANIPULAÇÃO DO ARQUIVO */
/* recebe um caminho, abre, testa o cabeçalho e faz a leitura dos pixels */
Imagem *imagem_carrega(char *caminho) {
    Imagem *imagem;
    int i, j;
    imagem = malloc(sizeof(Imagem));
    if (imagem == NULL) {
        printf("Memória insuficiente!\n\tLibere mais memória e tente novamente.\n");
        exit(5);
    }
    imagem->arquivo = fopen(caminho, "r");
    if (imagem->arquivo == NULL) {
        printf("Falha ao abrir o arquivo %s", caminho);
        exit(2);
    }
    imagem->nome = cria_string(caminho); /* salva o nome do arquivo */
    /* verifica se é um arquivo válido */
    bool arq_valido;
    char tipo[5];
    fgets(tipo, 5, imagem->arquivo);
    arq_valido = !(strlen(tipo) != 3 || tipo[0] != 'P' || !isdigit(tipo[1]));
    if (arq_valido) {
        imagem->tipo = tipo[1];
    } else {
        printf("Formato de arquivo inválido\n");
        printf("Tipo: %c%c\t(%d)\n\n", tipo[0], tipo[1], strlen(tipo));
        exit(3);
    }
    /* lê o restante do cabeçalho */
    fscanf(imagem->arquivo, "%d%d%d", &imagem->larg, &imagem->alt, &imagem->prof_cor); /* NOLINT */

    /* aloca e faz a leitura dos pixels */
    imagem->pixels = pixels_aloca(imagem->larg, imagem->alt);
    for (i = 0; i < imagem->alt; i++)
        for (j = 0; j < imagem->larg; j++)
            fscanf(imagem->arquivo, "%d%d%d", &imagem->pixels[i][j].r, &imagem->pixels[i][j].g, /*NOLINT */
                   &imagem->pixels[i][j].b);
    printf("Arquivo %s carregado com sucesso\n", imagem->nome);
    fclose(imagem->arquivo);
    return imagem;
}

/* recebe uma imagem e um caminho de saída para salvar */
void imagem_salva(Imagem *imagem, const char *arq_saida) {
    printf("Salvando arquivo %s... ", arq_saida);
    int i, j;
    imagem->arquivo = fopen(arq_saida, "w");
    if (imagem->arquivo == NULL) {
        printf("Erro ao salvar o arquivo\n"
               "Verifique se você tem as permissões necessárias");
        exit(9);
    }
    fprintf(imagem->arquivo, "P%c\n", imagem->tipo);
    fprintf(imagem->arquivo, "%d %d\n", imagem->larg, imagem->alt);
    fprintf(imagem->arquivo, "%d\n", imagem->prof_cor);
    for (i = 0; i < imagem->alt; i++)
        for (j = 0; j < imagem->larg; j++) {
            fprintf(imagem->arquivo, "%d %d %d\n", imagem->pixels[i][j].r, imagem->pixels[i][j].g,
                    imagem->pixels[i][j].b);
        }
    /* fecha o arquivo e libera a memória alocada */
    fclose(imagem->arquivo);
    pixels_apaga(imagem->pixels, imagem);
    free(imagem->nome);
    free(imagem);
    printf("salvo\n");
}

/** FILTROS
 *  recebem um ponteiro para uma imagem e aplicam um filtro sobre ela */
void filtro_negativo(Imagem *imagem) {
    printf("Aplicando filtro negativo...\n");
    int i, j;
    for (i = 0; i < imagem->alt - 1; i++)
        for (j = 0; j < imagem->larg - 1; j++) {
            imagem->pixels[i][j].r = imagem->prof_cor - imagem->pixels[i][j].r;
            imagem->pixels[i][j].g = imagem->prof_cor - imagem->pixels[i][j].g;
            imagem->pixels[i][j].b = imagem->prof_cor - imagem->pixels[i][j].b;
        }
}

void filtro_brilho(Imagem *imagem, float brilho) {
    printf("Aplicando %3.f%% brilho...\n", brilho);
    float fat = brilho / 100;
    int i, j;
    for (i = 0; i < imagem->alt - 1; i++)
        for (j = 0; j < imagem->larg - 1; j++) {
            imagem->pixels[i][j].r *= fat;
            if (imagem->pixels[i][j].r > imagem->prof_cor)
                imagem->pixels[i][j].r = imagem->prof_cor;
            imagem->pixels[i][j].g *= fat;
            if (imagem->pixels[i][j].g > imagem->prof_cor)
                imagem->pixels[i][j].g = imagem->prof_cor;
            imagem->pixels[i][j].b *= fat;
            if (imagem->pixels[i][j].b > imagem->prof_cor)
                imagem->pixels[i][j].b = imagem->prof_cor;
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

void filtro_girar(Imagem *imagem, int graus) {
    int i, j;
    printf("Girando imagem %d graus...\n", graus);
    Pixel **pont = NULL;
    if (graus == 180) {
        /* girar 180 graus */
        pont = pixels_aloca(imagem->larg, imagem->alt);
        for (i = 0; i < imagem->alt; i++)
            for (j = 0; j < imagem->larg; j++) {
                pont[imagem->alt - i - 1][imagem->larg - j - 1].r = imagem->pixels[i][j].r;
                pont[imagem->alt - i - 1][imagem->larg - j - 1].g = imagem->pixels[i][j].g;
                pont[imagem->alt - i - 1][imagem->larg - j - 1].b = imagem->pixels[i][j].b;
            }
    } /*else {
        pont = pixels_aloca(imagem->alt, imagem->larg);
        if (graus == 90)
            *//* TODO girar 90 graus *//*
        else
            *//* TODO girar 270 graus *//*
    }*/

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
    for (i = 0; i < imagem->alt - 1; i++)
        free(pixels[i]);
    free(pixels);
}

/* testa se o item é um parâmetro */
bool testa_param(const char *arg) {
    return (*arg == '-' || *arg == '\\' || *arg == '/' && strlen(arg) > 1);
}

/* informa do erro ao alocar os pixels */
void erro_pixels() {
    printf("Erro ao alocar espaço para carregar imagem\nLibere mais memória e tente novamente.\n");
}

/* imprime instruções e encerra o programa com código 1 */
void erro_param() {
    printf("\nUse [IMAGEM] [OPÇÕES]\n"
           "O primeiro argumento deve ser o nome do arquivo\n\n");
    printf("As opções podem ser:\n"
           "\t-o ARQUIVO\tInformar o arquivo de saída\n"
           "\t-n\t\tFiltro Negativo\n"
           "\t-b BRILHO\tBrilho   (em porcentagem)\n"
           "\t-e\t\tEspelhar (inverter horizontalmente)\n"
           "\t-v\t\tVirar    (inverter verticalmente)\n"
           "\t-g GRAUS\tGirar    (90, 180 ou 270 graus)\n");
    exit(1);
}

/*copia uma string para um novo endereço de memória dinamicamente alocado*/
char *cria_string(char *palavra) {
    char *string = malloc(sizeof(char) * strlen(palavra));
    if (string == NULL) {
        printf("Memória insuficiente!\n\tLibere mais memória e tente novamente.\n");
        exit(6);
    }
    strcpy(string, palavra);
    return string;
}
