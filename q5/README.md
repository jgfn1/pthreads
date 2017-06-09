# Explicação da forma de entrada

```
 São dados como entradas do programa
	1) DIMENSOES MATRIZ ESPARSA
	2) DIMENSOES MATRIZ DENSA
	3) MATRIZ ESPARSA
	4) VETOR
	5) MATRIZ DENSA
	6) MATRIZ DENSA_2

	O Programa realiza todos os calculos em paralelo e os armazena em uma variável matriz resultante para cada calculo da seguinte forma:
		resultado1 = ESPARSA x DENSA; (feito em paralelo)
		resultado2 = ESPARSA x VETOR; (feito em paralelo)
		resultado3 = ESPARSA x ESPARSA; (feito em paralelo)
    .. join ..
    exibeResultado1;
    exibeResultado2;
    exibeResultado3;

	Eu fiz também um programa na pasta old (q5-old.cpp), que realiza o calculo das multiplicacoes em paralelos mas utiliza uma UNICA MATRIZ RESULTADO;
      Em outras palavras:
      resultado = ESPARSA x DENSA; (feito em paralelo)
      .. join ..
      exibeResultado;
      resultado = ESPARSA x VETOR; (feito em paralelo)
      .. join ..
      exibeResultado;
      resultado = ESPARSA x ESPARSA; (feito em paralelo)
      .. join ..
      exibeResultado;
```
## Linhas x Colunas da matriz esparsa
8 4

## Linhas x Colunas da matriz densa

4 8

## Elementos da matriz esparsa, sendo lidos linha por linha, o primeiro numero indica quantos pares tenho em cada linha, em seguida um indice e valor

  1
      0 1
  1
      1 1
  1
      2 1
  1
      3 1
  3
      1 1
      3 1
      0 1
  1
      0 1
  2
      2 1
      3 1
  1
      2 1

## Elementos do vetor (veja que deve conter o mesmo numero de linhas da matriz esparsa)

1 2 3 4 5 6 7 8 9

## Matriz densa

2    -1    0    2    -1    0    3   6
14    2    -1   2     7    8    1   2
0    4    2     3     5    3    4   9
3    0    -1    1    16   12    1   0

## Segunda matriz densa, sendo lida de forma analoga a que foi lida a matriz anterior

2
    0 1
    2 1
1
    1 1
1
    2 1
1
    3 1
3
    1 1
    3 1
    0 1
1
    0 1
2
    2 1
    3 1
1
    2 1
