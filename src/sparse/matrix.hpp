#pragma once

#include <array>
#include <map>
#include <ostream>
#include <utility>

namespace sparse {

template <typename T, T DEFAULT_VALUE = T{}, size_t N_DIMS = 2>
class Matrix;

namespace details {

template <typename T, T DEFAULT_VALUE, size_t N_DIMS, size_t DIM>
class ShiftIndex {
  Matrix<T, DEFAULT_VALUE, N_DIMS>* _matrix;
  // to avoid side effect (described in [Sparse Matrix Code Dependence Analysis Simplification at Compile Time]) index copy is used instead of reference
  std::array<size_t, DIM + 1> _idx;
public:
  ShiftIndex(Matrix<T, DEFAULT_VALUE, N_DIMS>* matrix, std::array<size_t, DIM> idx) : _matrix{matrix} {
    std::copy(std::begin(idx), std::end(idx), std::begin(_idx));
  }
  auto operator[](size_t last_idx) {
    _idx[DIM] = last_idx;
    return ShiftIndex<T, DEFAULT_VALUE, N_DIMS, DIM+1>(_matrix, _idx);
  }
};

template <typename T, T DEFAULT_VALUE, size_t N_DIMS>
class ShiftIndex<T, DEFAULT_VALUE, N_DIMS, N_DIMS> {
  Matrix<T, DEFAULT_VALUE, N_DIMS>* _matrix;
  std::array<size_t, N_DIMS> _idx;
public:
  ShiftIndex(Matrix<T, DEFAULT_VALUE, N_DIMS>* matrix, std::array<size_t, N_DIMS> idx) : _matrix{matrix} {
    std::copy(std::begin(idx), std::end(idx), std::begin(_idx));
  }
  auto& operator=(T value){
    _matrix->put(_idx, value);
    return *this;
  }
  operator T() const
  {
    return _matrix->get(_idx);
  }
};

} // namespace details

template <typename T, T DEFAULT_VALUE, size_t N_DIMS>
class Matrix {
  std::map<std::array<size_t, N_DIMS>, T> _map;
  T _default_value = DEFAULT_VALUE;
 public:
  using const_iterator = typename std::map<std::array<size_t, N_DIMS>, T>::const_iterator;
  using D1ShiftIndex = details::ShiftIndex<T, DEFAULT_VALUE, N_DIMS, 1>;

  Matrix() = default;
  ~Matrix() = default;
  T const& get(std::array<size_t, N_DIMS> const& idx) const {
    auto const& it = _map.find(idx);
    if (it == _map.end()) {
      return _default_value;
    }
    return it->second;
  }
  void put(std::array<size_t, N_DIMS> const& idx, T value) {
    if (value == _default_value)
      _map.erase(idx);
    else
      _map[idx] = value;
  }
  auto operator[](size_t idx1) {
    std::array<size_t, 1> shiftIndex;
    shiftIndex[0] = idx1;
    return D1ShiftIndex(this, shiftIndex);
  }
  size_t size() const { return _map.size(); }

  const_iterator begin() const { return _map.cbegin(); }
  const_iterator end() const { return _map.cend(); }

  friend std::ostream& operator<<(std::ostream& out, Matrix & matrix) {
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