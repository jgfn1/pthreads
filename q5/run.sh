#!/bin/bash
echo "Temos dois casos de testes qual deseja testar?";
echo "1) In ";
echo "2) In-big";

g++ q5.cpp -std=c++0x -lpthread -o e;

read NUM
case $NUM in
	1) ./e < in ;;
  2) ./e < in-big ;;
  *) echo "Entrada inválida";

esac