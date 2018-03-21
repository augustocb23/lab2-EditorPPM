#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>
#include <ctype.h>

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
 * 
  */

/*estrutura de um arquivo*/
typedef struct img {
    File* arquivo;
    char* nome;
    int alt;
    int larg;
} Imagem;

/*declarações das funções secundárias*/
char* cria_string(char* palavra);

/* função main */
int main( int argc, char *argv[]){
    setlocale (LC_ALL, "portuguese"); /*Define a codificação*/
    
    /* testa os argumentos */
    if (argc == 1){
        /* verifica se o primeiro argumento é um parâmetro (inicia com - ou \) */
		printf("Use %s [SAÍDA] [OPÇÕES]\n", argv[0]);
        if (argv[1] == '-' || argv[1] == '\\') {
            printf("O primeiro argumento deve ser o nome arquivo de saída\n\n")
        } else printf("\n");
        
		printf("As opções podem ser:\n");
		exit(1);
	}

    /* abre o arquivo */
    Imagem imagem*;
    imagem = malloc(sizeof(Imagem));
    if (imagem == NULL){
        printf("Memória insuficiente!\n\tLibere mais memória e tente novamente.\n", argv[1]);
        exit(5);
    }
    imagem->arquivo = fopen(argv[1], w+);
    if (imagem->arquivo == NULL){
        printf("Falha ao abrir o arquivo %d", argv[1]);
        exit(2);
    }
    imagem->nome=cria_string(argv[1]); /* salva o nome do arquivo */
    /* verifica se é um arquivo válido */
    bool arq_valido = true;
    char tipo[5];
    fgets(tipo, 5, imagem->arquivo);
    if (strlen(tipo) != 2 || tipo[0] != 'P' || !isdigit(tipo[1]) )
        arq_valido = false;
    if (!arq_valido){
        printf("Formato de arquivo inválido\n");
        printf("%d\n\n", tipo);
        exit(3);
    }

    //TODO testa as dimensões exit(4)

    return 0;
}


/*copia uma string para um novo endereço de memória dinamicamente alocado*/
char* cria_string(char* palavra){
    char* string = malloc(sizeof(char)*strlen(palavra));
    if (string==NULL){
        printf("Memória insuficiente!\n\tLibere mais memória e tente novamente.\n");
        exit(6);
    }
    strcpy(string,palavra);
    return string;
}