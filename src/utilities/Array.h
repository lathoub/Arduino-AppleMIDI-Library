// ----------------------------------------------------------------------------
// Array.h
//
//
// Authors:
// Peter Polidoro peterpolidoro@gmail.com
// ----------------------------------------------------------------------------
#ifndef ARRAY_H
#define ARRAY_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <cstddef>
#endif


template <typename T,
  size_t MAX_SIZE>
class Array
{
public:
  Array();
  Array(const T & value);
  template <typename U,
    size_t N>
  Array(const U (&values)[N]);
  template <typename U,
    size_t N>
  Array(const Array<U,N> & values);
  const T & operator[](size_t index) const;
  T & operator[](size_t index);
  const T & at(size_t index) const;
  T & at(size_t index);
  T & front();
  T & back();
  void clear();
  template <typename U>
  void fill(const U & value);
  template <typename U,
    size_t N>
  void fill(const U (&values)[N]);
  template <typename U,
    size_t N>
  void fill(const Array<U,N> & values);
  template <typename U>
  void assign(size_t n,
    const U & value);
  template <typename U,
    size_t N>
  void assign(size_t n,
    const U (&values)[N]);
  template <typename U,
    size_t N>
  void assign(size_t n,
    const Array<U,N> & values);
  void push_back(const T & value);
  void pop_back();
  void remove(size_t index);
  size_t size() const;
  size_t max_size() const;
  bool empty() const;
  bool full() const;
  size_t free() const;
  const T * data() const;
  T * data();

  T * begin();
  T * end();

private:
  T values_[MAX_SIZE];
  size_t size_;
};

#include "Array.hpp"

#endif
