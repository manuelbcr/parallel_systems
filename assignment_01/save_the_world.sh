#!/bin/bash

#$ -q std.q
#$ -cwd
#$ -N world_saviors
#$ -o output.dat
#$ -j yes
#$ -pe openmpi-2perhost 8

echo "We are going to save the world!"

module load openmpi/4.0.3
mpiexec -n 8 /bin/hostname

echo "...ok, or at least to execute /bin/hostname"
