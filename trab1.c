#include <ctype.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * EDITOR DE IMAGENS PPM
 * Codifica��o Western Windows 1252
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
 *  9 Erro ao salvar o arquivo
 *
 * OP��ES
 *  -o [ARQUIVO]    arquivo de sa�da
 *  -u [TAM] [POS]  �rea de recorte (tamanho e posi��o inicial)
 *  -n              Filtro negativar
 *  -b [BRILHO]     define o brilho da imagem
 *  -t [CONTRASTE]  define o contraste da imagem
 *  -e              Espelhar imagem (inverter horizontalmente)
 *  -v              Virar imagem (inverter verticalmente)
 *  -g [GRAUS]      Girar imagem (90, 180, 270)
 *  -d [AP�TEMA]    Distorcer a imagem (at� metade do menor lado)
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
void filtro_corta(Imagem *imagem, unsigned int alt, unsigned int larg, int corte_i, int corte_j);
void filtro_negativo(Imagem *imagem);
void filtro_brilho(Imagem *imagem, float brilho);
void filtro_contraste(Imagem *imagem, float contraste);
void filtro_cor(Imagem *imagem, Pixel cor);
void filtro_espelhar(Imagem *imagem);
void filtro_virar(Imagem *imagem);
void filtro_girar(Imagem *imagem, int graus);
void filtro_distorcer(Imagem *imagem, int apotema);

/* aloca��o e exclus�o de matrizes */
Pixel **pixels_aloca(unsigned int alt, unsigned int larg);
void pixels_apaga(Imagem *imagem);
/* manipula��o do tipo Imagem */
Imagem *imagem_carrega(char *caminho);
void imagem_salva(Imagem *imagem, const char *arq_saida);
/*  fun��es secund�rias */
bool testa_param(const char *arg);
void erro_param();
void valida_cores(Imagem *imagem, int i, int j);
void erro_pixels();
char *cria_string(char *palavra);
int menor(int a, int b);
int aleatorio_entre(int min, int max);
/*</editor-fold>*/

void convulacao(Imagem *imagem, int kernel[3][3], int divisor);

