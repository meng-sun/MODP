#!/bin/bash/python
import sys 
import numpy as np 
import scipy.sparse 


def sortSecond(val):
  return val[1]

fh = open(sys.argv[1])

data = {}

x_interval = 1000
y_interval = 1000

alphabet = {}
numeric = 0

line = fh.readline()
while (line):
  line = line.rstrip()

  split_line = line.split(' ')
  name = split_line[0] + "," + split_line[1] + "," +\
         str(int(int(split_line[2])/x_interval)) + "," +\
         str(int(int(split_line[3])/y_interval))

  if name not in alphabet:
    alphabet[name] = numeric
    numeric += 1
    data[alphabet[name]] = 0
  data[alphabet[name]] += 1
 
  line = fh.readline()

fh.close()

fh_w = open("dictionary.txt", "w+")
for e in alphabet.keys():
  fh_w.write(e + " " + str(alphabet[e]) + " " + str(data[alphabet[e]]) + "\n")
