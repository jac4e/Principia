#pragma once

#include "numerics/fixed_arrays.hpp"

#include <algorithm>
#include <utility>
#include <vector>

#include "glog/logging.h"
#include "quantities/elementary_functions.hpp"

namespace principia {
namespace numerics {
namespace _fixed_arrays {
namespace internal {

using namespace principia::quantities::_elementary_functions;

// A helper class to compute the dot product of two arrays.  |LScalar| and
// |RScalar| are the types of the elements of the arrays.  |Left| and |Right|
// are the (deduced) types of the arrays.  They must both have an operator[].
// |size| is the size of the arrays.  This unrolling helps with performance.
template<typename LScalar, typename RScalar, int size, int i = size - 1>
struct DotProduct {
  template<typename Left, typename Right>
  static Product<LScalar, RScalar> Compute(Left const& left,
                                           Right const& right);
};

template<typename LScalar, typename RScalar, int size>
struct DotProduct<LScalar, RScalar, size, 0> {
  template<typename Left, typename Right>
  static Product<LScalar, RScalar> Compute(Left const& left,
                                           Right const& right);
};

template<typename LScalar, typename RScalar, int size, int i>
template<typename Left, typename Right>
Product<LScalar, RScalar>
DotProduct<LScalar, RScalar, size, i>::Compute(Left const& left,
                                               Right const& right) {
  return left[i] * right[i] +
         DotProduct<LScalar, RScalar, size, i - 1>::Compute(left, right);
}

template<typename LScalar, typename RScalar, int size>
template<typename Left, typename Right>
Product<LScalar, RScalar>
DotProduct<LScalar, RScalar, size, 0>::Compute(Left const& left,
                                               Right const& right) {
  return left[0] * right[0];
}

// The |data_| member is aggregate-initialized with an empty list initializer,
// which performs value initialization on the components.  For quantities this
// calls the default constructor, for non-class types this does
// zero-initialization.
template<typename Scalar, int size_>
constexpr FixedVector<Scalar, size_>::FixedVector() : data_{} {}

template<typename Scalar, int size_>
FixedVector<Scalar, size_>::FixedVector(uninitialized_t) {}

template<typename Scalar, int size_>
constexpr FixedVector<Scalar, size_>::FixedVector(
    std::array<Scalar, size_> const& data)
    : data_(data) {}

template<typename Scalar, int size_>
constexpr FixedVector<Scalar, size_>::FixedVector(
    std::array<Scalar, size_>&& data)
    : data_(std::move(data)) {}

template<typename Scalar, int size_>
TransposedView<FixedVector<Scalar, size_>>
FixedVector<Scalar, size_>::Transpose() const {
  return {.transpose = *this};
}

template<typename Scalar, int size_>
Scalar FixedVector<Scalar, size_>::Norm() const {
  return Sqrt(Norm²());
}

template<typename Scalar, int size_>
inline Square<Scalar> FixedVector<Scalar, size_>::Norm²() const {
  return DotProduct<Scalar, Scalar, size_>::Compute(data_, data_);
}

template<typename Scalar, int size_>
constexpr Scalar& FixedVector<Scalar, size_>::operator[](int const index) {
  CONSTEXPR_DCHECK(0 <= index);
  CONSTEXPR_DCHECK(index < size());
  return data_[index];
}

template<typename Scalar, int size_>
constexpr Scalar const& FixedVector<Scalar, size_>::operator[](
    int const index) const {
  CONSTEXPR_DCHECK(0 <= index);
  CONSTEXPR_DCHECK(index < size());
  return data_[index];
}

template<typename Scalar, int size_>
bool FixedVector<Scalar, size_>::operator==(FixedVector const& right) const {
  return data_ == right.data_;
}

template<typename Scalar, int size_>
bool FixedVector<Scalar, size_>::operator!=(FixedVector const& right) const {
  return data_ != right.data_;
}

template<typename H, typename Scalar, int size_>
H AbslHashValue(H h, FixedVector<Scalar, size_> const& vector) {
}

template<typename Scalar, int rows_, int columns_>
constexpr FixedMatrix<Scalar, rows_, columns_>::FixedMatrix()
    : data_{} {}

template<typename Scalar, int rows_, int columns_>
FixedMatrix<Scalar, rows_, columns_>::FixedMatrix(uninitialized_t) {}

template<typename Scalar, int rows_, int columns_>
constexpr FixedMatrix<Scalar, rows_, columns_>::FixedMatrix(
    std::array<Scalar, size()> const& data)
    : data_(data) {}

template<typename Scalar, int rows_, int columns_>
constexpr Scalar& FixedMatrix<Scalar, rows_, columns_>::operator()(
    int const row, int const column) {
  CONSTEXPR_DCHECK(0 <= row);
  CONSTEXPR_DCHECK(row < rows());
  CONSTEXPR_DCHECK(0 <= column);
  CONSTEXPR_DCHECK(column < columns());
  return data_[row * columns() + column];
}

template<typename Scalar, int rows_, int columns_>
constexpr Scalar const& FixedMatrix<Scalar, rows_, columns_>::operator()(
    int const row, int const column) const {
  CONSTEXPR_DCHECK(0 <= row);
  CONSTEXPR_DCHECK(row < rows());
  CONSTEXPR_DCHECK(0 <= column);
  CONSTEXPR_DCHECK(column < columns());
  return data_[row * columns() + column];
}

template<typename Scalar, int rows_, int columns_>
template<int r>
Scalar const* FixedMatrix<Scalar, rows_, columns_>::row() const {
  static_assert(r < rows_);
  return &data_[r * columns()];
}

template<typename Scalar, int rows_, int columns_>
template<typename LScalar, typename RScalar>
Product<Scalar, Product<LScalar, RScalar>>
FixedMatrix<Scalar, rows_, columns_>::operator()(
    FixedVector<LScalar, columns_> const& left,
    FixedVector<RScalar, rows_> const& right) const {
  return left.Transpose() * (*this * right);
}

template<typename Scalar, int rows_, int columns_>
FixedMatrix<Scalar, rows_, columns_>
FixedMatrix<Scalar, rows_, columns_>::Transpose() const {
  FixedMatrix<Scalar, rows(), columns()> m(uninitialized);
  for (int i = 0; i < rows(); ++i) {
    for (int j = 0; j < columns(); ++j) {
      m(j, i) = (*this)(i, j);
    }
  }
  return m;
}

template<typename Scalar, int rows_, int columns_>
Scalar FixedMatrix<Scalar, rows_, columns_>::FrobeniusNorm() const {
  Square<Scalar> Σᵢⱼaᵢⱼ²{};
  for (int i = 0; i < rows(); ++i) {
    for (int j = 0; j < columns(); ++j) {
      Σᵢⱼaᵢⱼ² += Pow<2>((*this)(i, j));
    }
  }
  return Sqrt(Σᵢⱼaᵢⱼ²);
}

template<typename Scalar, int rows_, int columns_>
bool FixedMatrix<Scalar, rows_, columns_>::operator==(
    FixedMatrix const& right) const {
  return data_ == right.data_;
}

template<typename Scalar, int rows_, int columns_>
bool FixedMatrix<Scalar, rows_, columns_>::operator!=(
    FixedMatrix const& right) const {
  return data_ != right.data_;
}

template<typename Scalar, int rows_, int columns_>
FixedMatrix<Scalar, rows_, columns_>
FixedMatrix<Scalar, rows_, columns_>::Identity() {
  FixedMatrix<Scalar, rows(), columns()> m;
  for (int i = 0; i < rows(); ++i) {
    m(i, i) = 1;
  }
  return m;
}

template<typename Scalar, int rows_>
constexpr FixedStrictlyLowerTriangularMatrix<Scalar, rows_>::
    FixedStrictlyLowerTriangularMatrix()
    : data_{} {}

template<typename Scalar, int rows_>
FixedStrictlyLowerTriangularMatrix<Scalar, rows_>::
    FixedStrictlyLowerTriangularMatrix(uninitialized_t) {}

template<typename Scalar, int rows_>
constexpr FixedStrictlyLowerTriangularMatrix<Scalar, rows_>::
FixedStrictlyLowerTriangularMatrix(std::array<Scalar, size()> const& data)
    : data_(data) {}

template<typename Scalar, int rows_>
constexpr Scalar& FixedStrictlyLowerTriangularMatrix<Scalar, rows_>::
operator()(int const row, int const column) {
  CONSTEXPR_DCHECK(0 <= column);
  CONSTEXPR_DCHECK(column < row);
  CONSTEXPR_DCHECK(row < rows());
  return data_[row * (row - 1) / 2 + column];
}

template<typename Scalar, int rows_>
constexpr Scalar const& FixedStrictlyLowerTriangularMatrix<Scalar, rows_>::
operator()(int const row, int const column) const {
  CONSTEXPR_DCHECK(0 <= column);
  CONSTEXPR_DCHECK(column < row);
  CONSTEXPR_DCHECK(row < rows());
  return data_[row * (row - 1) / 2 + column];
}

template<typename Scalar, int rows_>
template<int r>
Scalar const* FixedStrictlyLowerTriangularMatrix<Scalar, rows_>::row() const {
  static_assert(r < rows_);
  return &data_[r * (r - 1) / 2];
}

template<typename Scalar, int rows_>
bool FixedStrictlyLowerTriangularMatrix<Scalar, rows_>::operator==(
    FixedStrictlyLowerTriangularMatrix const& right) const {
  return data_ == right.data_;
}

template<typename Scalar, int rows_>
bool FixedStrictlyLowerTriangularMatrix<Scalar, rows_>::operator!=(
    FixedStrictlyLowerTriangularMatrix const& right) const {
  return data_ != right.data_;
}

template<typename Scalar, int rows_>
constexpr FixedLowerTriangularMatrix<Scalar, rows_>::
FixedLowerTriangularMatrix()
    : data_{} {}

template<typename Scalar, int rows_>
FixedLowerTriangularMatrix<Scalar, rows_>::
FixedLowerTriangularMatrix(uninitialized_t) {}

template<typename Scalar, int rows_>
constexpr FixedLowerTriangularMatrix<Scalar, rows_>::
FixedLowerTriangularMatrix(std::array<Scalar, size()> const& data)
    : data_(data) {}

template<typename Scalar, int rows_>
constexpr Scalar& FixedLowerTriangularMatrix<Scalar, rows_>::
operator()(int const row, int const column) {
  CONSTEXPR_DCHECK(0 <= column);
  CONSTEXPR_DCHECK(column <= row);
  CONSTEXPR_DCHECK(row < rows());
  return data_[row * (row + 1) / 2 + column];
}

template<typename Scalar, int rows_>
constexpr Scalar const& FixedLowerTriangularMatrix<Scalar, rows_>::
operator()(int const row, int const column) const {
  CONSTEXPR_DCHECK(0 <= column);
  CONSTEXPR_DCHECK(column <= row);
  CONSTEXPR_DCHECK(row < rows());
  return data_[row * (row + 1) / 2 + column];
}

template<typename Scalar, int rows_>
bool FixedLowerTriangularMatrix<Scalar, rows_>::operator==(
    FixedLowerTriangularMatrix const& right) const {
  return data_ == right.data_;
}

template<typename Scalar, int rows_>
bool FixedLowerTriangularMatrix<Scalar, rows_>::operator!=(
    FixedLowerTriangularMatrix const& right) const {
  return data_ != right.data_;
}

template<typename Scalar, int columns_>
constexpr FixedUpperTriangularMatrix<Scalar, columns_>::
FixedUpperTriangularMatrix()
    : data_{} {}

template<typename Scalar, int columns_>
FixedUpperTriangularMatrix<Scalar, columns_>::FixedUpperTriangularMatrix(
    uninitialized_t) {}

template<typename Scalar, int columns_>
constexpr FixedUpperTriangularMatrix<Scalar, columns_>::
FixedUpperTriangularMatrix(std::array<Scalar, size()> const& data)
    : data_(Transpose(data)) {}

template<typename Scalar, int columns_>
constexpr Scalar& FixedUpperTriangularMatrix<Scalar, columns_>::
operator()(int const row, int const column) {
  CONSTEXPR_DCHECK(0 <= row);
  CONSTEXPR_DCHECK(row <= column);
  CONSTEXPR_DCHECK(column < columns());
  return data_[column * (column + 1) / 2 + row];
}

template<typename Scalar, int columns_>
constexpr Scalar const& FixedUpperTriangularMatrix<Scalar, columns_>::
operator()(int const row, int const column) const {
  CONSTEXPR_DCHECK(0 <= row);
  CONSTEXPR_DCHECK(row <= column);
  CONSTEXPR_DCHECK(column < columns());
  return data_[column * (column + 1) / 2 + row];
}

template<typename Scalar, int columns_>
bool FixedUpperTriangularMatrix<Scalar, columns_>::operator==(
    FixedUpperTriangularMatrix const& right) const {
  return data_ == right.data_;
}

template<typename Scalar, int columns_>
bool FixedUpperTriangularMatrix<Scalar, columns_>::operator!=(
    FixedUpperTriangularMatrix const& right) const {
  return data_ != right.data_;
}

template<typename Scalar, int columns_>
auto FixedUpperTriangularMatrix<Scalar, columns_>::Transpose(
    std::array<Scalar, size()> const& data)
    -> std::array<Scalar, size()> {
  std::array<Scalar, rows() * columns()> full;
  int index = 0;
  for (int row = 0; row < rows(); ++row) {
    for (int column = row; column < columns(); ++column) {
      full[row * columns() + column] = data[index];
      ++index;
    }
  }

  std::array<Scalar, size()> result;
  index = 0;
  for (int column = 0; column < columns(); ++column) {
    for (int row = 0; row <= column; ++row) {
      result[index] = full[row * columns() + column];
      ++index;
    }
  }
  return result;
}

template<typename LScalar, typename RScalar, int size>
constexpr Product<LScalar, RScalar> InnerProduct(
    FixedVector<LScalar, size> const& left,
    FixedVector<RScalar, size> const& right) {
  return DotProduct<LScalar, RScalar, size>::Compute(left, right);
}

template<typename Scalar, int size>
constexpr FixedVector<double, size> Normalize(
    FixedVector<Scalar, size> const& vector) {
  return vector / vector.Norm();
}

template<typename LScalar, typename RScalar, int lsize, int rsize>
constexpr FixedMatrix<Product<LScalar, RScalar>, lsize, rsize> SymmetricProduct(
    FixedVector<LScalar, lsize> const& left,
    FixedVector<RScalar, rsize> const& right) {
  FixedMatrix<Product<LScalar, RScalar>, lsize, rsize> result(uninitialized);
  for (int i = 0; i < lsize; ++i) {
    for (int j = 0; j < i; ++j) {
      auto const r = 0.5 * (left[i] * right[j] + left[j] * right[i]);
      result(i, j) = r;
      result(j, i) = r;
    }
    result(i, i) = left[i] * right[i];
  }
  return result;
}

template<typename Scalar, int size>
constexpr FixedMatrix<Square<Scalar>, size, size> SymmetricSquare(
    FixedVector<Scalar, size> const& vector) {
  FixedMatrix<Square<Scalar>, size, size> result(uninitialized);
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < i; ++j) {
      auto const r =  vector[i] * vector[j];
      result(i, j) = r;
      result(j, i) = r;
    }
    result(i, i) = Pow<2>(vector[i]);
  }
  return result;
}

template<typename Scalar, int size>
constexpr FixedVector<Scalar, size> operator-(
    FixedVector<Scalar, size> const& right) {
  std::array<Scalar, size> result;
  for (int i = 0; i < size; ++i) {
    result[i] = -right[i];
  }
  return FixedVector<Difference<Scalar>, size>(std::move(result));
}

template<typename Scalar, int rows, int columns>
constexpr FixedMatrix<Scalar, rows, columns> operator-(
    FixedMatrix<Scalar, rows, columns> const& right) {
  FixedMatrix<Scalar, rows, columns> result(uninitialized);
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      result(i, j) = -right(i, j);
    }
  }
  return result;
}

