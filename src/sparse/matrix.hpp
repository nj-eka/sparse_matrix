#pragma once

#include <array>
#include <concepts>
#include <map>
#include <ostream>
#include <utility>
#include <type_traits>

#include "utils/logging.hpp"

namespace sparse {

template <std::copyable T, size_t N_DIMS = 2>
struct Matrix;

namespace details {

template <size_t N_DIMS>
using IndexType = std::array<size_t, N_DIMS>;

template <std::copyable T, size_t N_DIMS>
struct CellAccessor {
  virtual T const& get(IndexType<N_DIMS> const&) const noexcept = 0;
  // noexcept(noexcept(T(value)) && noexcept(operator=(std::decay<T>(value)))) = 0;
  virtual void set(IndexType<N_DIMS> const& idx, const T& value) noexcept(noexcept(T(value))) = 0;  
  virtual void set(IndexType<N_DIMS> const& idx, T&& value) noexcept(noexcept(T(std::forward<T>(value)))) = 0; 
  //noexcept(noexcept(T(std::forward<T>(value))) && noexcept(::operator=(std::forward<T>(value)))) = 0;
};

template <std::copyable T, size_t N_DIMS, size_t DIM>
class ShiftIndex {
  static_assert(N_DIMS > 0, "N_DIMS must be > 0");  
  static_assert(DIM < N_DIMS, "DIM out of declared range (N_DIMS)");

  ShiftIndex(ShiftIndex const&) = delete;
  ShiftIndex& operator=(ShiftIndex const&) = delete;

  CellAccessor<T, N_DIMS>* _cell;
  IndexType<DIM + 1> _idx;

  ShiftIndex(CellAccessor<T, N_DIMS>* cell, IndexType<DIM> idx): _cell{cell} {
    std::copy(std::begin(idx), std::end(idx), std::begin(_idx)); // noexcept(false)
  }

 public:
  auto operator[](size_t last_idx) {
    _idx[DIM] = last_idx;
    return ShiftIndex<T, N_DIMS, DIM + 1>(_cell, _idx);
  }
  friend struct ShiftIndex<T, N_DIMS, DIM - 1>;
  friend struct sparse::Matrix<T, N_DIMS>;
};

template <std::copyable T, size_t N_DIMS>
class ShiftIndex<T, N_DIMS, N_DIMS> {
  static_assert(N_DIMS > 0, "N_DIMS must be > 0");  

  ShiftIndex(ShiftIndex const&) = delete;
  ShiftIndex& operator=(ShiftIndex const&) = delete;

  CellAccessor<T, N_DIMS>* _cell;
  std::array<size_t, N_DIMS> const _idx;

  ShiftIndex(CellAccessor<T, N_DIMS>* cell, IndexType<N_DIMS> const& idx)
      : _cell{cell}, _idx{idx} {}

 public:
  operator T() const noexcept { return _cell->get(_idx); }
  auto& operator=(T&& value) noexcept(noexcept(_cell->set(_idx, std::forward<T>(value)))) {
    _cell->set(_idx, std::forward<T>(value));
    return *this;
  }
  auto& operator=(T const& value) noexcept(noexcept(_cell->set(_idx, value))) {
    _cell->set(_idx, value);
    return *this;
  }

  friend struct ShiftIndex<T, N_DIMS, N_DIMS - 1>;
  friend struct sparse::Matrix<T, N_DIMS>;
};

}  // namespace details

template <std::copyable T, size_t N_DIMS>
struct Matrix final : details::CellAccessor<T, N_DIMS> {
  static_assert(N_DIMS > 0, "N_DIMS must be > 0"); 

  using IndexType = details::IndexType<N_DIMS>;
  using HeadIndexType = details::IndexType<1>;
  using HeadShiftIndex = details::ShiftIndex<T, N_DIMS, 1>;
  using const_iterator = typename std::map<IndexType, T>::const_iterator;

 private:
  std::map<IndexType, T> _map; // matrix elements should be ordered by task
  T const _default;

 public:
  Matrix(T const & default_value = T{}) noexcept(noexcept(T(default_value))) : _default{default_value} {}
  Matrix(T && default_value) noexcept(noexcept(T(std::forward<T const>(default_value)))): _default{std::forward<T const>(default_value)} {}

  T const& get(IndexType const& idx) const noexcept override {
    auto const& it = _map.find(idx);
    if (it == _map.end()) {
      return _default;
    }
    return it->second;
  }
  void set(IndexType const& idx, const T& value) noexcept(noexcept(T(value))) override {
    if (value == _default){
      _map.erase(idx);
    } else {
      // _map[idx] = value;
      _map.insert_or_assign(idx, value);
    }
  }  
  void set(IndexType const& idx, T&& value) noexcept(noexcept(T(std::forward<T>(value)))) override {
    if (value == _default) {
      _map.erase(idx);
    } else {
      // _map.emplace(std::piecewise_construct, std::forward_as_tuple(idx),
      // std::forward_as_tuple(value)); T::T(const T&) _map.try_emplace(idx,
      // value);
      _map.insert_or_assign(idx, std::forward<T>(value));
    }
  }
  auto operator[](size_t idx1)  {
    LOG_PPF;
    HeadIndexType headIndex = {idx1};
    return HeadShiftIndex(this, headIndex);
  }
  size_t size() const noexcept { return _map.size(); }

  const_iterator begin() const noexcept { return _map.cbegin(); }
  const_iterator end() const noexcept { return _map.cend(); }
  const_iterator cbegin() const noexcept { return _map.cbegin(); }
  const_iterator cend() const noexcept { return _map.cend(); }

  friend std::ostream& operator<<(std::ostream& out, Matrix const& matrix) {
    for (auto const& cell : matrix) {
      auto const& [idx, value] = cell;
      out << "[" << idx[0];
      for (size_t i = 1; i < N_DIMS; ++i) out << "," << idx[i];
      out << "]=" << value << "\n";
    }
    return out;
  }
};

}  // namespace sparse