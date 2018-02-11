#pragma once

#include <vector>
#include <algorithm>

class vector_facade {
    std::vector<uint32_t> indices;
    size_t m_size;
    ssize_t last_set = -1;

public:
    vector_facade(size_t n) : m_size(n) {}

    void set(size_t index) {
        if (ssize_t(index) == last_set) {
            return;
        }

        assert(ssize_t(index) > last_set);
        last_set = index;
        indices.push_back(index);
    }

    void update_internal_structures() {}

    size_t cardinality() const {
        return indices.size();
    }

    size_t size() const {
        return m_size;
    }

    template <typename CALLBACK>
    void visit(CALLBACK callback) const {
        for (auto index: indices) {
            callback(index);
        }
    }

public:
    static std::optional<vector_facade> bit_and(const vector_facade& v1, const vector_facade& v2) {

        return bit_and_aux(v1, v2);
    }

    static bool bit_and_inplace(vector_facade& v1, const vector_facade& v2) {
        
        auto tmp = bit_and_aux(v1, v2);
        v1.indices = std::move(tmp.indices);

        return v1.cardinality() > 0;
    }

private:
    static vector_facade bit_and_aux(const vector_facade& v1, const vector_facade& v2) {
        assert(v1.size() == v2.size());
        
        vector_facade result(v1.size());

        std::set_intersection(v1.indices.begin(), v1.indices.end(),
                              v2.indices.begin(), v2.indices.end(),
                              std::back_inserter(result.indices));

        return result;
    }
};
