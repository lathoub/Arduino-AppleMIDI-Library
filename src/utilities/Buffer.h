#pragma once

#include "../AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

template <typename DataType, size_t Size>
class Buffer
{
public:
    Buffer();
    ~Buffer();

public:
    inline size_t getFree() const;
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
};

END_APPLEMIDI_NAMESPACE

#include "Buffer.hpp"
