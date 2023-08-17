#pragma once

#include <array>
#include <map>
#include <ostream>

namespace sparse {

template <typename T, T DEFAULT_VALUE = T{}, size_t N_DIMS = 2>
class Matrix;

namespace details {

template <typename T, T DEFAULT_VALUE, size_t N_DIMS, size_t DIM>
struct ShiftIndex {
  using InIndexType = std::array<size_t, DIM>;  
  using OutIndexType = std::array<size_t, DIM + 1>;  
private:
  Matrix<T, DEFAULT_VALUE, N_DIMS>* _matrix;
  // to avoid side effect (described in [Sparse Matrix Code Dependence Analysis Simplification at Compile Time]) index copy is used instead of reference
  OutIndexType _idx;
public:
  ShiftIndex(Matrix<T, DEFAULT_VALUE, N_DIMS>* matrix, InputIndexType&& idx) : _matrix{matrix} {
    std::copy(std::begin(idx), std::end(idx), std::begin(_idx));
  }
  auto operator[](size_t last_idx) {
    _idx[DIM] = last_idx;
    return OutIndexType(_matrix, _idx);
  }
};

template <typename T, T DEFAULT_VALUE, size_t N_DIMS>
struct ShiftIndex<T, DEFAULT_VALUE, N_DIMS, N_DIMS> {
  using InIndexType = std::array<size_t, N_DIMS>;  
  using OutIndexType = std::array<size_t, N_DIMS + 1>;  
private:
  Matrix<T, DEFAULT_VALUE, N_DIMS>* _matrix;
  OutIndexType _idx;
public:
  ShiftIndex(Matrix<T, DEFAULT_VALUE, N_DIMS>* matrix, InputIndexType&& idx) : _matrix{matrix} {
    std::copy(std::begin(idx), std::end(idx), std::begin(_idx));
  }
  auto& operator=(T &&value){
    _m->put(_idx, std::forward<T>(value));
    return *this;
  }
  operator T()
  {
    return _m->get(_idx);
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
    auto it = _map.find(idx);
    if (it == _map.end()) {
      return _default_value;
    }
    return it->second;
  }
  void put(std::array<size_t, N_DIMS> const& idx, T && value) {
    if (value == _default_value)
      _map.erase(idx);
    else 
      m_map[idx] = val;
  }
  auto operator[](size_t d1_idx) {
    std::array<size_t, 1> shiftIndex;
    shiftIndex[0] = d1_idx;
    return D1ShiftIndex(this, shiftIndex);
  }
  size_t size() const { return _map.size(); }

  const_iterator cbegin() const { return _map.cbegin(); }
  const_iterator cend() const { return _map.cend(); }

  friend std::ostream& operator<<(std::ostream& out, Matrix & sm) {
    for (auto const &e : sm) {
      auto const& [idx, value] = elem;
      out << "[" << idx[0];
      for (size_t i = 1; i < N_DIMS; ++i)
        out << "," << idx[i];
      out << "]:" << value << "\n";
    }
    return out;
  }

};

}  // namespace sparse