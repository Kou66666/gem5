#include <stdint.h>
#include <stdio.h>

/**
 * This file provides stub implementations of RISC-V assembly functions
 * for x86 compilation, allowing the program to compile and run on x86
 * systems without the actual RISC-V assembly code.
 */

/**
 * Stub implementation of the matmul_i8_rvm function for x86 compilation.
 * This simply calls the C implementation to perform matrix multiplication
 * with 32-bit accumulation.
 *
 * @param a Input matrix A (m×k) with int8_t elements
 * @param b Input matrix B (k×n) with int8_t elements
 * @param c Output matrix C (m×n) with int32_t elements
 * @param m Number of rows in matrix A and matrix C
 * @param n Number of columns in matrix B and matrix C
 * @param k Number of columns in matrix A and rows in matrix B
 */
void matmul_i8_rvm(int8_t *a, int8_t *b, int32_t *c, int m, int n, int k) {
    printf("Using C implementation instead of RISC-V assembly on x86\n");

    // Initialize the C matrix with zeros first
    for (int i = 0; i < m * n; i++) {
        c[i] = 0;
    }

    // Perform the matrix multiplication with 32-bit accumulation
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int p = 0; p < k; ++p) {
                // Cast to int32_t before multiplication to ensure proper 32-bit accumulation
                c[i * n + j] += (int32_t)a[i * k + p] * (int32_t)b[p * n + j];
            }
        }
    }
}
