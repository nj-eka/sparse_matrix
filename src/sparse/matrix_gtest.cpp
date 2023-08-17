#include <gtest/gtest.h>

#include <sstream>
#include <algorithm>

#include "matrix.hpp"

using namespace sparse;

TEST(sparse_matrix, create_empty) {
  sparse::Matrix<int, -1> matrix;
  ASSERT_EQ(matrix.size(), 0);
}

TEST(sparse_matrix, check_default_value) {
  constexpr int default_value = -1;
  sparse::Matrix<int, default_value> matrix;
  ASSERT_EQ(matrix[0][0], default_value);
  ASSERT_EQ(matrix[1000][20000], default_value);
  ASSERT_EQ(matrix.size(), 0);
}

TEST(sparse_matrix, assign) {
  sparse::Matrix<int, -1> m;
  m[100][100] = 314;
  ASSERT_EQ(m[100][100], 314);
  ASSERT_EQ(m.size(), 1);
}

TEST(sparse_matrix, default_value_assignmen) {
  constexpr int default_value = -1;
  sparse::Matrix<int, default_value> matrix;
  ASSERT_EQ(matrix.size(), 0);
  matrix[100][100] = 314;
  ASSERT_EQ(matrix.size(), 1);
  matrix[100][100] = default_value;
  ASSERT_EQ(matrix.size(), 0);
}

TEST(sparse_matrix, iteration1) {
  sparse::Matrix<int, -1> matrix;
  matrix[100][200] = 314;
  std::stringstream ss;
  for(auto const& cell: matrix)     
  {         
    auto const& [idx, v] = cell;
    auto const& [x, y] = idx;
    ss << x << ":" << y << "=" << v;
  }
  ASSERT_EQ(ss.str(), "100:200=314");
}

TEST(sparse_matrix, iteration2) {
  sparse::Matrix<int, -1> matrix;
  matrix[10][100] = 11;
  matrix[20][200] = 22;
  matrix[30][300] = 33;
  std::stringstream ss;
  // for (auto const& e: matrix) {
  //   auto const& [x, y, v] = e;
  //   ss << x << y << v << "\n";
  // }
  // ASSERT_EQ(ss.str(), "1010011\n2020022\n3030033\n");
  ss << matrix;
  ASSERT_EQ(ss.str(), "[10,100]=11\n[20,200]=22\n[30,300]=33\n");
  ASSERT_EQ(matrix.size(), 3);
}

TEST(sparse_matrix, reassignment) {
  sparse::Matrix<int> matrix;
  ((matrix[100][100] = 314) = 0) = 217;
  ASSERT_EQ(matrix[100][100], 217);
}

TEST(sparse_matrix, dim1) {
  constexpr int default_value = -1;
  sparse::Matrix<int, default_value, 1> matrix;
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

TEST(sparse_matrix, dim3) {
  constexpr int default_value = -1;
  sparse::Matrix<int, default_value, 3> matrix;
  ASSERT_EQ(matrix[1000][2000][3000], default_value);
  ASSERT_EQ(matrix.size(), 0);
  matrix[1000][2000][3000] = 314;
  ASSERT_EQ(matrix[1000][2000][3000], 314);
  ASSERT_EQ(matrix[1000][2000][30000], default_value);
  ASSERT_EQ(matrix.size(), 1);
  matrix[1000][2000][3000] = default_value;
  ASSERT_EQ(matrix[1000][2000][3000], default_value);
  ASSERT_EQ(matrix.size(), 0);
}