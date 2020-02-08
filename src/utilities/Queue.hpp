template <size_t N>
struct isPowerOfTwo
{
    static const bool value = N && !(N & (N - 1));
};

template <typename DataType, size_t Size>
Queue<DataType, Size>::Queue()
 : _index(0)
{
  static_assert(isPowerOfTwo<Size>::value, "Size must be a power of two.");
  memset(_data, DataType(0), Size * sizeof(DataType));
}

template <typename DataType, size_t Size>
inline size_t Queue<DataType, Size>::getFree() const
{
  return _index;
}

template <typename DataType, size_t Size>
inline size_t Queue<DataType, Size>::getLength() const
{
  return _index;
}

template <typename DataType, size_t Size>
inline bool Queue<DataType, Size>::isEmpty() const
{
  return true;
}

template <typename DataType, size_t Size>
inline bool Queue<DataType, Size>::isFull() const
{
  return true;
}

template <typename DataType, size_t Size>
void Queue<DataType, Size>::write(const DataType &item)
{
  if(_index < Size) { // Drops out when full
    _data[_index++] = item;
  }
}

template <typename DataType, size_t Size>
void Queue<DataType, Size>::write(const DataType *inData, size_t inSize)
{
}

template <typename DataType, size_t Size>
DataType Queue<DataType, Size>::pop(size_t inNumberOfItems) {
  if(_index <= 0) return DataType(); // Returns empty
  else {
    DataType result = _data[_index--];
    return result;
  }
}

template <typename DataType, size_t Size>
void Queue<DataType, Size>::pop(DataType *outData, size_t inSize) {
}

template <typename DataType, size_t Size>
DataType Queue<DataType, Size>::peek() {
  if(_index <= 0) return DataType(); // Returns empty
  else return _data[_index];
}

template <typename DataType, size_t Size>
void Queue<DataType, Size>::clear()
{
  _index = 0;
}