template<typename LScalar, typename RScalar, int size>
constexpr FixedVector<Sum<LScalar, RScalar>, size> operator+(
    FixedVector<LScalar, size> const& left,
    FixedVector<RScalar, size> const& right) {
  std::array<Sum<LScalar, RScalar>, size> result;
  for (int i = 0; i < size; ++i) {
    result[i] = left[i] + right[i];
  }
  return FixedVector<Sum<LScalar, RScalar>, size>(std::move(result));
}

template<typename LScalar, typename RScalar, int rows, int columns>
constexpr FixedMatrix<Sum<LScalar, RScalar>, rows, columns> operator+(
    FixedMatrix<LScalar, rows, columns> const& left,
    FixedMatrix<RScalar, rows, columns> const& right) {
  FixedMatrix<Sum<LScalar, RScalar>, rows, columns> result(uninitialized);
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      result(i, j) = left(i, j) + right(i, j);
    }
  }
  return result;
}

template<typename LScalar, typename RScalar, int size>
constexpr FixedVector<Difference<LScalar, RScalar>, size> operator-(
    FixedVector<LScalar, size> const& left,
    FixedVector<RScalar, size> const& right) {
  std::array<Difference<LScalar, RScalar>, size> result;
  for (int i = 0; i < size; ++i) {
    result[i] = left[i] - right[i];
  }
  return FixedVector<Difference<LScalar, RScalar>, size>(std::move(result));
}

