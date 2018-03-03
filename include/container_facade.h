#pragma once

#include <algorithm>


template <typename CONTAINER, typename INSERTER>
void intersect_aux(const CONTAINER& a, const CONTAINER& b, INSERTER output)
{
    auto it = b.begin();
    for (const auto& x: a) {
        it = std::lower_bound(it, b.end(), x);
        if (it == b.end()) {
            return;
        }

        if (*it == x) {
            output = x;
        }
    }
}

uint64_t log2(const uint64_t x) {
    if (x == 0)
        return 0;
    
    return (63 - __builtin_clzl(x));
}

template <typename CONTAINER, typename INSERTER>
void intersect(const CONTAINER& a, const CONTAINER& b, INSERTER output)
{
    // the cost of linear search
    const size_t lin = a.size() + b.size();

    // the cost of binary search
    const size_t bin1 = a.size() * log2(b.size());
    const size_t bin2 = b.size() * log2(a.size());

    if (bin1 < lin || bin2 < lin) {
        if (bin1 < bin2)
            intersect_aux(a, b, output);
        else
            intersect_aux(b, a, output);
    }
    else {
        std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), output);
    }
}


template <template<typename> class CONTAINER, bool append = true, bool has_size = true, bool has_resize = true>
class container_facade {

public:
    static constexpr bool custom_filter = false;

private:
    CONTAINER<uint32_t> indices;
    size_t m_size;
    ssize_t last_set = -1;

public:
    container_facade(size_t n) : m_size(n) {}

    void set(size_t index) {
        if (ssize_t(index) == last_set) {
            return;
        }

        assert(ssize_t(index) > last_set);
        last_set = index;
        if constexpr (append)
            indices.push_back(index);
        else
            indices.push_front(index);
    }

    void update_internal_structures() {}

    size_t cardinality() const {
        if constexpr (!has_size) {
            size_t n = 0;
            auto first = indices.begin();
            auto last  = indices.end();

            for (; first != last; first++) {
                n += 1;
            }

            return n;
        } else 
            return std::size(indices);
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
    static std::optional<container_facade> bit_and(const container_facade& v1, const container_facade& v2) {

        return bit_and_aux(v1, v2);
    }

    static bool bit_and_inplace(container_facade& v1, const container_facade& v2) {
        
        auto tmp = bit_and_aux(v1, v2);
        v1.indices = std::move(tmp.indices);

        return v1.cardinality() > 0;
    }

private:
    static container_facade bit_and_aux(const container_facade& v1, const container_facade& v2) {
        assert(v1.size() == v2.size());
        
        container_facade result(v1.size());
        if constexpr (has_resize) {
            result.indices.reserve(std::min(v1.cardinality(), v2.cardinality()));
        }

        auto get_inserter = [&result]() {
            if constexpr (append)
                return std::back_inserter(result.indices);
            else
                return std::front_inserter(result.indices);
        };

        if constexpr (has_size) {
            intersect(v1.indices, v2.indices, get_inserter());
        } else {
            std::set_intersection(v1.indices.begin(), v1.indices.end(),
                                  v2.indices.begin(), v2.indices.end(),
                                  get_inserter());
        }

        return result;
    }
};
