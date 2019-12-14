#!/bin/bash/python
import sys 
import numpy as np 
import scipy.sparse 

def sortSecond(val):
  return val[1]

fh_graph = open(sys.argv[2])
fh = open(sys.argv[1])

alphabet = {}

line = fh_graph.readline()
while (line):
  line = line.rstrip()
  split_line = line.split(' ')
  alphabet[split_line[0]] = int(split_line[1])
  line = fh_graph.readline()

data = []

x_interval = 1000
y_interval = 1000

line = fh.readline()
while (line):
  line = line.rstrip()

  split_line = line.split(' ')
  if (split_line[2] == "0") and (split_line[3] == "0"):
    line = fh.readline()
    continue
  name = split_line[0] + "," + split_line[1] + "," +\
         str(int(int(split_line[2])/x_interval)) + "," +\
         str(int(int(split_line[3])/y_interval))


  if name not in alphabet:
    print("Dictionary mismatch problem.")
    exit(1)
  data.append((alphabet[name], int(split_line[4])))
 
  line = fh.readline()

data.sort(key=sortSecond)
fh.close()

fh_w = open("access_pattern_"+sys.argv[1], "w+")
for e in data:
  fh_w.write(str(e[0]) + "\n")
