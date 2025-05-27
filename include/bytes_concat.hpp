#ifndef BYTES_CONCAT_LIB_HPP
#define BYTES_CONCAT_LIB_HPP

#include <utility>
#include <array>
#include <algorithm>

namespace bconcat {

    class TrivialConcatenator {
    public:

        static auto concat(auto&&... args) {
            char bytes[(sizeof(args) + ...)];
            uint64_t s{};
            ((memcpy(bytes + s, &args, sizeof(args)), s += sizeof(args)), ...);

            return std::to_array(bytes);
        }

        template<typename... Args>
        auto operator()(Args&&... args) const {
            return concat(std::forward<Args>(args)...);
        }
    };


    #include <iterator>
    #include <vector>
    #include <concepts>
    #include <type_traits>


    template <typename T>
    concept Iterable = requires(T t) {
        { std::begin(t) } -> std::input_iterator;
        { std::end(t) } -> std::sentinel_for<decltype(std::begin(t))>;
    };


    template<typename T>
    concept MemcpyCopiable = requires(T t){
        {std::is_trivially_copyable_v<T>};
    };

    class UnifiedConcatenator {
    public:
        using value_type = typename std::vector<char>;

        template<typename T> requires Iterable<T>
        static void append(value_type& dst, T&& src)  {
            std::copy(std::begin(src), std::end(src), std::back_inserter(dst));
        }


        template<typename T> requires (!Iterable<T> && MemcpyCopiable<T>)
        static void append(value_type& dst, T&& src) {
            auto curr_sz = dst.size();
            dst.resize(dst.size() + sizeof(src));

            memcpy(dst.data() + curr_sz , &src, sizeof(src));
        }


        template<typename... Args>
        static auto concat(Args&&... args) {
            value_type bytes{};
            ((append(bytes, std::forward<Args>(args))), ...);

            return bytes;
        }


        template<typename... Args>
        auto operator()(Args&&... args) const {
            return concat(std::forward<Args>(args)...);
        }

    };

    // TODO: DeepConcatenator
}


#endif // BYTES_CONCAT_LIB_HPP
