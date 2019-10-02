#pragma once

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

template <typename DataType, size_t Size>
class RingBuffer
{
private:
    static const size_t sMask = Size - 1;

public:
    RingBuffer();
    ~RingBuffer();

public:
    inline size_t getLength() const;
    inline bool isEmpty() const;
    inline bool isFull() const;

public:
    void write(DataType inData);
    void write(const DataType *inData, size_t inSize);
    void pop(size_t inNumberOfItems = 1);
    void clear();

public:
    DataType peek(size_t inOffset = 0) const;
    DataType read();
    void read(DataType *outData, size_t inSize);

private:
    DataType mData[Size];
    size_t mLength;
    size_t mWriteHead;
    size_t mReadHead;
};

END_APPLEMIDI_NAMESPACE

#include "RingBuffer.hpp"
