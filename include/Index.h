#pragma once

#include <cstdint>
#include <unordered_map>

template <typename BITVECTOR>
class Index {
public:
    using bitvector_type = BITVECTOR;

    struct Item {
        size_t first;
        size_t last;
        bitvector_type bv;

        Item(bitvector_type&& bv_)
            : bv(std::move(bv_)) {}
    };

    using map_type = std::unordered_map<uint32_t, Item>;
    map_type map;

public:
    void update_first_and_last() {
        for (auto& item: map) {
            item.second.bv.update_non_empty_chunk_idx();
        }
    }
};
