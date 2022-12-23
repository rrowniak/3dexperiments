#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <vector>
#include <chrono>

class Timer
{
public:
    Timer()
    {
        begin = last = Time::now();
    }
    double getTime() const
    {
        auto t = Time::now();
        fsec fs = t - begin;
        return fs.count();
    }
    double getDelta()
    {
        auto t = Time::now();
        fsec fs = t - last;
        last = t;
        return fs.count();
    }
private:
    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::duration<double> fsec;

    Time::time_point begin;
    Time::time_point last;
};

template<typename T>
size_t vecsizeof(const typename std::vector<T>& vec)
{
    return sizeof(T) * vec.size();
}

#endif // _UTILS_HPP_