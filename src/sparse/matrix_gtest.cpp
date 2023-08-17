#include <gtest/gtest.h>

#include <sstream>
#include <algorithm>

#include "matrix.hpp"

using namespace sparse;

TEST(sparse_matrix, create_empty) {
  sparse::Matrix<int, -1> sm;
  ASSERT_EQ(sm.size(), 0);
}

TEST(sparse_matrix, check_default_value) {
  sparse::Matrix<int, -1> sm;
  ASSERT_EQ(-1, sm[0, 0]);
  ASSERT_EQ(-1, sm[1000, 20000]);
  ASSERT_EQ(sm.size(), 0);
}

TEST(sparse_matrix, assign) {
  sparse::Matrix<int, -1> m;
  m[100][100] = 314;
  ASSERT_EQ(m[100][100], 314);
  ASSERT_EQ(m.size(), 1);
}

TEST(sparse_matrix, default_value_assignmen) {
  constexpr int default_value = -1;
  sparse::Matrix<int, default_value> sm;
  ASSERT_EQ(sm.size(), 0);
  sm[100][100] = 314;
  ASSERT_EQ(sm.size(), 1);
  sm[100][100] = default_value;
  ASSERT_EQ(sm.size(), 0);
}

TEST(sparse_matrix, iteration1) {
  sparse::Matrix<int, -1> sm;
  sm[100][200] = 314;
  std::stringstream ss;
  for(auto const& e: sm)     
  {         
    int x, y, v;
    std::tie(x, y, v) = e;
    ss << x << ":" << y << "=" << v;
  }
  ASSERT_EQ(ss.str(), "100:200=314")     
}

TEST(sparse_matrix, iteration2) {
  sparse::Matrix<int, -1> sm;
  sm[10][100] = 11;
  sm[20][200] = 22;
  sm[30][300] = 33;
  std::stringstream ss;
  // for (auto const& e: sm) {
  //   auto const& [x, y, v] = e;
  //   ss << x << y << v << "\n";
  // }
  // ASSERT_EQ(ss.str(), "1010011\n2020022\n3030033\n");
  ss << sm;
  ASSERT_EQ(ss.str(), "[10,100]=11\n[20:200]=22\n[30:300]=33\n");
  ASSERT_EQ(sm.size(), 3);
}

TEST(sparse_matrix, iteration2) {
  sparse::Matrix<int> sm;
  ((sm[100][100] = 314) = 0) = 217;
  ASSERT_EQ(sm[100][100], 217);
}

TEST(sparse_matrix, dim1) {
  constexpr int default_value = -1
  sparse::Matrix<int, default_value, 1> sm;
  ASSERT_EQ(sm[1000], default_value);
  ASSERT_EQ(m.size(), 0);
  sm[100] = 314;
  ASSERT_EQ(sm[100], 314);
  ASSERT_EQ(sm[1000], default_value);
  ASSERT_EQ(sm.size(), 1);
  sm[100] = default_value;
  ASSERT_EQ(sm[100], default_value);
  ASSERT_EQ(0, m.size());
}

TEST(sparse_matrix, dim3) {
  constexpr int default_value = -1
  sparse::Matrix<int, default_value, 3> sm;
  ASSERT_EQ(sm[1000][2000][3000], default_value);
  ASSERT_EQ(m.size(), 0);
  sm[1000][2000][3000] = 314;
  ASSERT_EQ(sm[1000][2000][3000], 314);
  ASSERT_EQ(sm[10000], default_value);
  ASSERT_EQ(sm.size(), 1);
  sm[1000][2000][3000] = default_value;
  ASSERT_EQ(sm[1000][2000][3000], default_value);
  ASSERT_EQ(0, m.size());
}