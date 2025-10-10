#pragma once
#include <functional>

namespace purah { namespace addserv {

    template <typename T>
    inline void hash_combine(size_t& hash, const T& obj) {
        hash ^= std::hash<T>{}(obj) + 0x9e3779b9 + (hash << 6) + (hash >> 2); 
    }

} }