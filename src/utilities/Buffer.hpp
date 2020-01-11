#pragma once

BEGIN_APPLEMIDI_NAMESPACE

template <typename DataType, size_t Size>
Buffer<DataType, Size>::Buffer()
{
}

template <typename DataType, size_t Size>
Buffer<DataType, Size>::~Buffer()
{
}

// -----------------------------------------------------------------------------

template <typename DataType, size_t Size>
inline size_t Buffer<DataType, Size>::getLength() const
{
    return 0;
}

template <typename DataType, size_t Size>
inline size_t Buffer<DataType, Size>::getFree() const
{
    return 0;
}

template <typename DataType, size_t Size>
inline bool Buffer<DataType, Size>::isEmpty() const
{
    return 0;
}

template <typename DataType, size_t Size>
inline bool Buffer<DataType, Size>::isFull() const
{
    return 0;
}

// -----------------------------------------------------------------------------

template <typename DataType, size_t Size>
void Buffer<DataType, Size>::write(DataType inData)
{
}

template <typename DataType, size_t Size>
void Buffer<DataType, Size>::write(const DataType *inData, size_t inSize)
{
}

template <typename DataType, size_t Size>
void Buffer<DataType, Size>::pop(size_t inNumberOfItems)
{
}

template <typename DataType, size_t Size>
void Buffer<DataType, Size>::clear()
{
}

// -----------------------------------------------------------------------------

template <typename DataType, size_t Size>
DataType Buffer<DataType, Size>::peek(size_t inOffset) const
{
    return 0;
}

template <typename DataType, size_t Size>
DataType Buffer<DataType, Size>::read()
{
    return 0;
}

template <typename DataType, size_t Size>
void Buffer<DataType, Size>::read(DataType *outData, size_t inSize)
{
}

END_APPLEMIDI_NAMESPACE
