#include <vector>
#include <iostream>
#include <numeric>
#include "Atomic.h"

template <typename T>
struct Averager
{
    Averager(size_t numElements, T initialValue)
    {
        resize(numElements, initialValue);
    }
    
    void clear(T initialValue)
    {
        averagedVector.assign(getSize(), initialValue);
        compute();
    }
    
    void resize(size_t s, T initialValue)
    {
        averagedVector.resize(s);
        clear(initialValue);
        compute();
    }
    
    void add(T t)
    {
        auto localIndex = writeIndex.get();
        averagedVector[localIndex] = t;
        localIndex = (localIndex + 1) % averagedVector.size();
        writeIndex.set(localIndex);
        compute();
    }
    
    float getAverage() const
    {
        return atomAverage.get();
    }
    
    size_t getSize() const
    {
        return averagedVector.size();
    }
    
    void compute()
    {
        auto average = atomAverage.get();
        average = std::accumulate(averagedVector.begin(), averagedVector.end(), initialValue) / averagedVector.size();
        atomAverage.set(average);
    }
    
    void print()
    {
        for (auto &i : averagedVector)
        {
            std::cout << i << " ";
        }
        
        std::cout << "\n";
    }
private:
    T initialValue;
    juce::Atomic<float> atomAverage {0.f};
    std::vector<T> averagedVector;
    juce::Atomic<size_t> writeIndex {0};
};

int main()
{
    
    Averager<float> avTest {9, 0.f};
    
    for (float i=-5.f; i < 5.f; i=i+1.0f)
    {
        avTest.add(i);
        std::cout << avTest.getAverage() << std::endl;
    }
    
     avTest.print();
    
    return 0;
    
}
