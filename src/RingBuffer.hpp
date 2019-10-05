#pragma once

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

template <size_t N>
struct isPowerOfTwo
{
    static const bool value = N && !(N & (N - 1));
};

template <typename DataType, size_t Size>
RingBuffer<DataType, Size>::RingBuffer()
    : mLength(0), mWriteHead(0), mReadHead(0)
{
    static_assert(isPowerOfTwo<Size>::value, "Size must be a power of two.");
    memset(mData, DataType(0), Size * sizeof(DataType));
}

template <typename DataType, size_t Size>
RingBuffer<DataType, Size>::~RingBuffer()
{
}

// -----------------------------------------------------------------------------

template <typename DataType, size_t Size>
inline size_t RingBuffer<DataType, Size>::getLength() const
{
    return mLength;
}

template <typename DataType, size_t Size>
inline bool RingBuffer<DataType, Size>::isEmpty() const
{
    return mLength == 0;
}

template <typename DataType, size_t Size>
inline bool RingBuffer<DataType, Size>::isFull() const
{
    return mLength == Size;
}

// -----------------------------------------------------------------------------

template <typename DataType, size_t Size>
void RingBuffer<DataType, Size>::write(DataType inData)
{
    mData[mWriteHead] = inData;
    mWriteHead = (mWriteHead + 1) & sMask;
    mLength++;
    if (mLength > Size)
    {
        mLength = Size;
        mReadHead = (mReadHead + 1) & sMask;
    }
}

template <typename DataType, size_t Size>
void RingBuffer<DataType, Size>::write(const DataType *inData, size_t inSize)
{
    for (auto i = 0; i < inSize; ++i)
    {
        write(inData[i]);
    }
}

template <typename DataType, size_t Size>
void RingBuffer<DataType, Size>::pop(size_t inNumberOfItems)
{
    for (size_t i = 0; i < inNumberOfItems; ++i)
    {
        read();
    }
}

template <typename DataType, size_t Size>
void RingBuffer<DataType, Size>::clear()
{
    memset(mData, DataType(0), Size * sizeof(DataType));
    mReadHead = 0;
    mWriteHead = 0;
    mLength = 0;
}

// -----------------------------------------------------------------------------

template <typename DataType, size_t Size>
DataType RingBuffer<DataType, Size>::peek(size_t inOffset) const
{
    const auto head = (mReadHead + inOffset) & sMask;
    return mData[head];
}

template <typename DataType, size_t Size>
DataType RingBuffer<DataType, Size>::read()
{
    mLength--;
    if (mLength < 0)
    {
        mLength = 0;
        return 0;
    }
    const DataType data = mData[mReadHead];
    mReadHead = (mReadHead + 1) & sMask;
    return data;
}

template <typename DataType, size_t Size>
void RingBuffer<DataType, Size>::read(DataType *outData, size_t inSize)
{
    for (auto i = 0; i < inSize; ++i)
    {
        outData[i] = read();
    }
}

END_APPLEMIDI_NAMESPACE