/** FUN��O PRINCIPAL */
int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "portuguese"); /*Define a codifica��o*/
    srand((unsigned) time(NULL));

    /* booleanos dos filtros */
    bool filtros = false, cores = false;
    bool o = false, n = false, e = false, v = false, b = false, g = false, d = false, c = false, t = false, u = false, k = false;
    int brilho = 0, graus = 0, apotema = 0, contraste = 0, corte_i = 0, corte_j = 0, kernel[3][3], divisor = 1;
    unsigned int corte_larg = 0;
    unsigned int corte_alt = 0;
    Pixel cor = {0, 0, 0};
    char *arq_saida = NULL;
    int i;

    /* testa os argumentos */
    if (argc < 3)
        erro_param();
    if (testa_param(argv[1])) /* verifica se o primeiro par�metro pode ser um arquivo */
        erro_param();

    /* verifica os par�metros, come�ando pelo segundo */
    for (i = 2; i < argc; i++) {
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
                case 'u': /* corte */
                    /* verifica se a �rea foi definida */
                    if (argc == i + 1 || testa_param(argv[i + 1]) || argc == i + 2 || testa_param(argv[i + 2])) {
                        printf("�rea de recorte n�o definida ou inv�lida!\n");
                        erro_param();
                    }
                    /* l� os dois valores */
                    corte_alt = (unsigned int) atoi(argv[i + 1]); /* NOLINT */
                    corte_larg = (unsigned int) atoi(argv[i + 2]); /* NOLINT */
                    if (corte_alt < 1 || corte_larg < 1) {
                        printf("A �rea deve ser maior que 0!\n");
                        erro_param();
                    }
                    i += 2; /* pula os pr�ximos argumentos (a �rea) */
                    /* verifica se h� mais argumentos para saber se foi informada uma posi��o de corte */
                    if (argc != i + 1 && !testa_param(argv[i + 1]))
                        if (argc != i + 2 && !testa_param(argv[i + 2])) {
                            corte_i = atoi(argv[i + 1]); /* NOLINT */
                            corte_j = atoi(argv[i + 2]); /* NOLINT */
                            i += 2; /* pula os pr�ximos argumentos (a posi��o) */
                        } else {
                            printf("Posi��o inicial de corte inv�lida: %s,%s\n", argv[i + 1], argv[i + 2]);
                            printf("Informe X e Y, ex.: -u 130 125 25 0\n");
                            erro_param();
                        }
                    u = true;
                    filtros++;
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
                            goto jump_brilho; /* testa se n�o � um valor negativo */
                        printf("Porcentagem do brilho n�o definida ou inv�lida!\n");
                        erro_param();
                    }
                jump_brilho:
                    brilho = atoi(argv[i + 1]); /* NOLINT */
                    if (abs(brilho) > 100) {
                        printf("Porcentagem deve estar entre -100 e 100!\n");
                        erro_param();
                    }
                    i++; /* pula o pr�ximo argumento (a porcentagem) */
                    b = true;
                    filtros++;
                    break;
                case 'g': /* girar */
                    /* verifica se os graus foram definidos e s�o v�lidos */
                    if (argc == i + 1 || testa_param(argv[i + 1])) {
                        printf("Graus para rotacionar n�o definidos ou inv�lidos!\n");
                        erro_param();
                    }
                    graus = atoi(argv[i + 1]); /* NOLINT */
                    if (!(graus == 90 || graus == 180 || graus == 270)) {
                        printf("Os graus devem ser 90, 180 ou 270!\n");
                        erro_param();
                    }
                    i++; /* pula o pr�ximo argumento (os graus) */
                    g = true;
                    filtros++;
                    break;
                case 'd': /* distorcer */
                    /* verifica se a ap�tema foi definida */
                    if (argc == i + 1 || testa_param(argv[i + 1])) {
                        printf("Ap�tema n�o definida ou inv�lida!\n");
                        erro_param();
                    }
                    apotema = atoi(argv[i + 1]); /* NOLINT */
                    if (!apotema) {
                        printf("Valor da ap�tema deve ser diferente de zero!\n");
                        erro_param();
                    }
                    i++; /* pula o pr�ximo argumento (a apotema) */
                    d = true;
                    filtros++;
                    break;
                case 'c': /* cores */
                    /* verifica se os par�metros de cores foram definidos */
                    if (argc == i + 1) {
                        printf("Nenhuma cor definida!\b");
                        goto erro_cor; /* dentro do default do switch abaixo */
                    }
                    while (!testa_param(argv[i + 1])) {
                        /* l� o primeiro caractere para identificar qual a cor */
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
                                printf("Cor inv�lida: %c\n", argv[i + 1][0]);
                            erro_cor:
                                printf("A cor � definida por r, g ou b seguida de um n�mero\n"
                                       "Os par�metros representam vermelho, verde e azul, respectivamente\n"
                                       "Pode-se definir apenas uma, duas ou as tr�s cores\n");
                                erro_param();
                        }
                        i++; /* avan�a pro pr�ximo par�metro */
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
                case 'k':
                    /* verifica se o kernel foi definido */
                    if (argc == i + 9) {
                        printf("N�cleo de convula��o n�o definido!\n");
                        erro_param();
                    }
                    /* l� at� encontrar o pr�ximo par�metro */
                    int celulas_lidas = 0;
                    while (true) {
                        /* verifica se � um n�mero negativo ou um par�metro */
                        if (argv[i + 1][0] == '-' && !isdigit(argv[i + 1][1]))
                            break;
                        if (argv[i + 1][0] == 'x') {
                            printf("O divisor deve ser definido ap�s o n�cleo!\n");
                            erro_param();
                        }
                        kernel[celulas_lidas / 3][celulas_lidas % 3] = atoi(argv[i + 1]); /* NOLINT */
                        celulas_lidas++;
                        i++; /* avan�a pro pr�ximo par�metro */
                        if (argc == i + 1 || celulas_lidas == 9) /* verifica se existem outros argumentos */
                            break;
                    }
                    if (celulas_lidas < 9) {
                        printf("O n�cleo deve ter 9 par�metros!\n");
                        erro_param();
                    }
                    if (argv[i + 1] != NULL)
                        if (argv[i + 1][0] == 'x') {
                            divisor = atoi(&argv[i + 1][1]); /* NOLINT */
                            i++;
                        }
                    k = true;
                    filtros++;
                    break;
                case 't':
                    /* verifica se a porcentagem foi definida */
                    if (argc == i + 1 || testa_param(argv[i + 1])) {
                        if (argv[i + 1][0] == '-' && isdigit(argv[i + 1][1]))
                            goto jump_contrast; /* testa se n�o � um valor negativo */
                        printf("Porcentagem do contraste n�o definida ou inv�lida!\n");
                        erro_param();
                    }
                jump_contrast:
                    contraste = atoi(argv[i + 1]); /* NOLINT */
                    if (abs(contraste) > 100) {
                        printf("Porcentagem deve estar entre -100 e 100!\n");
                        erro_param();
                    }
                    i++; /* pula o pr�ximo argumento (a porcentagem) */
                    t = true;
                    filtros++;
                    break;
                default:
                opcao_invalida:
                    printf("Op��o inv�lida: %s\n", argv[i]);
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

    /* abre o arquivo e define o arquivo de sa�da */
    Imagem *imagem = imagem_carrega(argv[1]);
    if (o)
        printf("O arquivo de sa�da est� definido como %s\n\n", arq_saida);
    else {
        printf("Arquivo de sa�da n�o definido. A imagem ser� sobre-escrita\n\n");
        arq_saida = imagem->nome;
    }

    /* testa os par�metros dos filtros */
    if (u) {
        /*verifica se a �rea n�o � maior que a imagem */
        if (corte_alt > imagem->alt || corte_larg > imagem->larg) {
            printf("�rea de recorte n�o pode ser maior que a imagem!\n");
            erro_param();
        }
        /* verifica se a posi��o � v�lida */
        if (corte_i > imagem->alt || corte_j > imagem->larg) {
            printf("Posi��o de recorte n�o pode ser maior que as dimens�es da imagem!\n");
            erro_param();
        }
    }
    if (d) { /* verifica se a ap�tema informada n�o � maior que a da imagem */
        if (apotema > menor(imagem->alt, imagem->larg) / 2) {
            printf("A ap�tema da �rea de distor��o n�o pode ser maior que a da imagem!\n");
            erro_param();
        }
    }
    if (c) { /* verifica se nenhuma cor � maior que a profundidade de cores da imagem */
        if (abs(cor.r) > imagem->prof_cor || abs(cor.g) > imagem->prof_cor || abs(cor.b) > imagem->prof_cor) {
            printf("As cores definidas n�o podem ser maiores do que a escala de cores da imagem!\n");
            erro_param();
        }
    }

    /* aplica os filtros */
    if (k)
        convulacao(imagem, kernel, divisor);
    if (u)  /* recorta a imagem */
        filtro_corta(imagem, corte_alt, corte_larg, corte_i, corte_j);
    if (b) /* brilho */
        filtro_brilho(imagem, brilho);
    if (t) /* contraste */
        filtro_contraste(imagem, contraste);
    if (c) /* cor */
        filtro_cor(imagem, cor);
    if (n) /* negativo */
        filtro_negativo(imagem);
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

