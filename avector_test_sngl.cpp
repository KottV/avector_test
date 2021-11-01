#include <vector>
#include <iostream>
#include <numeric>
#include <atomic>

namespace juce
{
    namespace AtomicHelpers
    {
        template <typename T> struct DiffTypeHelper     { using Type = T; };
        template <typename T> struct DiffTypeHelper<T*> { using Type = std::ptrdiff_t; };
    }
    template <typename Type>
    struct Atomic  final
    {
        using DiffType = typename AtomicHelpers::DiffTypeHelper<Type>::Type;
        Atomic() noexcept  : value (Type()) {}
        Atomic (Type initialValue) noexcept  : value (initialValue) {}
        Atomic (const Atomic& other) noexcept  : value (other.get()) {}
        ~Atomic() noexcept
        {
#if __cpp_lib_atomic_is_always_lock_free
            static_assert (std::atomic<Type>::is_always_lock_free,
                           "This class can only be used for lock-free types");
#endif
        }
        Type get() const noexcept               { return value.load(); }
        void set (Type newValue) noexcept       { value = newValue; }
        Type exchange (Type newValue) noexcept  { return value.exchange (newValue); }
        bool compareAndSetBool (Type newValue, Type valueToCompare) noexcept
        {
            return value.compare_exchange_strong (valueToCompare, newValue);
        }
        
        Atomic<Type>& operator= (const Atomic& other) noexcept
        {
            value = other.value.load();
            return *this;
        }
        
        Atomic<Type>& operator= (Type newValue) noexcept
        {
            value = newValue;
            return *this;
        }
        Type operator+= (DiffType amountToAdd) noexcept { return value += amountToAdd; }
        Type operator-= (DiffType amountToSubtract) noexcept { return value -= amountToSubtract; }
        Type operator++() noexcept { return ++value; }
        Type operator--() noexcept { return --value; }
        void memoryBarrier() noexcept          { atomic_thread_fence (std::memory_order_seq_cst); }
        std::atomic<Type> value;
    };
} //end namespace juce

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
