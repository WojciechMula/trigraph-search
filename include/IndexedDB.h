#pragma once

#include "NaiveDB.h"

#include <functional>

template <typename COMBINER>
class IndexedDB: public NaiveDB {
public:
    using bitvector_type = typename COMBINER::bitvector_type;
    using index_type = Index<bitvector_type>;

protected:
    const index_type index;

public:
    IndexedDB(const Collection& rows_, index_type&& index_)
        : NaiveDB(rows_)
        , index(std::move(index_)) {}

public:
    virtual int matches(const std::string& word) const override {

        const size_t n = word.size();

        if (n < 3) {
            return NaiveDB::matches(word);
        }

        if (n == 3) {
            return matches_len3(word);
        }

        COMBINER combiner;

        if (!get_matches_longer(word, combiner)) {
            return 0;
        }

        if constexpr (bitvector_type::custom_filter) {
            return combiner.value().filter_out_false_positives(rows, word);
        } else {
            return filter_out_false_positives(combiner.value(), word);
        }
    }

protected:
    size_t matches_len3(const std::string& word) const {

        assert(word.size() == 3);

        const int32_t b0 = uint8_t(word[0]);
        const int32_t b1 = uint8_t(word[1]);
        const int32_t b2 = uint8_t(word[2]);
        const uint32_t trigram = b0 | (b1 << 8) | (b2 << 16);

        auto it = index.map.find(trigram);
        if (it == index.map.end()) {
            return 0;
        } else {
            return it->second.bv.cardinality();
        }
    }

    bool get_matches_longer(const std::string& word, COMBINER& combiner) const {

        assert(word.size() > 3);

        for (size_t i=0; i < word.size() - 2; i++) {
            const int32_t b0 = uint8_t(word[i + 0]);
            const int32_t b1 = uint8_t(word[i + 1]);
            const int32_t b2 = uint8_t(word[i + 2]);
            const uint32_t trigram = b0 | (b1 << 8) | (b2 << 16);

            auto it = index.map.find(trigram);
            if (it == index.map.end()) {
                return false;
            }

            if (!combiner.add(it->second.bv))
                break;
        }

        return combiner.has_value();
    }

    size_t filter_out_false_positives(const bitvector_type& bv, const std::string& word) const {

        size_t count = 0;
        auto visitor = [&word, &count, this](size_t index) {
            if (std::strstr(rows[index].c_str(), word.c_str())) {
                count += 1;
            }
        };

        bv.visit(visitor);
        return count;
    }

    size_t filter_out_false_positives(size_t index, const std::string& word) const {

        return std::strstr(rows[index].c_str(), word.c_str()) != nullptr; 
    }
};
