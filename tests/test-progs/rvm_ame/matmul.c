#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern void matmul_i8_rvm(int8_t *a, int8_t *b, int32_t *c, int m, int n, int k);

void matmul_i8_c(int8_t *a, int8_t *b, int32_t *c, int m, int n, int k) {

  printf("aaaaaaaaaaaa %6d bbbbbbbbbbbbb\n", c[0]);  // Wider field for int32

  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      for (int p = 0; p < k; ++p) {
        int8_t a_val = a[i * k + p];
        int8_t b_val = b[p * n + j];
        int32_t product = (int32_t)a_val * (int32_t)b_val;

        c[i * n + j] += product;
      }
    }
  }
}

// Function to run a single test with specified dimensions
int run_test(int m, int n, int k) {
  printf("\n===== Testing matrix multiplication with m=%d, n=%d, k=%d =====\n", m, n, k);

  // Allocate matrices dynamically
  int a_size = m * k;
  int b_size = k * n;
  int c_size = m * n;

  int8_t *a = (int8_t*)malloc(a_size * sizeof(int8_t));
  int8_t *b = (int8_t*)malloc(b_size * sizeof(int8_t));
  int32_t *c1 = (int32_t*)malloc(c_size * sizeof(int32_t));
  int32_t *c2 = (int32_t*)malloc(c_size * sizeof(int32_t));

  if (!a || !b || !c1 || !c2) {
    printf("Memory allocation failed\n");
    return -1;
  }

  // Initialize with random int8_t values (-128 to 127)
  for (int i = 0; i < a_size; i++) {
    a[i] = (int8_t)((rand() % 256) - 128); // Random values from -128 to 127
  }

  for (int i = 0; i < b_size; i++) {
     b[i] = (int8_t)((rand() % 256) - 128); // Random values from -128 to 127
  }

  // Initialize c1 and c2 with zeros
  for (int i = 0; i < c_size; i++) {
    c1[i] = 0;
    c2[i] = 0;
  }

//   // Print matrix A
//   printf("Matrix A (%dx%d):\n", m, k);
//   for (int i = 0; i < m; i++) {
//     for (int j = 0; j < k; j++) {
//       printf("%3d ", a[i * k + j]);
//     }
//     printf("\n");
//   }
//
//   // Print matrix B
//   printf("Matrix B (%dx%d):\n", k, n);
//   for (int i = 0; i < k; i++) {
//     for (int j = 0; j < n; j++) {
//       printf("%3d ", b[i * n + j]);
//     }
//     printf("\n");
//   }

  // Run both implementations
  matmul_i8_rvm(a, b, c1, m, n, k);
  matmul_i8_c(a, b, c2, m, n, k);

 //  // Print results
 //  printf("c1 matrix (RISC-V assembly):\n");
 //  for (int i = 0; i < m; ++i) {
 //    for (int j = 0; j < n; ++j) {
 //      printf("%6d ", c1[i * n + j]);  // Wider field for int32
 //    }
 //    printf("\n");
 //  }

 //  printf("c2 matrix (C implementation):\n");
 //  for (int i = 0; i < m; ++i) {
 //    for (int j = 0; j < n; ++j) {
 //      printf("%6d ", c2[i * n + j]);  // Wider field for int32
 //    }
 //    printf("\n");
 //  }

  // Check for mismatches
  int result = 0;
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      if (c1[i * n + j] != c2[i * n + j]) {
        printf("Mismatch at (%d, %d): %d != %d\n", i, j, c1[i * n + j], c2[i * n + j]);
        result = 1;
      }
    }
  }

  if (result == 0) {
    printf("Test passed: All results match!\n");
  }

  // Free allocated memory
  free(a);
  free(b);
  free(c1);
  free(c2);

  return result;
}

int main() {
  // Use a fixed seed for reproducible random numbers
  unsigned int fixed_seed = 12346;  // You can change this to any value you want
  printf("Using fixed random seed: %u\n", fixed_seed);
  srand(fixed_seed);

  // Define test cases with different dimensions
  typedef struct {
    int m, n, k;
  } TestCase;

  TestCase test_cases[] = {
    {2, 3, 2},    // Original test case
    {4, 4, 4},    // Square matrices
    {3, 5, 2},    // Rectangular matrices
    {5, 2, 4},    // Different dimensions
    {8, 8, 8},    // Larger matrices
    {1, 1, 1},    // Minimal case
    {2, 2, 2},    // Small square
    {4, 8, 16},   // Powers of two
    {16, 8, 4},   // Powers of two, reverse order
    {3, 7, 11},   // Prime numbers
    {10, 20, 30}, // Multiples of 10
    {15, 15, 15}, // Medium square
    {32, 32, 32}, // Power of 2 square
    {64, 32, 16}, // Powers of 2 descending
    {16, 32, 64}, // Powers of 2 ascending
    {7, 13, 23},  // Prime numbers
    {9, 27, 45},  // Multiples of 9
    {12, 24, 48}, // Multiples of 12
    {5, 25, 50},  // Multiples of 5
    {31, 47, 59}, // Large prime-like numbers
    {6, 18, 36},  // Multiples of 6
    {20, 30, 40}, // Increments of 10
    {33, 33, 33}, // Repeating odd number
    {44, 44, 44}, // Repeating even number
    {17, 34, 51}, // Multiples of 17
    {13, 26, 52}, // Multiples of 13
    {11, 22, 44}, // Multiples of 11
    {29, 37, 43}, // Prime numbers
    {27, 36, 45}, // Perfect squares sequence
    {50, 40, 30}, // Decreasing sequence
    {38, 42, 46}, // Increments of 4
    {64, 64, 64}  // Maximum size
  };

  int num_tests = sizeof(test_cases) / sizeof(TestCase);
  int failed_tests = 0;

  printf("Running %d matrix multiplication tests...\n", num_tests);

  // Run all test cases
  for (int i = 0; i < num_tests; i++) {
    int m = test_cases[i].m;
    int n = test_cases[i].n;
    int k = test_cases[i].k;

    int result = run_test(m, n, k);
    if (result != 0) {
      failed_tests++;
    }

    printf("\n");
  }

  // Print summary
  printf("===== Test Summary =====\n");
  printf("Total tests: %d\n", num_tests);
  printf("Passed: %d\n", num_tests - failed_tests);
  printf("Failed: %d\n", failed_tests);

  return failed_tests > 0 ? 1 : 0;
}
