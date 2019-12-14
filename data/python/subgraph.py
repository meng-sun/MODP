#!/bin/bash/python
import sys 
import numpy as np 
import scipy.sparse 

def sortSecond(val):
  return val[1]

fh_graph = open(sys.argv[2])

alphabet = {}

line = fh_graph.readline()
while (line):
  line = line.rstrip()
  split_line = line.split(' ')
  alphabet[split_line[0]] = int(split_line[1])
  line = fh_graph.readline()

fh_graph.close()

fh = open(sys.argv[1])

subgraph_dictionary = {}

x_interval = 1000
y_interval = 1000

line = fh.readline()
while (line):
  line = line.rstrip()

  split_line = line.split(' ')
  name = split_line[0] + "," + split_line[1] + "," +\
         str(int(int(split_line[2])/x_interval)) + "," +\
         str(int(int(split_line[3])/y_interval))

  if name not in alphabet:
    print("Dictionary mismatch problem.")
    exit(1)
  if alphabet[name] not in subgraph_dictionary:
    subgraph_dictionary[alphabet[name]] = ''
  subgraph_dictionary[alphabet[name]] += \
    line + "\n";
 
  line = fh.readline()

fh.close()

for e in subgraph_dictionary.keys():
  txt_file = str(e) + "_subgraph.txt"
  fh_w = open(txt_file, "w+")
  fh_w.write(subgraph_dictionary[e])
  fh_w.close()
