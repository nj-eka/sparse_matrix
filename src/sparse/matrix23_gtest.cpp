#include <gtest/gtest.h>

#include <sstream>
#include <algorithm>

#include "matrix23.hpp"

using namespace sparse::std23;

TEST(sparse_matrix23, create_empty) {
  Matrix<int> matrix;
  ASSERT_EQ(matrix.size(), 0);
}

TEST(sparse_matrix23, check_default_value) {
  constexpr int default_value = -1;
  Matrix<int> matrix(default_value);
  // ошибка: в макрос «ASSERT_EQ» передано 3 аргументов, но используется только 2 ASSERT_EQ(matrix[0, 0], default_value);
  ASSERT_EQ((matrix[0, 0]), default_value);
  ASSERT_EQ((matrix[1000, 20000]), default_value);
  ASSERT_EQ(matrix.size(), 0);
}

TEST(sparse_matrix23, assign) {
  Matrix<int> m;
  m[100, 100] = 314;
  ASSERT_EQ((m[100, 100]), 314);
  ASSERT_EQ(m.size(), 1);
}

TEST(sparse_matrix23, default_value_assignmen) {
  constexpr int default_value = -1;
  Matrix<int> matrix(default_value);
  ASSERT_EQ(matrix.size(), 0);
  matrix[100, 100] = 314;
  ASSERT_EQ(matrix.size(), 1);
  matrix[100, 100] = default_value;
  ASSERT_EQ(matrix.size(), 0);
}

TEST(sparse_matrix23, iteration1) {
  Matrix<int> matrix;
  matrix[100, 200] = 314;
  std::stringstream ss;
  for(auto const& cell: matrix)     
  {         
    auto const& [idx, v] = cell;
    auto const& [x, y] = idx;
    ss << x << ":" << y << "=" << v;
  }
  ASSERT_EQ(ss.str(), "100:200=314");
}

TEST(sparse_matrix23, iteration2) {
  Matrix<int> matrix;
  matrix[10, 100] = 11;
  matrix[20, 200] = 22;
  matrix[30, 300] = 33;
  std::stringstream ss;
  ss << matrix;
  ASSERT_EQ(ss.str(), "[10,100]=11\n[20,200]=22\n[30,300]=33\n");
  ASSERT_EQ(matrix.size(), 3);
}

TEST(sparse_matrix23, reassignment) {
  Matrix<int> matrix;
  ((matrix[100, 100] = 314) = 0) = 217;
  ASSERT_EQ((matrix[100, 100]), 217);
}

TEST(sparse_matrix23, dim1) {
  constexpr int default_value = -1;
  Matrix<int, 1> matrix(default_value);
  ASSERT_EQ(matrix[1000], default_value);
  ASSERT_EQ(matrix.size(), 0);
  matrix[100] = 314;
  ASSERT_EQ(matrix[100], 314);
  ASSERT_EQ(matrix[1000], default_value);
  ASSERT_EQ(matrix.size(), 1);
  matrix[100] = default_value;
  ASSERT_EQ(matrix[100], default_value);
  ASSERT_EQ(matrix.size(), 0);
}

TEST(sparse_matrix23, dim3) {
  constexpr int default_value = -1;
  Matrix<int, 3> matrix(default_value);
  ASSERT_EQ((matrix[1000, 2000, 3000]), default_value);
  ASSERT_EQ(matrix.size(), 0);
  matrix[1000, 2000, 3000] = 314;
  ASSERT_EQ((matrix[1000, 2000, 3000]), 314);
  ASSERT_EQ((matrix[1000, 2000, 30000]), default_value);
  ASSERT_EQ(matrix.size(), 1);
  matrix[1000, 2000, 3000] = default_value;
  ASSERT_EQ((matrix[1000, 2000, 3000]), default_value);
  ASSERT_EQ(matrix.size(), 0);
}