template<typename LScalar, typename RScalar, int rows, int columns>
constexpr FixedMatrix<Difference<LScalar, RScalar>, rows, columns> operator-(
    FixedMatrix<LScalar, rows, columns> const& left,
    FixedMatrix<RScalar, rows, columns> const& right) {
  FixedMatrix<Difference<LScalar, RScalar>, rows, columns> result(
      uninitialized);
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      result(i, j) = left(i, j) - right(i, j);
    }
  }
  return result;
}

template<typename Scalar, int size>
constexpr FixedVector<Scalar, size>& operator+=(
    FixedVector<Scalar, size>& left,
    FixedVector<Scalar, size> const& right) {
  left = left + right;
  return left;
}

template<typename Scalar, int rows, int columns>
constexpr FixedMatrix<Scalar, rows, columns>& operator+=(
    FixedMatrix<Scalar, rows, columns>& left,
    FixedMatrix<Scalar, rows, columns> const& right) {
  left = left + right;
  return left;
}

template<typename Scalar, int size>
constexpr FixedVector<Scalar, size>& operator-=(
    FixedVector<Scalar, size>& left,
    FixedVector<Scalar, size> const& right) {
  left = left - right;
  return left;
}

template<typename Scalar, int rows, int columns>
constexpr FixedMatrix<Scalar, rows, columns>& operator-=(
    FixedMatrix<Scalar, rows, columns>& left,
    FixedMatrix<Scalar, rows, columns> const& right) {
  left = left - right;
  return left;
}

