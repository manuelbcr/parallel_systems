#!/bin/sh

echo "Compiling matmul_chapel.chpl..."
chpl matmul_chapel.chpl --fast
echo "finished compiling matmul_chapel"

