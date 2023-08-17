#pragma once

#include <array>
#include <map>
#include <ostream>
#include <utility>

// #include <iostream>
// #define LOG (std::cout << __PRETTY_FUNCTION__ << std::endl)

namespace sparse {

template <typename T, size_t N_DIMS = 2>
class Matrix;

namespace details {

template <typename T, size_t N_DIMS, size_t DIM>
class ShiftIndex {
  Matrix<T, N_DIMS>* _matrix;
  // todo: try to optimize the index transfer while avoiding some side effects (described in [Sparse Matrix Code Dependence Analysis Simplification at Compile Time])
  std::array<size_t, DIM + 1> _idx;
public:
  ShiftIndex(Matrix<T, N_DIMS>* matrix, std::array<size_t, DIM> idx) : _matrix{matrix} {
    std::copy(std::begin(idx), std::end(idx), std::begin(_idx));
  }
  auto operator[](size_t last_idx) {
    _idx[DIM] = last_idx;
    return ShiftIndex<T, N_DIMS, DIM+1>(_matrix, _idx);
  }
};

template <typename T, size_t N_DIMS>
class ShiftIndex<T, N_DIMS, N_DIMS> {
  Matrix<T, N_DIMS>* _matrix;
  std::array<size_t, N_DIMS> _idx;
public:
  ShiftIndex(Matrix<T, N_DIMS>* matrix, std::array<size_t, N_DIMS> idx) : _matrix{matrix} {
    std::copy(std::begin(idx), std::end(idx), std::begin(_idx));
    // LOG;
  }
  auto& operator=(T &&value){
    // LOG;
    _matrix->put(_idx, std::forward<T>(value));
    return *this;
  }
  auto& operator=(T const& value){
    // LOG;
    _matrix->put(_idx, value);
    return *this;
  }
  operator T() const
  {
    // LOG;
    return _matrix->get(_idx);
  }
};

} // namespace details

template <typename T, size_t N_DIMS>
class Matrix {
  std::map<std::array<size_t, N_DIMS>, T> _map;
  T const _default;
 public:
  using const_iterator = typename std::map<std::array<size_t, N_DIMS>, T>::const_iterator;
  using ZeroShiftIndex = details::ShiftIndex<T, N_DIMS, 1>;

  Matrix(const T& default_value = T{}): _default{default_value} {};

  T const& get(std::array<size_t, N_DIMS> const& idx) const {
    auto const& it = _map.find(idx);
    if (it == _map.end()) {
      return _default;
    }
    return it->second;
  }
  void put(std::array<size_t, N_DIMS> const& idx, T &&value) {
    if (value == _default){
      _map.erase(idx);
    } else {
      // _map[idx] = value; // + 1) T::T(&&) + 2) T& T::operator=(const T&)
      // _map.emplace(idx, std::forward<T>(value));  // + 1) T::T(&&) + 2) T& T::operator=(const T&)
      // _map.emplace(std::piecewise_construct, std::forward_as_tuple(idx), std::forward_as_tuple(value)); // T::T(const T&)
      // _map.try_emplace(idx, value); // T::T(const T&)
      _map.insert_or_assign(idx, std::forward<T>(value)); // insert = 1) T::T(&&) + 2) T& T::operator=(const T&) assign = T& T::operator=(const T&) + T::T(const T&)
    }
  }
  void put(std::array<size_t, N_DIMS> const& idx, const T &value) {
    // LOG;
    if (value == _default)
      _map.erase(idx);
    else
      _map[idx] = value;
  }  
  auto operator[](size_t idx1) {
    std::array<size_t, 1> shiftIndex;
    shiftIndex[0] = idx1;
    return ZeroShiftIndex(this, shiftIndex);
  }
  size_t size() const { return _map.size(); }

  const_iterator begin() const { return _map.cbegin(); }
  const_iterator end() const { return _map.cend(); }

  friend std::ostream& operator<<(std::ostream& out, Matrix const & matrix) {
    for (auto const &cell : matrix) {
      auto const& [idx, value] = cell;
      out << "[" << idx[0];
      for (size_t i = 1; i < N_DIMS; ++i)
        out << "," << idx[i];
      out << "]=" << value << "\n";
    }
    return out;
  }

};

}  // namespace sparse