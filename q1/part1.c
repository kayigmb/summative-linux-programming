#include <stdio.h>
#include <stdlib.h>

int global_counter = 5;

int compute_sum(int *arr, int n) {
  int sum = 0;
  for (int i = 0; i < n; i++) {
    sum += arr[i];
  }
  return sum;
}

void fill_array(int *arr, int n) {
  for (int i = 0; i < n; i++) {
    arr[i] = i * 2;
  }
}

void check_value(int val) {
  if (val > 10) {
    printf("Value is greater than 10\n");
  } else {
    printf("Value is 10 or less\n");
  }
}

int main() {
  int n = global_counter;

  int *data = (int *)malloc(n * sizeof(int));

  fill_array(data, n);
  int result = compute_sum(data, n);

  check_value(result);

  printf("Sum = %d\n", result);

  free(data);
  return 0;
}
