#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#define MIN(a, b)  ((a) < (b) ? (a) : (b))


template <typename  T>
class RingBuffer
{
public:
    RingBuffer(unsigned n=4096);
    ~RingBuffer();

    void write(T* data,unsigned count); ///< 写入指定长的数据
    T at(unsigned index);               ///< 查看指定位置的数据，从当前读指针开始
    void advanceReadPos(unsigned n);    ///< 推进读指针
    void clear();

    unsigned dataSize() const;
private:
    T*  m_buffer = nullptr;
    unsigned m_readPos;
    unsigned m_dataSize;  ///< 现存的数据量
    unsigned m_capacity;  ///< 最大能存放的数量
};

template <typename T>
RingBuffer<T>::RingBuffer(unsigned n)
{
    m_buffer = new T[n];
    m_readPos = 0;
    m_dataSize = 0;
    m_capacity = n;
}

template <typename T>
RingBuffer<T>::~RingBuffer()
{
    delete[] m_buffer;
}

template <typename T>
void RingBuffer<T>::write(T* data,unsigned count)
{
    if(count <= 0) return;

    //计算剩余空间
    unsigned space = m_capacity - m_dataSize;
    //依序赋值
    for(unsigned i = 0; i < count ; i++)
    {
        m_buffer[(i + m_readPos + m_dataSize) % m_capacity] = data[i];
    }
    //添加数小于剩余空间，len直接加
    if(count < space)
    {
        m_dataSize += count;
    }
    //添加数大于剩余空间，len=size
    else{
        m_readPos  = (m_readPos + m_dataSize + count) % m_capacity;
        m_dataSize = m_capacity;
    }
}

template <typename T>
T RingBuffer<T>::at(unsigned index)
{
    T retData;
    retData = m_buffer[(m_readPos + index) % m_capacity];
    return retData;
}



template <typename T>
void RingBuffer<T>::advanceReadPos(unsigned n)
{
    if(n <= m_dataSize){
        m_readPos = (m_readPos + n) % m_capacity;
        m_dataSize -= n;
    }else {
        m_readPos = (m_readPos + m_dataSize) % m_capacity;
        m_dataSize = 0;
    }
}

template <typename T>
void RingBuffer<T>::clear()
{
    m_readPos  = 0;
    m_dataSize = 0;
}

template <typename T>
unsigned RingBuffer<T>::dataSize() const
{
    return m_dataSize;
}

#endif // RINGBUFFER_H
