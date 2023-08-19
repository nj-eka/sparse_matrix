#pragma once

#include <array>
#include <concepts>
#include <map>
#include <memory>
#include <ostream>
#include <type_traits>
#include <utility>

#include "utils/logging.hpp"

namespace sparse {

template <std::copyable T, size_t N_DIMS = 2>
struct Matrix;

namespace details {

template <size_t N_DIMS>
using IndexType = std::array<size_t, N_DIMS>;

template <size_t N_DIMS>
using IndexTypeShared = std::shared_ptr<IndexType<N_DIMS>>;

template <size_t N_DIMS>
using IndexTypeConstShared = std::shared_ptr<IndexType<N_DIMS> const>;

template <std::copyable T, size_t N_DIMS>
struct CellAccessor {
  virtual T const& get(IndexTypeConstShared<N_DIMS>) const noexcept = 0;
  virtual void set(IndexTypeConstShared<N_DIMS> idx, T const& value) noexcept(noexcept(T(value))) = 0;
  // && noexcept(operator=(std::decay<T>(value)))
  virtual void set(IndexTypeConstShared<N_DIMS> idx, T&& value) noexcept(noexcept(T(std::forward<T>(value)))) = 0;
  // && noexcept(::operator=(std::forward<T>(value)))
};

template <std::copyable T, size_t N_DIMS, size_t DIM>
class ShiftIndex {
  static_assert(N_DIMS > 0, "N_DIMS must be > 0");
  static_assert(DIM < N_DIMS, "DIM out of declared range (N_DIMS)");

  CellAccessor<T, N_DIMS>* _cell;  // no shared_ptr cuz Matrix object is created at stack as usual
  IndexTypeShared<N_DIMS> const _idx;

  ShiftIndex(CellAccessor<T, N_DIMS>* cell, IndexTypeShared<N_DIMS> idx) noexcept : _cell{cell}, _idx{idx} {}

  ShiftIndex(ShiftIndex const&) = delete;
  ShiftIndex& operator=(ShiftIndex const&) = delete;

  friend struct ShiftIndex<T, N_DIMS, DIM - 1>;
  friend struct sparse::Matrix<T, N_DIMS>;

 public:
  auto operator[](size_t last_idx) noexcept {
    (*_idx)[DIM] = last_idx;  // m[]...[] - only in this way it is supposed to be used
    return ShiftIndex<T, N_DIMS, DIM + 1>(_cell, _idx);
  }
};

template <std::copyable T, size_t N_DIMS>
class ShiftIndex<T, N_DIMS, N_DIMS> {
  static_assert(N_DIMS > 0, "N_DIMS must be > 0");

  CellAccessor<T, N_DIMS>* _cell;  // no shared_ptr cuz Matrix object is created at stack as usual
  IndexTypeConstShared<N_DIMS> const _idx;

  ShiftIndex(CellAccessor<T, N_DIMS>* cell, IndexTypeShared<N_DIMS> idx) noexcept : _cell{cell}, _idx{idx} {}

  ShiftIndex(ShiftIndex const&) = delete;
  ShiftIndex& operator=(ShiftIndex const&) = delete;

  friend struct ShiftIndex<T, N_DIMS, N_DIMS - 1>;
  friend struct sparse::Matrix<T, N_DIMS>;

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
};

}  // namespace details

template <std::copyable T, size_t N_DIMS>
struct Matrix final : details::CellAccessor<T, N_DIMS> {
  static_assert(N_DIMS > 0, "N_DIMS must be > 0");

  using IndexType = details::IndexType<N_DIMS>;
  using IndexTypeConstShared = details::IndexTypeConstShared<N_DIMS>;
  using HeadShiftIndex = details::ShiftIndex<T, N_DIMS, 1>;

  using const_iterator = typename std::map<IndexType, T>::const_iterator;

 private:
  std::map<IndexType, T> _map;  // matrix elements should be ordered by task
  T _default;

  void swap(Matrix&& other) noexcept {
    std::swap(_map, other._map);
    std::swap(_default, other._default);
  }

 public:
  // ctor
  Matrix(T const& default_value) noexcept(noexcept(T(default_value))) : _default{default_value} {}
  Matrix(T&& default_value = T{}) noexcept(noexcept(T(std::forward<T>(default_value))))
      : _default{std::forward<T>(default_value)} {}
  Matrix(Matrix const& other) : _map(other._map), _default(other._default) {}
  Matrix(Matrix&& other) { swap(std::forward<T>(other)); }

  // assignment
  Matrix& operator=(Matrix const& other) {
    if (this != &other) {
      _map = other._map;          // copy-assignment
      _default = other._default;  // copy-assignment
    }
    return *this;
  }
  Matrix& operator=(Matrix&& other) noexcept {
    swap(std::forward<T>(other));
    return *this;
  }

  // details::CellAccessor impl
  T const& get(IndexTypeConstShared idx) const noexcept override {
    auto const& it = _map.find(*idx.get());
    if (it == _map.end()) {
      return _default;
    }
    return it->second;
  }
  void set(IndexTypeConstShared idx, T const& value) noexcept(noexcept(T(value))) override {
    if (value == _default) {
      _map.erase(*idx.get());
    } else {
      // _map[idx] = value;
      _map.insert_or_assign(*idx.get(), value);
    }
  }
  void set(IndexTypeConstShared idx, T&& value) noexcept(noexcept(T(std::forward<T>(value)))) override {
    if (value == _default) {
      _map.erase(*idx.get());
    } else {
      // _map.emplace(std::piecewise_construct, std::forward_as_tuple(idx),
      // std::forward_as_tuple(value)); T::T(const T&) _map.try_emplace(idx,
      // value);
      _map.insert_or_assign(*idx.get(), std::forward<T>(value));
    }
  }

  // ShiftIndex
  auto operator[](size_t idx1) {
    LOG_PPF;
    return HeadShiftIndex(this, details::IndexTypeShared<N_DIMS>(new IndexType{idx1}));
  }

  // misc
  size_t size() const noexcept { return _map.size(); }

  // iter
  const_iterator begin() const noexcept { return _map.cbegin(); }
  const_iterator end() const noexcept { return _map.cend(); }
  const_iterator cbegin() const noexcept { return _map.cbegin(); }
  const_iterator cend() const noexcept { return _map.cend(); }

  // friends
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