#!/bin/bash
g++ q3.cpp -std=c++0x -lpthread -o e;
./e < in;
echo "__ comentário:"
echo " Foram feitas algumas melhorias em relação ao que foi sugerido "
echo "  e é possível visualizar o servidor criando as páginas que foram requisitadas,"
echo "  posteriormente o cliente em algum momento lê depois que o servidor as executa"
echo "  e somente depois que o cliente lê o servidor limpa o espaço alocado para a página no buffer."
