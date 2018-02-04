#pragma once

#include "Index.h"
#include <cstring>

template <typename BITVECTOR>
class Builder final {

public:
    using index_type = Index<BITVECTOR>;
    using bitvector_type = typename index_type::bitvector_type;

private:
    index_type index;
    size_t size;

public:
    Builder(size_t size_)
        : size(size_) {}

    index_type&& capture() {
        index.update_first_and_last();
        return std::move(index);
    }

public:
    template <typename COLLECTION>
    void add(const COLLECTION& collection) {
        assert(size == collection.size());
        size_t i=0;
        for (const auto& str: collection) {
            add(i++, str);
        }
    }

private:
    void add(size_t row, const std::string& str) {
        if (str.size() < 3) {
            return;
        }

        for (size_t i=0; i < str.size() - 2; i++) {
            const int32_t b0 = uint8_t(str[i + 0]);
            const int32_t b1 = uint8_t(str[i + 1]);
            const int32_t b2 = uint8_t(str[i + 2]);
            const uint32_t trigram = b0 | (b1 << 8) | (b2 << 16);
            
            auto it = index.map.find(trigram);
            if (it == index.map.end()) {
                BITVECTOR bv(size);

                it = index.map.insert({trigram, std::move(bv)}).first;
            }

            it->second.bv.set(row);
        }
    }
};
