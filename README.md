# Laboratório de Programação II - 2018/1
### Tarefa 1 - Vetores/Matrizes

## Descrição:
Você deverá programar um sistema capaz de manipular uma imagem em formato PPM (variante P3) e realizar operações básicas de aplicação de filtros de imagem à mesma.

Seu sistema deverá ser capaz de aplicar ao menos 8 filtros diferentes à imagem: ao menos 4 de cor (ex.: negativo, ajuste de canal de cor, brilho, contraste, etc) e ao menos 4 posicionais (ex.: flip, mirror, rotação, distorção, etc.). Além disso, também deverá permitir o uso da técnica de matriz de convolução, para um núcleo de, no mínimo, 3x3. O usuário deverá poder preencher o núcleo de convolução livremente. Tanto para os filtros quanto para a convolução, o usuário deverá ser capaz de, opcionalmente, indicar um recorte/seleção da imagem para sua aplicação.

Você deverá enviar o código C e um arquivo leiame.txt, que deverá conter uma descrição de suas escolhas de filtros, como cada um funciona internamente e como utilizá-los. Seu código será avaliado quanto à funcionalidade, correção, legibilidade e documentação.

OBS.: Seu código será testado em máquina com linux, utilizando compilador C padrão ANSI (gcc -ansi)
OBS. 2: Seu código será verificado contra plágio. Em caso positivo, o mesmo será desqualificado e a nota zerada.

- Data de entrega do código: 06/04/18, via e-mail para `candia@inf.ufsm.br`
- Número de participantes: **Individual**

Ex.: Formato PPM (variante P3)

    P3
    4 4
    255
    0  0  0   100 0  0       0  0  0    255   0 255
    0  0  0    0 255 175     0  0  0     0    0  0
    0  0  0    0  0  0       0 15 175    0    0  0
    255 0 255  0  0  0       0  0  0    255  255 255  


No formato PPM/P3, a primeira linha do arquivo contém a string “P3”, a segunda linha contém informação de número de pixels por linha e coluna, a terceira linha contém o valor máximo de cor por canal, e da quarta linha em diante os valores de cor por pixel, sempre na ordem RGB (vermelho, verde, azul).
Seu programa será testado com arquivos:
- estritamente neste formato, NÃO serão utilizados arquivos com comentários (linhas iniciando com ‘#’) para os testes e avaliação;
- os arquivos de testes serão quadrados (NN x NN) e retangulares (MM x NN).