/** MANIPULA��O DO ARQUIVO */
/*<editor-fold desc="file operations">*/
/* recebe um caminho, abre, testa o cabe�alho e faz a leitura dos pixels */
Imagem *imagem_carrega(char *caminho) {
    Imagem *imagem;
    int i, j;
    imagem = malloc(sizeof(Imagem));
    if (imagem == NULL) {
        printf("Mem�ria insuficiente!\n\tLibere mais mem�ria e tente novamente.\n");
        exit(5);
    }
    imagem->arquivo = fopen(caminho, "r");
    if (imagem->arquivo == NULL) {
        printf("Falha ao abrir o arquivo %s", caminho);
        exit(2);
    }
    imagem->nome = cria_string(caminho); /* salva o nome do arquivo */
    /* verifica se � um arquivo v�lido */
    bool arq_valido;
    char tipo[5];
    fgets(tipo, 5, imagem->arquivo);
    arq_valido = !(strlen(tipo) != 3 || tipo[0] != 'P' || !isdigit(tipo[1]));
    if (arq_valido) {
        imagem->tipo = tipo[1];
    } else {
        printf("Formato de arquivo inv�lido\n");
        printf("Tipo: %c%c\t(%d)\n\n", tipo[0], tipo[1], strlen(tipo));
        exit(3);
    }
    /* l� o restante do cabe�alho */
    fscanf(imagem->arquivo, "%d%d%d", &imagem->larg, &imagem->alt, &imagem->prof_cor); /* NOLINT */

    /* aloca e faz a leitura dos pixels */
    imagem->pixels = pixels_aloca(imagem->alt, imagem->larg);
    for (i = 0; i < imagem->alt; i++)
        for (j = 0; j < imagem->larg; j++) /* verifica se os 3 dados foram lidos (retorno da fun��o) */
            if (fscanf(imagem->arquivo, "%d%d%d", &imagem->pixels[i][j].r, &imagem->pixels[i][j].g, /*NOLINT */
                       &imagem->pixels[i][j].b) != 3) {
                printf("Formato de arquivo inv�lido\n"
                       "O arquivo deve ser uma imagem PPM 3 no formato ASCII\n\n");
                exit(3);
            }
    printf("Arquivo %s carregado com sucesso\n", imagem->nome);
    fclose(imagem->arquivo);
    return imagem;
}