template<typename LScalar, typename RScalar, int size>
constexpr FixedVector<Product<LScalar, RScalar>, size> operator*(
    LScalar const left,
    FixedVector<RScalar, size> const& right) {
  std::array<Product<LScalar, RScalar>, size> result;
  for (int i = 0; i < size; ++i) {
    result[i] = left * right[i];
  }
  return FixedVector<Product<LScalar, RScalar>, size>(std::move(result));
}

template<typename LScalar, typename RScalar, int size>
constexpr FixedVector<Product<LScalar, RScalar>, size> operator*(
    FixedVector<LScalar, size> const& left,
    RScalar const right) {
  std::array<Product<LScalar, RScalar>, size> result;
  for (int i = 0; i < size; ++i) {
    result[i] = left[i] * right;
  }
  return FixedVector<Product<LScalar, RScalar>, size>(std::move(result));
}

template<typename LScalar, typename RScalar, int rows, int columns>
constexpr FixedMatrix<Product<LScalar, RScalar>, rows, columns> operator*(
    LScalar const left,
    FixedMatrix<RScalar, rows, columns> const& right) {
  FixedMatrix<Product<LScalar, RScalar>, rows, columns> result(
      uninitialized);
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      result(i, j) = left * right(i, j);
    }
  }
  return result;
}

