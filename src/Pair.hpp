#pragma once

template <typename T1, typename T2>
struct Pair
{
    T1 first;
    T2 second;

    friend bool operator==(const Pair<T1, T2>& lhs, const Pair<T1, T2>& rhs) {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }

    friend bool operator!=(const Pair<T1, T2>& lhs, const Pair<T1, T2>& rhs) {
        return (lhs.first != rhs.first) || (lhs.second != rhs.second);
    }
};
