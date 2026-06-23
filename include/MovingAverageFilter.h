#pragma once

#include <Arduino.h>

/**
 * @brief Fast, non-blocking, O(1) Moving Average Filter using a Ring Buffer.
 * 
 * @tparam T The data type of the filter values (e.g. uint32_t or float)
 * @tparam N The window size (number of samples to average)
 */
template <typename T, size_t N>
class MovingAverageFilter {
private:
    T buffer[N];
    size_t index;
    T sum;
    size_t count;

public:
    MovingAverageFilter() {
        reset();
    }

    /**
     * @brief Adds a new sample to the filter. Updates the sum in O(1) time.
     * 
     * @param val The new sample.
     */
    void add(T val) {
        if (count < N) {
            buffer[index] = val;
            sum += val;
            count++;
        } else {
            sum = sum - buffer[index] + val;
            buffer[index] = val;
        }
        index = (index + 1) % N;
    }

    /**
     * @brief Gets the current moving average.
     * 
     * @return T The computed average.
     */
    T get() const {
        if (count == 0) return 0;
        return sum / static_cast<T>(count);
    }

    /**
     * @brief Resets the filter buffer, sum, and count to zero.
     */
    void reset() {
        for (size_t i = 0; i < N; i++) {
            buffer[i] = 0;
        }
        index = 0;
        sum = 0;
        count = 0;
    }
};
