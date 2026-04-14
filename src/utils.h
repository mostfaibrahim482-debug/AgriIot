#pragma once
#include <Arduino.h>
#include <math.h>

template <typename T>
T clampValue(T value, T low, T high)
{
    if (value < low)
        return low;
    if (value > high)
        return high;
    return value;
}

inline float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    if (fabs(in_max - in_min) < 0.0001f)
        return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline void insertionSortInt(int *arr, int n)
{
    for (int i = 1; i < n; i++)
    {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

inline int medianOfSortedInt(const int *arr, int n)
{
    if (n <= 0)
        return 0;
    if (n % 2 == 1)
        return arr[n / 2];
    return (arr[n / 2 - 1] + arr[n / 2]) / 2;
}