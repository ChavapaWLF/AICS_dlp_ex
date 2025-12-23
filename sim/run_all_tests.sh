#!/bin/bash

cd "$(dirname "$0")"

echo "========== Deep Learning Processor Test Report =========="
echo ""

# Compile
make clean > /dev/null 2>&1
make all > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "Compilation FAILED"
    exit 1
fi

# Test counters
total=0
passed=0

# Test Inner Product
echo "[1] Inner Product (4x16-bit):"
for i in 1 2 3 4; do
    total=$((total + 1))
    echo -n "  Test$i: "
    ./obj_dir/inner_product_4x16/Vinner_product_4x16 ../data/test$i 2>/dev/null
    if [ $? -eq 0 ]; then
        passed=$((passed + 1))
    fi
done
echo ""

# Test Matrix-Vector
echo "[2] Matrix-Vector Mult (4x4x16-bit):"
for i in 1 2 3 4; do
    total=$((total + 1))
    echo -n "  Test$i: "
    ./obj_dir/matrix_vector_mult_4x4x16/Vmatrix_vector_mult_4x4x16 ../data/test$i 2>/dev/null
    if [ $? -eq 0 ]; then
        passed=$((passed + 1))
    fi
done
echo ""

# Test Matrix-Matrix
echo "[3] Matrix-Matrix Mult (4x4x4x16-bit):"
for i in 1 2 3 4; do
    total=$((total + 1))
    echo -n "  Test$i: "
    ./obj_dir/matrix_mult_4x4x4x16/Vmatrix_mult_4x4x4x16 ../data/test$i 2>/dev/null
    if [ $? -eq 0 ]; then
        passed=$((passed + 1))
    fi
done
echo ""

# Summary
echo "=========================================================="
echo "Result: $passed/$total tests passed"
if [ $passed -eq $total ]; then
    echo "Status: ALL PASS"
    exit 0
else
    echo "Status: SOME FAILED"
    exit 1
fi
