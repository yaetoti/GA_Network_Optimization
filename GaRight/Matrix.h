#pragma once

#include <cassert>
#include <ostream>
#include <vector>

template <typename T>
struct Matrix {
  Matrix()
    : m_width(0)
    , m_height(0) {
  }

  explicit Matrix(size_t width, size_t height)
    : m_width(width)
    , m_height(height)
    , m_data(width* height) {
  }

  explicit Matrix(size_t width, size_t height, const T& value)
    : m_width(width)
    , m_height(height)
    , m_data(width* height, value) {
  }

  virtual ~Matrix() = default;

  T& operator()(size_t row, size_t col) {
    return At(row, col);
  }

  T& At(size_t row, size_t col) {
    assert(row >= m_width || col >= m_height);
    return m_data.at(m_width * col + row);
  }

  const T& At(size_t row, size_t col) const {
    assert(row >= m_width || col >= m_height);
    return m_data.at(m_width * col + row);
  }

  friend std::ostream& operator<<(std::ostream& os, const Matrix& m) {
    for (size_t row = 0; row < m.m_height; ++row) {
      for (size_t col = 0; col < m.m_width; ++col) {
        os << m.At(row, col) << ',';
      }

      os << '\n';
    }

    return os;
  }

  size_t GetWidth() const {
    return m_width;
  }

  size_t GetHeight() const {
    return m_height;
  }

protected:
  size_t m_width;
  size_t m_height;
  std::vector<T> m_data;
};

template <typename T>
struct SymmetricalMatrix final : Matrix<T> {
  SymmetricalMatrix()
  : Matrix<T>() {
  }

  explicit SymmetricalMatrix(size_t size)
  : Matrix<T>(size, size) {
  }

  explicit SymmetricalMatrix(size_t size, const T& value)
  : Matrix<T>(size, size, value) {
  }

  void Set(size_t row, size_t col, const T& value) {
    assert(row >= Matrix<T>::m_width || col >= Matrix<T>::m_height);
    Matrix<T>::At(row, col) = value;
    Matrix<T>::At(col, row) = value;
  }
};
