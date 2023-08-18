#pragma once

#include <array>
#include <map>
#include <ostream>
#include <utility>

#include "utils/logging.hpp"

namespace sparse {

template <typename T, size_t N_DIMS = 2>
struct Matrix;

namespace details {

template <size_t N_DIMS>
using IndexType = std::array<size_t, N_DIMS>;

template <typename T, size_t N_DIMS>
struct CellAccessor {
  virtual T const& get(IndexType<N_DIMS> const&) const = 0;
  virtual void set(IndexType<N_DIMS> const&, T&&) = 0;
  virtual void set(IndexType<N_DIMS> const&, const T&) = 0;
};

template <typename T, size_t N_DIMS, size_t DIM>
class ShiftIndex {
  ShiftIndex(ShiftIndex const&) = delete;
  ShiftIndex& operator=(ShiftIndex const&) = delete;

  CellAccessor<T, N_DIMS>* _cell;
  IndexType<DIM + 1> _idx;

  ShiftIndex(CellAccessor<T, N_DIMS>* cell, IndexType<DIM> idx) : _cell{cell} {
    std::copy(std::begin(idx), std::end(idx), std::begin(_idx));
    LOG_PPF;
  }

 public:
  auto operator[](size_t last_idx) {
    LOG_PPF;
    _idx[DIM] = last_idx;
    return ShiftIndex<T, N_DIMS, DIM + 1>(_cell, _idx);
  }
  friend struct ShiftIndex<T, N_DIMS, DIM - 1>;
  friend struct sparse::Matrix<T, N_DIMS>;
};

template <typename T, size_t N_DIMS>
class ShiftIndex<T, N_DIMS, N_DIMS> {
  ShiftIndex(ShiftIndex const&) = delete;
  ShiftIndex& operator=(ShiftIndex const&) = delete;

  CellAccessor<T, N_DIMS>* _cell;
  std::array<size_t, N_DIMS> const _idx;

  ShiftIndex(CellAccessor<T, N_DIMS>* cell, IndexType<N_DIMS> const& idx)
      : _cell{cell}, _idx{idx} {}

 public:
  auto& operator=(T&& value) {
    _cell->set(_idx, std::forward<T>(value));
    return *this;
  }
  auto& operator=(T const& value) {
    _cell->set(_idx, value);
    return *this;
  }
  operator T() const { return _cell->get(_idx); }

  friend struct ShiftIndex<T, N_DIMS, N_DIMS - 1>;
  friend struct sparse::Matrix<T, N_DIMS>;
};

}  // namespace details

template <typename T, size_t N_DIMS>
struct Matrix final : details::CellAccessor<T, N_DIMS> {
  using IndexType = details::IndexType<N_DIMS>;
  using HeadIndexType = details::IndexType<1>;
  using HeadShiftIndex = details::ShiftIndex<T, N_DIMS, 1>;
  using const_iterator = typename std::map<IndexType, T>::const_iterator;

 private:
  std::map<IndexType, T> _map;
  T const _default;

 public:
  Matrix(const T& default_value = T{}) : _default{default_value} {}
  T const& get(IndexType const& idx) const override {
    auto const& it = _map.find(idx);
    if (it == _map.end()) {
      return _default;
    }
    return it->second;
  }
  void set(IndexType const& idx, T&& value) override {
    if (value == _default) {
      _map.erase(idx);
    } else {
      // _map.emplace(std::piecewise_construct, std::forward_as_tuple(idx),
      // std::forward_as_tuple(value)); T::T(const T&) _map.try_emplace(idx,
      // value);
      _map.insert_or_assign(idx, std::forward<T>(value));
    }
  }
  void set(IndexType const& idx, const T& value) override {
    if (value == _default)
      _map.erase(idx);
    else
      _map[idx] = value;
  }
  auto operator[](size_t idx1) {
    LOG_PPF;
    HeadIndexType headIndex = {idx1};
    return HeadShiftIndex(this, headIndex);
  }
  size_t size() const { return _map.size(); }

  const_iterator begin() const { return _map.cbegin(); }
  const_iterator end() const { return _map.cend(); }
  const_iterator cbegin() const { return _map.cbegin(); }
  const_iterator cend() const { return _map.cend(); }

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