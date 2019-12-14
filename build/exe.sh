#!/bin/bash
for i in {0..22} ; do
for j in {0..22} ; do
for k in {0..22} ; do ./generalize "bfs" "$i $j $k" ; done; done; done