/* recebe uma imagem e um caminho de sa�da para salvar */
void imagem_salva(Imagem *imagem, const char *arq_saida) {
    printf("Salvando arquivo %s... ", arq_saida);
    int i, j;
    imagem->arquivo = fopen(arq_saida, "w");
    if (imagem->arquivo == NULL) {
        printf("Erro ao salvar o arquivo\n"
               "Verifique se voc� tem as permiss�es necess�rias");
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
    /* fecha o arquivo e libera a mem�ria alocada */
    fclose(imagem->arquivo);
    pixels_apaga(imagem);
    free(imagem->nome);
    free(imagem);
    printf("salvo\n");
}
/*</editor-fold>*/

/** FILTROS */
/*<editor-fold desc="filters">*/
/* recebem um ponteiro para uma imagem e aplicam um filtro sobre ela */
void filtro_corta(Imagem *imagem, unsigned int alt, unsigned int larg, int corte_i, int corte_j) {
    printf("Recortando imagem no tamanho %dx%d, iniciando na posi��o %d,%d...\n", alt, larg, corte_i, corte_j);
    /* copia o espa�o a ser recortado para uma nova matriz */
    int recorte_i, recorte_j, imagem_j;
    Pixel **recorte = pixels_aloca(alt, larg);
    for (recorte_i = 0; recorte_i < alt && corte_i < imagem->alt; recorte_i++, corte_i++) {
        imagem_j = corte_j;
        for (recorte_j = 0; recorte_j < larg && imagem_j < imagem->larg; recorte_j++, imagem_j++)
            recorte[recorte_i][recorte_j] = imagem->pixels[corte_i][imagem_j];
    }

    /* apaga a matriz original */
    pixels_apaga(imagem);
    /* altera os par�metros da imagem */
    imagem->pixels = recorte;
    imagem->alt = alt;
    imagem->larg = larg;
}

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
    Pixel **pont = pixels_aloca(imagem->alt, imagem->larg);
    int i, j;
    for (i = 0; i < imagem->alt; i++) {
        int k = 0;
        for (j = imagem->larg - 1; j >= 0; j--) {
            pont[i][k] = imagem->pixels[i][j];
            k++;
        }
    }
    pixels_apaga(imagem);
    imagem->pixels = pont;
}

void filtro_virar(Imagem *imagem) {
    printf("Virando imagem...\n");
    Pixel **pont = pixels_aloca(imagem->alt, imagem->larg);
    int i, j;
    int k = 0;
    for (i = imagem->alt - 1; i >= 0; i--) {
        for (j = 0; j < imagem->larg; j++)
            pont[k][j] = imagem->pixels[i][j];
        k++;
    }
    pixels_apaga(imagem);
    imagem->pixels = pont;
}

void filtro_girar(Imagem *imagem, int graus) {
    printf("Girando imagem %d graus...\n", graus);
    Pixel **pont = NULL;
    int i, j;
    if (graus == 180) {
        /* girar 180 graus */
        pont = pixels_aloca(imagem->alt, imagem->larg);
        for (i = 0; i < imagem->alt; i++)
            for (j = 0; j < imagem->larg; j++)
                pont[imagem->alt - i - 1][imagem->larg - j - 1] = imagem->pixels[i][j];
    } else {
        pont = pixels_aloca(imagem->larg, imagem->alt);
        for (i = 0; i < imagem->larg; i++)
            for (j = 0; j < imagem->alt; j++)
                if (graus == 90)
                    /* girar 90 graus */
                    pont[i][imagem->alt - j - 1] = imagem->pixels[j][i];
                else
                    /*girar 270 graus */
                    pont[imagem->larg - i - 1][j] = imagem->pixels[j][i];
        /* redefine as dimens�es da imagem */
        unsigned int aux = imagem->larg;
        imagem->larg = imagem->alt;
        imagem->alt = aux;
    }
    pixels_apaga(imagem);
    imagem->pixels = pont;
}

