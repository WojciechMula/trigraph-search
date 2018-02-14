#pragma once

#include <limits>

template <typename BITVECTOR>
class IndexedDB2: public IndexedDB<BITVECTOR> {

    using super = IndexedDB<BITVECTOR>;

public:
    using bitvector_type = typename super::bitvector_type;
    using index_type = typename super::index_type;

public:
    IndexedDB2(const Collection& rows_, index_type&& index_)
        : super(rows_, std::move(index_)) {}

public:
    virtual int matches(const std::string& word) const override {

        const size_t n = word.size();

        if (n < 3) {
            return NaiveDB::matches(word);
        }

        if (n == 3) {
            return super::matches_len3(word);
        }

        auto it = find_smallest_bitvector(word);
        if (!it.has_value()) {
            return 0;
        }

        if constexpr (bitvector_type::custom_filter) {
            return it.value()->second.bv.filter_out_false_positives(super::rows, word);
        } else {
            return super::filter_out_false_positives(it.value()->second.bv, word);
        }
    }

protected:
    std::optional<typename index_type::map_type::const_iterator>
    find_smallest_bitvector(const std::string& word) const {

        size_t cardinality = std::numeric_limits<size_t>::max();
        std::optional<typename index_type::map_type::const_iterator> min;

        for (size_t i=0; i < word.size() - 2; i++) {
            const int32_t b0 = uint8_t(word[i + 0]);
            const int32_t b1 = uint8_t(word[i + 1]);
            const int32_t b2 = uint8_t(word[i + 2]);
            const uint32_t trigram = b0 | (b1 << 8) | (b2 << 16);

            auto it = super::index.map.find(trigram);
            if (it == super::index.map.end()) {
                return std::nullopt;
            }

            if (const auto c = it->second.get_cardinality(); c < cardinality) {
                min = it;
                cardinality = c;
            }
        }

        return min;
    }
};

