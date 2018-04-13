#include <ctype.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * EDITOR DE IMAGENS PPM
 * Codificação Western Windows 1252
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
 *  -t [CONTRASTE]  define o contraste da imagem
 *  -e              Espelhar imagem (inverter horizontalmente)
 *  -v              Virar imagem (inverter verticalmente)
 *  -g [GRAUS]      Girar imagem (90, 180, 270, -90, -180, -270)
 *  -d [APÓTEMA]    Distorcer a imagem (até metade do menor lado)
  */

/*<editor-fold desc="structs">*/
/* estrutura de um pixel */
typedef struct pxl {
    int r;
    int g;
    int b;
} Pixel;
/* estrutura de um arquivo */
typedef struct img {
    FILE *arquivo;
    char *nome;
    char tipo;
    unsigned int alt;
    unsigned int larg;
    int prof_cor;
    Pixel **pixels;
} Imagem;
/*</editor-fold>*/

/*<editor-fold desc="previous declarations">*/
/* filtros */
void filtro_negativo(Imagem *imagem);
void filtro_brilho(Imagem *imagem, float brilho);
void filtro_contraste(Imagem *imagem, float contraste);
void filtro_cor(Imagem *imagem, Pixel cor);
void filtro_espelhar(Imagem *imagem);
void filtro_virar(Imagem *imagem);
void filtro_girar(Imagem *imagem, int graus);
void filtro_distorcer(Imagem *imagem, int apotema);

/* alocação e exclusão de matrizes */
Pixel **pixels_aloca(unsigned int larg, unsigned int alt);
void pixels_apaga(Pixel **pixels, Imagem *imagem);
/* manipulação do tipo Imagem */
Imagem *imagem_carrega(char *caminho);
void imagem_salva(Imagem *imagem, const char *arq_saida);
/*  funções secundárias */
bool testa_param(const char *arg);
void erro_param();
void valida_cores(Imagem *imagem, int i, int j);
void erro_pixels();
char *cria_string(char *palavra);
int menor(int a, int b);
int aleatorio_entre(int min, int max);
/*</editor-fold>*/

/** FUNÇÃO PRINCIPAL */
int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "portuguese"); /*Define a codificação*/
    srand((unsigned) time(NULL));

    /* booleanos dos filtros */
    bool filtros = false, cores = false;
    bool o = false, n = false, e = false, v = false, b = false, g = false, d = false, c = false, t = false;
    int brilho = 0, graus = 0, apotema = 0, contraste = 0;
    Pixel cor = {0, 0, 0};
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
                case 'n': /* negativo */
                    n = true;
                    filtros++;
                    break;
                case 'e': /* espelhar */
                    e = true;
                    filtros++;
                    break;
                case 'v': /* virar */
                    v = true;
                    filtros++;
                    break;
                case 'b': /* brilho */
                    /* verifica se a porcentagem foi definida */
                    if (argc == i + 1 || testa_param(argv[i + 1])) {
                        if (argv[i + 1][0] == '-' && isdigit(argv[i + 1][1]))
                            goto jump_brilho; /* testa se não é um valor negativo */
                        printf("Porcentagem do brilho não definida ou inválida!\n");
                        erro_param();
                    }
                jump_brilho:
                    brilho = atoi(argv[i + 1]); /* NOLINT */
                    if (abs(brilho) > 100) {
                        printf("Porcentagem deve estar entre -100 e 100!\n");
                        erro_param();
                    }
                    i++; /* pula o próximo argumento (a porcentagem) */
                    b = true;
                    filtros++;
                    break;
                case 'g': /* girar */
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
                case 'd': /* distorcer */
                    /* verifica se a apótema foi definida */
                    if (argc == i + 1 || testa_param(argv[i + 1])) {
                        printf("Apótema não definida ou inválida!\n");
                        erro_param();
                    }
                    apotema = atoi(argv[i + 1]); /* NOLINT */
                    i++; /* pula o próximo argumento (a apotema) */
                    d = true;
                    filtros++;
                    break;
                case 'c': /* cores */
                    /* verifica se os parâmetros de cores foram definidos */
                    if (argc == i + 1) {
                        printf("Nenhuma cor definida!\b");
                        goto erro_cor; /* dentro do default do switch abaixo */
                    }
                    while (!testa_param(argv[i + 1])) {
                        /* lê o primeiro caractere para identificar qual a cor */
                        switch (argv[i + 1][0]) {
                            case 'r':
                                cor.r = atoi(&argv[i + 1][1]); /* NOLINT */
                                cores++;
                                break;
                            case 'g':
                                cor.g = atoi(&argv[i + 1][1]); /* NOLINT */
                                cores++;
                                break;
                            case 'b':
                                cor.b = atoi(&argv[i + 1][1]); /* NOLINT */
                                cores++;
                                break;
                            default:
                                printf("Cor inválida: %c\n", argv[i + 1][0]);
                            erro_cor:
                                printf("A cor é definida por r, g ou b seguida de um número\n"
                                       "Os parâmetros representam vermelho, verde e azul, respectivamente\n"
                                       "Pode-se definir apenas uma, duas ou as três cores\n");
                                erro_param();
                        }
                        i++; /* avança pro próximo parâmetro */
                        if (argc == i + 1) /* verifica se existem outros argumentos */
                            break;
                    }
                    if (!cores) {
                        printf("Pelo menos uma cor deve ser definida!\n");
                        goto erro_cor; /* dentro do default do switch acima */
                    }
                    c = true;
                    filtros++;
                    break;
                case 't':
                    /* verifica se a porcentagem foi definida */
                    if (argc == i + 1 || testa_param(argv[i + 1])) {
                        if (argv[i + 1][0] == '-' && isdigit(argv[i + 1][1]))
                            goto jump_contrast; /* testa se não é um valor negativo */
                        printf("Porcentagem do contraste não definida ou inválida!\n");
                        erro_param();
                    }
                jump_contrast:
                    contraste = atoi(argv[i + 1]); /* NOLINT */
                    if (abs(contraste) > 100) {
                        printf("Porcentagem deve estar entre -100 e 100!\n");
                        erro_param();
                    }
                    i++; /* pula o próximo argumento (a porcentagem) */
                    t = true;
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

    /* testa os parâmetros dos filtros */
    if (d) { /* verifica se a apótema informada não é maior que a da imagem */
        if (apotema > menor(imagem->alt, imagem->larg) / 2) {
            printf("A apótema da área de distorção não pode ser maior que a da imagem!\n");
            erro_param();
        }
    }
    if (c) { /* verifica se nenhuma cor é maior que a profundidade de cores da imagem */
        if (abs(cor.r) > imagem->prof_cor || abs(cor.g) > imagem->prof_cor || abs(cor.b) > imagem->prof_cor) {
            printf("As cores definidas não podem ser maiores do que a escala de cores da imagem!\n");
            erro_param();
        }
    }

    /* aplica os filtros */
    if (n) /* negativo */
        filtro_negativo(imagem);
    if (b) /* brilho */
        filtro_brilho(imagem, brilho);
    if (t) /* contraste */
        filtro_contraste(imagem, contraste);
    if (c) /* cor */
        filtro_cor(imagem, cor);
    if (e) /* espelhar */
        filtro_espelhar(imagem);
    if (v) /* virar */
        filtro_virar(imagem);
    if (g) /* girar */
        filtro_girar(imagem, graus);
    if (d) /* distorcer */
        filtro_distorcer(imagem, apotema);

    printf("\n");
    /* salva o arquivo */
    imagem_salva(imagem, arq_saida);
    return 0;
}