void filtro_distorcer(Imagem *imagem, int apotema) {
    printf("Aplicando distor��o sobre uma �rea de %d pixels quadrados...", apotema * 2);
    Pixel aux;
    int i, j;
    for (i = 0; i < imagem->alt; i++)
        for (j = 0; j < imagem->larg; ++j) {
            /* busca um pixel aleat�rio dentro da �rea */
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
            /* troca com o pixel da itera��o */
            aux = imagem->pixels[i_aleat][j_aleat];
            imagem->pixels[i_aleat][j_aleat] = imagem->pixels[i][j];
            imagem->pixels[i][j] = aux;
        }
}

void convulacao(Imagem *imagem, int kernel[3][3], int divisor) {
    printf("Aplicando convula��o...");
    int i, j, k, l;
    for (i = 0; i < imagem->alt; i++)
        for (j = 0; j < imagem->larg; j++) {
            int soma_r = 0;
            int soma_g = 0;
            int soma_b = 0;
            for (k = 0; k < 3; k++)
                for (l = 0; l < 3; l++) {
                    if (i == 0 || j == 0 || i == imagem->alt - 1 || j == imagem->larg - 1)
                        continue;
                    int fator_r = imagem->pixels[i][j].r * kernel[k][l];
                    int fator_g = imagem->pixels[i][j].g * kernel[k][l];
                    int fator_b = imagem->pixels[i][j].b * kernel[k][l];
                    soma_r += fator_r;
                    soma_g += fator_g;
                    soma_b += fator_b;
                }
            imagem->pixels[i][j].r = (imagem->pixels[i][j].r + soma_r) / divisor;
            imagem->pixels[i][j].g = (imagem->pixels[i][j].g + soma_g) / divisor;
            imagem->pixels[i][j].b = (imagem->pixels[i][j].b + soma_b) / divisor;
            valida_cores(imagem, i, j);
        }
}
/*</editor-fold>*/

/** FUN��ES SECUND�RIAS */
/*<editor-fold desc="secondary functions">*/
/* recebe as dimens�es e retorna uma matriz de pixels */
Pixel **pixels_aloca(unsigned int alt, unsigned int larg) {
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
void pixels_apaga(Imagem *imagem) {
    int i;
    if (imagem == NULL || imagem->pixels == NULL)
        return;
    for (i = 0; i < imagem->alt - 1; i++)
        free(imagem->pixels[i]);
    free(imagem->pixels);
}

/* testa se o item � um par�metro */
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
    printf("Erro ao alocar espa�o para carregar imagem\nLibere mais mem�ria e tente novamente.\n");
}

/* imprime instru��es e encerra o programa com c�digo 1 */
void erro_param() {
    printf("\nUse [IMAGEM] [OP��ES]\n"
           "O primeiro argumento deve ser o nome do arquivo\n\n");
    printf("As op��es podem ser:\n"
           "\t-o ARQUIVO\tInformar o arquivo de sa�da\n"
           "\t-u ALT LARG Y X\tRecortar  (posi��o inicial de corte, opcional)\n"
           "\t-n\t\tFiltro Negativo\n"
           "\t-b BRILHO\tBrilho    (em porcentagem)\n"
           "\t-t CONTRASTE\tContraste (em porcentagem)\n"
           "\t-v\t\tVirar     (inverter verticalmente)\n"
           "\t-e\t\tEspelhar  (inverter horizontalmente)\n"
           "\t-g GRAUS\tGirar     (90, 180 ou 270 graus)\n"
           "\t-d AP�TEMA\tDistorcer (ap�tema de distor��o, em pixels)\n");
    printf("Convula��o:\n"
           "\t� poss�vel aplicar um n�cleo de convula��o de 3x3\n"
           "\tAp�s o par�metro -k, informe os valores de cada c�lula\n"
           "\tUse x[VALOR] para informar um divisor (opcional)\n");
    exit(1);
}

/* copia uma string para um novo endere�o de mem�ria dinamicamente alocado */
char *cria_string(char *palavra) {
    char *string = malloc(sizeof(char) * strlen(palavra));
    if (string == NULL) {
        printf("Mem�ria insuficiente!\n\tLibere mais mem�ria e tente novamente.\n");
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
