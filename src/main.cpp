#include <iostream>

#include "sparse/matrix.hpp"

int main() {
  sparse::Matrix<int, 0> sm;
  constexpr int n = 10;
  for (int i = 0; i < n; ++i) {
    sm[i][i] = i;
    sm[n-1-i][i] = i;
  }
  for (int i = 1; i <= 8; ++i) {
    for (int j = 1; j <= 8; ++j) {
      if (j > 1) std::cout << " ";
      std::cout << sm[i][j];
    }
    std::cout << std::endl;
  }
  std::cout << "size = " << sm.size() << std::endl;
  std::cout << sm;

  return EXIT_SUCCESS;;
}
