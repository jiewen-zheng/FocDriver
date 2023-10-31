//
// Created by moon on 2023/10/29.
//

#ifndef FIFO_ARRAY_H
#define FIFO_ARRAY_H

#include <cstddef>

template<typename T>
class FifoArray {
public:
    FifoArray();
    explicit FifoArray(size_t size, bool cover = true);
    ~FifoArray();

    bool linkMemory(T *t, size_t size);

    size_t available();
    bool write(T t);
    bool write(T *t, size_t size);
    T read();
    size_t read(T *t, size_t size);
    void clear();

private:
    bool   release = false;
    T      *memory = nullptr;
    bool   cover   = false;
    size_t _size   = 0;
    size_t occupy  = 0;

    T *writePtr = nullptr;
    T *readPtr  = nullptr;
};

template<typename T>
FifoArray<T>::FifoArray() {
    release = false;
    memory = nullptr;
    this->_size = 0;
    this->cover = false;
    occupy = 0;

    writePtr = nullptr;
    readPtr  = nullptr;
}

template<typename T>
FifoArray<T>::FifoArray(size_t size, bool cover) {
    T *tmp = new T[size];
    if (!tmp)
        return;

    release = true;
    memory  = tmp;
    this->_size = size;
    this->cover = cover;
    occupy = 0;

    writePtr = memory;
    readPtr  = memory;
}

template<typename T>
FifoArray<T>::~FifoArray() {
    if (release) delete[]memory;
    memory = nullptr;
}

template<typename T>
bool FifoArray<T>::linkMemory(T *t, size_t size) {
    if (!t)
        return false;

    memory   = t;
    _size    = size;
    writePtr = memory;
    readPtr  = memory;
    return true;
}

template<typename T>
size_t FifoArray<T>::available() {
    return occupy;
}

template<typename T>
bool FifoArray<T>::write(T t) {
    if (!memory) return false;

    size_t unoccupied = _size - occupy;

    if (unoccupied == 0 && !cover)
        return false;

    *writePtr++ = t;

    if (writePtr >= memory + _size) {
        writePtr = memory;
    }

    occupy++;
    return true;
}

template<typename T>
bool FifoArray<T>::write(T *t, size_t size) {
    if (!memory) return false;

    size_t unoccupied = _size - occupy;
    if (unoccupied == 0 && !cover)
        return false;

    if (unoccupied < size) size = cover ? size : unoccupied;

    while (size--) {
        *writePtr++ = *t++;

        if (writePtr >= memory + _size) {
            writePtr = memory;
        }
        occupy      = (occupy < _size) ? occupy + 1 : size;
    }

    return true;
}

template<typename T>
T FifoArray<T>::read() {
    if (!memory || occupy == 0) return false;

    T data = *readPtr++;
    if (readPtr >= memory + _size) {
        readPtr = memory;
    }
    occupy--;
    return data;
}

template<typename T>
size_t FifoArray<T>::read(T *t, size_t size) {
    if (!memory || occupy == 0) return 0;

    size_t read_size = (size > occupy) ? occupy : size;
    size = read_size;

    while (read_size--) {
        *t++ = *readPtr++;
        if (readPtr >= memory + _size) {
            readPtr = memory;
        }
        occupy--;
    }

    return size;
}

template<typename T>
void FifoArray<T>::clear() {
    occupy   = 0;
    writePtr = memory;
    readPtr  = memory;
}

#endif //FIFO_ARRAY_H
