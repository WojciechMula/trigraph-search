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

    size_t size_in_bytes() const {
        size_t total = 0;

        total += sizeof(map);
        total += map.size() * sizeof(Item);
        for (const auto& item: map) {
            total += item.second.bv.size_in_bytes();
        }

        return total;
    }

public:
    void update_internal_structures() {
        for (auto& item: map) {
            item.second.bv.update_internal_structures();
        }
    }
};
