#!/bin/bash

mkdir benchmark_test
cd benchmark_test
wget http://mvapich.cse.ohio-state.edu/download/mvapich/osu-micro-benchmarks-5.6.2.tar.gz
tar -zxvf osu-micro-benchmarks-5.6.2.tar.gz

module load openmpi/3.1.1
cd osu-micro-benchmarks-5.6.2/
./configure CC=mpicc CXX=mpic++

make