template<typename LScalar, typename RScalar, int rows, int columns>
constexpr FixedMatrix<Product<LScalar, RScalar>, rows, columns> operator*(
    FixedMatrix<LScalar, rows, columns> const& left,
    RScalar const right) {
  FixedMatrix<Product<LScalar, RScalar>, rows, columns> result(
      uninitialized);
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      result(i, j) = left(i, j) * right;
    }
  }
  return result;
}

template<typename LScalar, typename RScalar, int size>
constexpr FixedVector<Quotient<LScalar, RScalar>, size> operator/(
    FixedVector<LScalar, size> const& left,
    RScalar const& right) {
  FixedVector<Quotient<LScalar, RScalar>, size> result(uninitialized);
  for (int i = 0; i < left.size(); ++i) {
    result[i] = left[i] / right;
  }
  return result;
}

template<typename LScalar, typename RScalar, int rows, int columns>
constexpr FixedMatrix<Quotient<LScalar, RScalar>, rows, columns> operator/(
    FixedMatrix<LScalar, rows, columns> const& left,
    RScalar const right) {
  FixedMatrix<Quotient<LScalar, RScalar>, rows, columns> result(
      uninitialized);
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      result(i, j) = left(i, j) / right;
    }
  }
  return result;
}

template<typename LScalar, typename RScalar, int size>
constexpr Product<LScalar, RScalar> operator*(
    LScalar* const left,
    FixedVector<RScalar, size> const& right) {
  return DotProduct<LScalar, RScalar, size>::Compute(left, right.data_);
}

