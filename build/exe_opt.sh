#!/bin/bash
for i in {0..22} ; do
for j in {0..22} ; do
for k in {0..22} ; do ./simulate "$i $j $k" ; done; done; done
