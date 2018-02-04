#pragma once

#include <cstdint>
#include <unordered_map>

template <typename BITVECTOR>
class Index {
public:
    using bitvector_type = BITVECTOR;

    struct Item {
        mutable std::optional<size_t> cardinality;
        bitvector_type bv;

        Item(bitvector_type&& bv_)
            : bv(std::move(bv_)) {}

        size_t get_cardinality() const {
            if (!cardinality.has_value()) {
                cardinality = bv.cardinality();
            }

            return cardinality.value();
        }
    };

    using map_type = std::unordered_map<uint32_t, Item>;
    map_type map;

public:
    size_t size() const {
        return map.size();
    }

public:
    void update_first_and_last() {
        for (auto& item: map) {
            item.second.bv.update_non_empty_chunk_idx();
        }
    }
};
