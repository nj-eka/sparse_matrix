#include <iostream>

#include "sparse/matrix.hpp"

/**
 * @brief hw-06
 *
 * [task](06_homework.pdf)
 *
 * ![Sequence diagram](puml/sequence_diagram.svg)
 */
int main() {
  sparse::Matrix<int> matrix;
  constexpr int n = 10;
  for (int i = 0; i < n; ++i) {
    matrix[i][i] = i;
    matrix[n - 1 - i][i] = i;
  }
  for (int i = 1; i <= 8; ++i) {
    for (int j = 1; j <= 8; ++j) {
      if (j > 1) std::cout << " ";
      std::cout << matrix[i][j];
    }
    std::cout << std::endl;
  }
  std::cout << "size = " << matrix.size() << std::endl;
  std::cout << matrix;

  return EXIT_SUCCESS;
}
