#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#define MIN(a, b)  ((a) < (b) ? (a) : (b))


template <typename  T>
class RingBuffer
{
public:
    RingBuffer(unsigned n=4096);
    ~RingBuffer();

    void addData(T* samples,unsigned num);
    T getData(unsigned n);
    void pushRd(unsigned n);
    int read(T* out, int maxLen);

    unsigned getLen() const;
private:
    T* bufferData;
    unsigned rd;
    unsigned len;
    unsigned size;
};

template <typename T>
RingBuffer<T>::RingBuffer(unsigned n)
{
    bufferData = new T[n];
    rd = 0;
    len = 0;
    size = n;
}

template <typename T>
RingBuffer<T>::~RingBuffer()
{
    delete[] bufferData;
}

template <typename T>
void RingBuffer<T>::addData(T* samples,unsigned num)
{
    if(num <= 0) return;

    //计算剩余空间
    unsigned space = size - len;
    //依序赋值
    for(int i = 0; i < num ; i++)
    {
        bufferData[(i + rd + len) % size] = samples[i];
    }
    //添加数小于剩余空间，len直接加
    if(num < space)
    {
        len += num;
    }
    //添加数大于剩余空间，len=size
    else{
        rd = (rd + len + num) % size;
        len = size;
    }
}

template <typename T>
T RingBuffer<T>::getData(unsigned n)
{
    return bufferData[(rd + n) % size];
}

template <typename T>
int RingBuffer<T>::read(T* out, int maxLen)
{
    int count = MIN(maxLen, getLen());

    for (int i = 0; i < count; ++i) {
        out[i] = bufferData[rd];
        rd = (rd + 1) % size;
    }
    return count;
}

template <typename T>
void RingBuffer<T>::pushRd(unsigned n)
{
    if(n <= len){
        rd = (rd + n) % size;
        len -= n;
    }else {
        rd = (rd + len) % size;
        len = 0;
    }
}

template <typename T>
unsigned RingBuffer<T>::getLen() const
{
    return len;
}

#endif // RINGBUFFER_H
