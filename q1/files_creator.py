#!/usr/bin/python

from random import randint
for i in range(1,11):
	file = open(str(i) + ".in", "w");
	for j in range(1,11):
		file.write(str(randint(1,10)) + '\n')