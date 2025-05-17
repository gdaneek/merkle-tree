
#pragma once


template<typename T>
constexpr inline T ilog2(T x) {	// x > 0
    return x? 63 - __builtin_clzll(x) : -1;
}

template<typename T>
constexpr inline T round_to_even(T in) {
    return in + (in & 1);
}



 std::ostream& operator<<(std::ostream& os, const std::array<uint8_t, sizeof(uint64_t)>& arg) {
        os << *reinterpret_cast<uint64_t const * const >(arg.data());
        return os;
    }
