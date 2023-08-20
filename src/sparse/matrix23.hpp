#pragma once

#include <array>
#include <concepts>
#include <cstdarg>
#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "utils/logging.hpp"

namespace sparse::std23 {

template <std::copyable T, size_t N_DIMS = 2>
struct Matrix;

template <size_t N_DIMS>
using IndexType = std::array<size_t, N_DIMS>;

/**
 * @brief for internal use by Matrix class
 * @details
 * namespace contains implementation details of public Matrix class that are subjects to change
 * and not intendeed for use ouside this context
 */
namespace details {

template <size_t N_DIMS>
using IndexTypeShared = std::shared_ptr<IndexType<N_DIMS>>;

template <size_t N_DIMS>
using IndexTypeConstShared = std::shared_ptr<IndexType<N_DIMS> const>;

template <std::copyable T, size_t N_DIMS>
struct CellAccessor {
  virtual T const& get(IndexType<N_DIMS> const&) const noexcept = 0;
  virtual void set(IndexType<N_DIMS> const& idx, T const& value) noexcept(noexcept(T(value))) = 0;
  // && noexcept(operator=(std::decay<T>(value)))
  virtual void set(IndexType<N_DIMS> const& idx, T&& value) noexcept(noexcept(T(std::forward<T>(value)))) = 0;
  // && noexcept(::operator=(std::forward<T>(value)))
};

template <std::copyable T, size_t N_DIMS>
class ProxyCell {
  static_assert(N_DIMS > 0, "N_DIMS must be > 0");

  CellAccessor<T, N_DIMS>* _cell;  // no shared_ptr cuz Matrix object is created at stack as usual
  IndexTypeConstShared<N_DIMS> const _idx;

  ProxyCell(CellAccessor<T, N_DIMS>* cell, IndexTypeShared<N_DIMS> idx) noexcept : _cell{cell}, _idx{idx} {}

  ProxyCell(ProxyCell const&) = delete;
  ProxyCell& operator=(ProxyCell const&) = delete;

  friend struct Matrix<T, N_DIMS>;

 public:
  operator T() const noexcept { return _cell->get(*_idx.get()); }
  auto& operator=(T&& value) noexcept(noexcept(_cell->set(*_idx.get(), std::forward<T>(value)))) {
    _cell->set(*_idx.get(), std::forward<T>(value));
    return *this;
  }
  auto& operator=(T const& value) noexcept(noexcept(_cell->set(*_idx.get(), value))) {
    _cell->set(*_idx.get(), value);
    return *this;
  }
};

}  // namespace details

/**
 * @brief sparse matrix with `N_DIMS` "infinite" dimensions
 *
 * @code {.cpp}
 *    Matrix<int, 3> m(-1);
 *    m[100, 200, 300] = 1;
 *    assert(m[100, 200, 300] == 1);
 *    assert(m[1, 2, 3] == -1);
 *    assert(m.size() == 1);
 *    m[100, 200, 300] = -1;
 *    assert(m.size() == 0);
 * @endcode
 * @code {.cpp}
 *    Matrix<int> matrix;
 *    ((matrix[100, 100] = 314) = 0) = 217;
 *    assert(matrix[100, 100], 217);
 * @endcode
 * @note: `default_value` is moved from template parameters list to class member list
 *        to make it possible to assign matrix with diffrent `default_value`
 *        to expand possible applying.
 *
 * @tparam T copyable type of matrix element
 * @tparam N_DIMS number of matrix dimensions (>= 1)
 */
template <std::copyable T, size_t N_DIMS>
struct Matrix final : private details::CellAccessor<T, N_DIMS> {
  static_assert(N_DIMS > 0, "N_DIMS must be > 0");

  using IndexTypeConstShared = details::IndexTypeConstShared<N_DIMS>;
  using const_iterator = typename std::map<IndexType<N_DIMS>, T>::const_iterator;

  /** @name ctor */
  ///@{
  Matrix(T const& default_value) noexcept(noexcept(T(default_value))) : _default{default_value} {}
  Matrix(T&& default_value = T{}) noexcept(noexcept(T(std::forward<T>(default_value))))
      : _default{std::forward<T>(default_value)} {}
  Matrix(Matrix const& other) : _map(other._map), _default(other._default) {}
  Matrix(Matrix&& other) noexcept { swap(std::forward<T>(other)); }
  ///@}

  /** @name assignment */
  ///@{
  Matrix& operator=(Matrix const& other) noexcept(noexcept(Matrix(other))) {
    if (this != &other) {
      Matrix tmp(other);
      swap(std::move(tmp));
    }
    return *this;
  }
  Matrix& operator=(Matrix&& other) noexcept {
    swap(std::forward<T>(other));
    return *this;
  }
  ///@}

  /** @name index */
  ///@{
  T const& operator[](size_t first...) const {
    LOG_PPF;
    va_list args;
    va_start(args, first);
    IndexType<N_DIMS> index{first};
    for (size_t i = 1; i < N_DIMS; ++i) {
      index[i] = va_arg(args, size_t);
    }
    va_end(args);
    return get(index);
  }

  details::ProxyCell<T, N_DIMS> operator[](size_t first...) {
    LOG_PPF;
    va_list args;
    va_start(args, first);
    auto sp_index = std::shared_ptr<IndexType<N_DIMS>>(new IndexType<N_DIMS>{first});
    for (size_t i = 1; i < N_DIMS; ++i) {
      (*sp_index)[i] = va_arg(args, size_t);
    }
    va_end(args);
    return details::ProxyCell<T, N_DIMS>(this, sp_index);
  }
  ///@}

  /** @name details::CellAccessor */
  ///@{
  T const& get(IndexType<N_DIMS> const& idx) const noexcept {
    auto const& it = _map.find(idx);
    if (it == _map.end()) {
      return _default;
    }
    return it->second;
  }
  void set(IndexType<N_DIMS> const& idx, T const& value) noexcept(noexcept(T(value))) {
    if (value == _default)
      _map.erase(idx);
    else
      _map.insert_or_assign(idx, value);
  }
  void set(IndexType<N_DIMS> const& idx, T&& value) noexcept(noexcept(T(std::forward<T>(value)))) {
    if (value == _default)
      _map.erase(idx);
    else
      _map.insert_or_assign(idx, std::forward<T>(value));
  }
  ///@}

  /** @name misc */
  ///@{
  size_t size() const noexcept { return _map.size(); }
  ///@}

  /** @name iter */
  ///@{
  const_iterator begin() const noexcept { return _map.cbegin(); }
  const_iterator end() const noexcept { return _map.cend(); }
  const_iterator cbegin() const noexcept { return _map.cbegin(); }
  const_iterator cend() const noexcept { return _map.cend(); }
  ///@}

  /** @name friends */
  ///@{
  friend std::ostream& operator<<(std::ostream& out, Matrix const& matrix) {
    for (auto const& cell : matrix) {
      auto const& [idx, value] = cell;
      out << "[" << idx[0];
      for (size_t i = 1; i < N_DIMS; ++i) out << "," << idx[i];
      out << "]=" << value << "\n";
    }
    return out;
  }
  ///@}

 private:
  std::map<IndexType<N_DIMS>, T> _map; /**< std::map is used to store elements because matrix iterator outputs elemenets
                                  sorted by index as required. */
  T _default;

  void swap(Matrix&& other) noexcept {
    std::swap(_map, other._map);
    std::swap(_default, other._default);
  }
};

}  // namespace sparse::std23