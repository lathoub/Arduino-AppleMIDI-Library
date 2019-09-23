#pragma once

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

    template<typename DataType, int Size>
    class RingBuffer
    {
    private:
        static const int sMask = Size - 1;

    public:
        RingBuffer();
        ~RingBuffer();

    public:
        inline int getLength() const;
        inline bool isEmpty() const;

    public:
        void write(DataType inData);
        void write(const DataType* inData, int inSize);
        void pop(int inNumberOfItems = 1);
        void clear();

    public:
        DataType peek(int inOffset = 0) const;
        DataType read();
        void read(DataType* outData, int inSize);

    private:
        DataType mData[Size];
        int mLength;
        int mWriteHead;
        int mReadHead;
    };
    
END_APPLEMIDI_NAMESPACE

#include "RingBuffer.hpp"