template<typename LScalar, typename RScalar, int size>
constexpr Product<LScalar, RScalar> operator*(
    TransposedView<FixedVector<LScalar, size>> const& left,
    FixedVector<RScalar, size> const& right) {
  return DotProduct<LScalar, RScalar, size>::Compute(
      left.transpose.data_, right.data_);
}

template<typename LScalar, typename RScalar, int lsize, int rsize>
constexpr FixedMatrix<Product<LScalar, RScalar>, lsize, rsize> operator*(
    FixedVector<LScalar, lsize> const& left,
    TransposedView<FixedVector<RScalar, rsize>> const& right) {
  FixedMatrix<Product<LScalar, RScalar>, lsize, rsize> result(uninitialized);
  for (int i = 0; i < lsize; ++i) {
    for (int j = 0; j < rsize; ++j) {
      result(i, j) = left[i] * right.transpose[j];
    }
  }
  return result;
}

template<typename LScalar, typename RScalar,
         int rows, int dimension, int columns>
constexpr FixedMatrix<Product<LScalar, RScalar>, rows, columns>
operator*(FixedMatrix<LScalar, rows, dimension> const& left,
          FixedMatrix<RScalar, dimension, columns> const& right) {
  FixedMatrix<Product<LScalar, RScalar>, rows, columns> result{};
  for (int i = 0; i < left.rows(); ++i) {
    for (int j = 0; j < right.columns(); ++j) {
      for (int k = 0; k < left.columns(); ++k) {
        result(i, j) += left(i, k) * right(k, j);
      }
    }
  }
  return result;
}

template<typename LScalar, typename RScalar, int rows, int columns>
constexpr FixedVector<Product<LScalar, RScalar>, rows> operator*(
    FixedMatrix<LScalar, rows, columns> const& left,
    FixedVector<RScalar, columns> const& right) {
  std::array<Product<LScalar, RScalar>, rows> result;
  auto const* row = left.data_.data();
  for (int i = 0; i < rows; ++i) {
    result[i] =
        DotProduct<LScalar, RScalar, columns>::Compute(row, right.data_);
    row += columns;
  }
  return FixedVector<Product<LScalar, RScalar>, rows>(std::move(result));
}

template<typename Scalar, int size>
std::ostream& operator<<(std::ostream& out,
                         FixedVector<Scalar, size> const& vector) {
  std::stringstream s;
  for (int i = 0; i < size; ++i) {
    s << (i == 0 ? "{" : "") << vector[i]
      << (i == size - 1 ? "}" : ", ");
  }
  out << s.str();
  return out;
}

template<typename Scalar, int rows, int columns>
std::ostream& operator<<(std::ostream& out,
                         FixedMatrix<Scalar, rows, columns> const& matrix) {
  out << "rows: " << rows << " columns: " << columns << "\n";
  for (int i = 0; i < rows; ++i) {
    out << "{";
    for (int j = 0; j < columns; ++j) {
      out << matrix(i, j);
      if (j < columns - 1) {
        out << ", ";
      }
    }
    out << "}\n";
  }
  return out;
}

template<typename Scalar, int rows>
std::ostream& operator<<(
    std::ostream& out,
    FixedLowerTriangularMatrix<Scalar, rows> const& matrix) {
  out << "rows: " << matrix.rows() << "\n";
  for (int i = 0; i < matrix.rows(); ++i) {
    out << "{";
    for (int j = 0; j <= i; ++j) {
      out << matrix(i, j);
      if (j < i) {
        out << ", ";
      }
    }
    out << "}\n";
  }
  return out;
}

template<typename Scalar, int columns>
std::ostream& operator<<(
    std::ostream& out,
    FixedUpperTriangularMatrix<Scalar, columns> const& matrix) {
  out << "columns: " << matrix.columns() << "\n";
  for (int i = 0; i < matrix.columns(); ++i) {
    out << "{";
    for (int j = i; j < matrix.columns(); ++j) {
      if (j > i) {
        out << ", ";
      }
      out << matrix(i, j);
    }
    out << "}\n";
  }
  return out;
}

}  // namespace internal
}  // namespace _fixed_arrays
}  // namespace numerics
}  // namespace principia
