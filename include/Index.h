#pragma once

#include <cstdint>
#include <unordered_map>

template <typename BITVECTOR>
class Index {
public:
    using bitvector_type = BITVECTOR;
    using map_type = std::unordered_map<uint32_t, bitvector_type>;
    map_type map;
};