/** MANIPULAÇÃO DO ARQUIVO */
/*<editor-fold desc="file operations">*/
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
        for (j = 0; j < imagem->larg; j++) /* verifica se os 3 dados foram lidos (retorno da função) */
            if (fscanf(imagem->arquivo, "%d%d%d", &imagem->pixels[i][j].r, &imagem->pixels[i][j].g, /*NOLINT */
                       &imagem->pixels[i][j].b) != 3) {
                printf("Formato de arquivo inválido\n"
                       "O arquivo deve ser uma imagem PPM 3 no formato ASCII\n\n");
                exit(3);
            }
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
/*</editor-fold>*/

/** FILTROS */
/*<editor-fold desc="filters">*/
/* recebem um ponteiro para uma imagem e aplicam um filtro sobre ela */

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

void filtro_brilho(Imagem *imagem, float brilho) {
    printf("Aplicando %3.f%% de brilho...\n", brilho);
    float fat = (brilho / 100) * imagem->prof_cor;
    int i, j;
    for (i = 0; i < imagem->alt; i++)
        for (j = 0; j < imagem->larg; j++) {
            /* aplica o brilho em cada faixa de cor */
            imagem->pixels[i][j].r += fat;
            imagem->pixels[i][j].g += fat;
            imagem->pixels[i][j].b += fat;
            /* verifica se ultrapassou a profundidade de cor */
            valida_cores(imagem, i, j);
        }
}

void filtro_contraste(Imagem *imagem, float contraste) {
    printf("Aplicando %3.f%% de contraste...\n", contraste);
    float fat = (259 * (contraste + 255)) / (255 * (259 - contraste));
    int i, j;
    for (i = 0; i < imagem->alt; i++)
        for (j = 0; j < imagem->larg; j++) {
            /* aplica o contraste em cada faixa de cor */
            imagem->pixels[i][j].r = (int) (fat * (imagem->pixels[i][j].r - 128) + 128);
            imagem->pixels[i][j].g = (int) (fat * (imagem->pixels[i][j].g - 128) + 128);
            imagem->pixels[i][j].b = (int) (fat * (imagem->pixels[i][j].b - 128) + 128);
            /* verifica se ultrapassou a profundidade de cor */
            valida_cores(imagem, i, j);
        }
}

