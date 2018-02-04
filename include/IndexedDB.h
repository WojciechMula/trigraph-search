#pragma once

#include "NaiveDB.h"

#include <functional>

template <typename BITVECTOR>
class IndexedDB: public NaiveDB {
public:
    using bitvector_type = BITVECTOR;
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

        auto bv = get_matches_longer(word);
        if (bv.has_value()) {
            return filter_out_false_positives(bv.value(), word);
        }

        return 0;
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

    std::optional<bitvector_type> get_matches_longer(const std::string& word) const {

        assert(word.size() > 3);

        const bitvector_type* first = nullptr;
        std::optional<bitvector_type> result;

        for (size_t i=0; i < word.size() - 2; i++) {
            const int32_t b0 = uint8_t(word[i + 0]);
            const int32_t b1 = uint8_t(word[i + 1]);
            const int32_t b2 = uint8_t(word[i + 2]);
            const uint32_t trigram = b0 | (b1 << 8) | (b2 << 16);

            auto it = index.map.find(trigram);
            if (it == index.map.end()) {
                return std::nullopt;
            }

            if (first == nullptr) {
                first = &it->second.bv;
            } else if (!result.has_value()) {
                result = bitvector_type::bit_and(*first, it->second.bv);
                if (!result.has_value()) {
                    return std::nullopt;
                }
            } else {
                bool nonempty = bitvector_type::bit_and_inplace(result.value(), it->second.bv);
                if (!nonempty) {
                    return std::nullopt;
                }
            }
        }

        assert(result.has_value());

        return std::move(result);
    }

    size_t filter_out_false_positives(const bitvector_type& bv, const std::string& word) const {

        size_t count = 0;
        auto visitor = [&word, &count, this](size_t index) {
            if (rows[index].find(word) != std::string::npos) {
                count += 1;
            }
        };

        bv.visit(visitor);
        return count;
    }
};