void filtro_cor(Imagem *imagem, Pixel cor) {
    printf("Aplicando filtro de cor RGB %d %d %d...\n", cor.r, cor.g, cor.b);
    int i, j;
    for (i = 0; i < imagem->alt; i++)
        for (j = 0; j < imagem->larg; j++) {
            /* incrementa as cores em cada pixel */
            imagem->pixels[i][j].r += cor.r;
            imagem->pixels[i][j].g += cor.g;
            imagem->pixels[i][j].b += cor.b;
            /* verifica se ultrapassou a profundidade de cor */
            valida_cores(imagem, i, j);
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
    printf("Girando imagem %d graus...\n", graus);
    Pixel **pont = NULL;
    int i, j;
    if (graus == 180) {
        /* girar 180 graus */
        pont = pixels_aloca(imagem->larg, imagem->alt);
        for (i = 0; i < imagem->alt; i++)
            for (j = 0; j < imagem->larg; j++)
                pont[imagem->alt - i - 1][imagem->larg - j - 1] = imagem->pixels[i][j];
    } else {
        pont = pixels_aloca(imagem->alt, imagem->larg);
        for (i = 0; i < imagem->larg; i++)
            for (j = 0; j < imagem->alt; j++)
                if (graus == 90)
                    /* girar 90 graus */
                    pont[i][imagem->alt - j - 1] = imagem->pixels[j][i];
                else
                    /*girar 270 graus */
                    pont[imagem->larg - i - 1][j] = imagem->pixels[j][i];
        /* redefine as dimensões da imagem */
        unsigned int aux = imagem->larg;
        imagem->larg = imagem->alt;
        imagem->alt = aux;
    }
    pixels_apaga(imagem->pixels, imagem);
    imagem->pixels = pont;
}

void filtro_distorcer(Imagem *imagem, int apotema) {
    printf("Aplicando distorção sobre uma área de %d pixels quadrados...", apotema * 2);
    Pixel aux;
    int i, j;
    for (i = 0; i < imagem->alt; i++)
        for (j = 0; j < imagem->larg; ++j) {
            /* busca um pixel aleatório dentro da área */
            int i_aleat = aleatorio_entre(i - apotema, i + apotema);
            if (i_aleat < 0)
                i_aleat = 0;
            if (i_aleat > imagem->alt - 1)
                i_aleat = imagem->alt - 1;
            int j_aleat = aleatorio_entre(j - apotema, j + apotema);
            if (j_aleat < 0)
                j_aleat = 0;
            if (j_aleat > imagem->larg - 1)
                j_aleat = imagem->larg - 1;
            /* troca com o pixel da iteração */
            aux = imagem->pixels[i_aleat][j_aleat];
            imagem->pixels[i_aleat][j_aleat] = imagem->pixels[i][j];
            imagem->pixels[i][j] = aux;
        }
}
/*</editor-fold>*/

/** FUNÇÕES SECUNDÁRIAS */
/*<editor-fold desc="secondary functions">*/
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

/* verifica se ultrapassou a profundidade de cor */
void valida_cores(Imagem *imagem, int i, int j) {
    if (imagem->pixels[i][j].r < 0)
        imagem->pixels[i][j].r = 0;
    if (imagem->pixels[i][j].r > imagem->prof_cor)
        imagem->pixels[i][j].r = imagem->prof_cor;
    if (imagem->pixels[i][j].g < 0)
        imagem->pixels[i][j].g = 0;
    if (imagem->pixels[i][j].g > imagem->prof_cor)
        imagem->pixels[i][j].g = imagem->prof_cor;
    if (imagem->pixels[i][j].b > imagem->prof_cor)
        imagem->pixels[i][j].b = imagem->prof_cor;
    if (imagem->pixels[i][j].b < 0)
        imagem->pixels[i][j].b = 0;
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
           "\t-b BRILHO\tBrilho    (em porcentagem)\n"
           "\t-t CONTRASTE\tContraste (em porcentagem)\n"
           "\t-v\t\tVirar     (inverter verticalmente)\n"
           "\t-e\t\tEspelhar  (inverter horizontalmente)\n"
           "\t-g GRAUS\tGirar     (90, 180 ou 270 graus)\n"
           "\t-d APÓTEMA\tDistorcer (apótema de distorção, em pixels)");
    exit(1);
}

/* copia uma string para um novo endereço de memória dinamicamente alocado */
char *cria_string(char *palavra) {
    char *string = malloc(sizeof(char) * strlen(palavra));
    if (string == NULL) {
        printf("Memória insuficiente!\n\tLibere mais memória e tente novamente.\n");
        exit(6);
    }
    strcpy(string, palavra);
    return string;
}

/* recebe dois valores e retorna o menor deles */
int menor(int a, int b) {
    if (a < b)
        return a;
    else
        return b;
}

int aleatorio_entre(int min, int max) {
    return rand() % (max - min) + min; /* NOLINT */
}
/*</editor-fold>*/
