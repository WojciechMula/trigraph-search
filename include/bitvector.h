#pragma once

#include <memory>
#include <optional>

class bitvector {

protected:
    size_t m_size;
    uint64_t* data;

    struct {
        size_t first = 0;
        size_t last = 0;
    } non_empty_chunk;

    constexpr static size_t chunks_count(size_t size) {
        return (size + 63) / 64;
    }

public:
    bitvector(size_t n) : bitvector(n, true) {
        memset(data, 0, chunks_count(m_size) * sizeof(uint64_t));
    }

    bitvector(const bitvector& bv)
        : bitvector(bv.m_size, true) {

        non_empty_chunk = bv.non_empty_chunk;
        memcpy(data, bv.data, chunks_count(m_size) * sizeof(uint64_t));
    }

    bitvector(bitvector&& bv)
        : m_size(bv.m_size)
        , data(bv.data)
        , non_empty_chunk(bv.non_empty_chunk) {

        bv.data = nullptr;
    }

    bitvector& operator=(bitvector&& bv) {
        delete[] data;

        m_size  = bv.m_size;
        data    = bv.data;
        bv.data = nullptr;

        return *this;
    }

    ~bitvector() {
        delete[] data;
    }

    void set(size_t index) {

        const size_t n = index / 64;
        const size_t k = index % 64;

        data[n] |= uint64_t(1) << k;;
    }

    size_t size() const {
        return m_size;
    }

    size_t cardinality() const {
        size_t k = 0;
        for (size_t i=non_empty_chunk.first; i <= non_empty_chunk.last; i++) {
            k += __builtin_popcountll(data[i]);
        }

        return k;
    }

    template <typename CALLBACK>
    void visit(CALLBACK callback) const {
        for (size_t i=non_empty_chunk.first; i <= non_empty_chunk.last; i++) {
            uint64_t tmp = data[i];

            size_t k = i * 64;
            while (tmp) {
                if (tmp & 0x1) {
                    callback(k);
                }

                tmp >>= 1;
                k += 1;
            }
        }
    }

    void update_non_empty_chunk_idx() {

        bool set_first = true;

        const size_t n = chunks_count(m_size);
        for (size_t i=0; i < n; i++) {
            if (data[i] == 0) {
                continue;
            }

            if (set_first) {
                non_empty_chunk.first = i;
                non_empty_chunk.last  = i;
                set_first = false;
            }
            non_empty_chunk.last = i;
        }
    }

private:
    bitvector(size_t size, bool /**/)
        : m_size(size)
        , data(new uint64_t[chunks_count(m_size)]) {}

public:
    static std::optional<bitvector> bit_and(const bitvector& v1, const bitvector& v2) {
        assert(v1.size() == v2.size());

        const size_t first = std::max(v1.non_empty_chunk.first, v2.non_empty_chunk.first);
        const size_t last  = std::min(v1.non_empty_chunk.last, v2.non_empty_chunk.last);

        if (first > last) {
            return std::nullopt;
        }

        bitvector result(v1.size(), true);

        for (size_t i=0; i < first; i++) {
            result.data[i] = 0;
        }

        bool set_first = true;
        for (size_t i=first; i <= last; i++) {
            const uint64_t tmp = v1.data[i] & v2.data[i];
            result.data[i] = tmp;
            if (tmp != 0) {
                if (set_first) {
                    result.non_empty_chunk.first = i;
                    set_first = false;
                }
                result.non_empty_chunk.last = i;
            }
        }

        const size_t n = chunks_count(result.m_size);
        for (size_t i=last + 1; i < n; i++) {
            result.data[i] = 0;
        }

        if (set_first) {
            return std::nullopt;
        }

        return result;
    }

    static bool bit_and_inplace(bitvector& v1, const bitvector& v2) {
        assert(v1.size() == v2.size());

        const size_t first = std::max(v1.non_empty_chunk.first, v2.non_empty_chunk.first);
        const size_t last  = std::min(v1.non_empty_chunk.last, v2.non_empty_chunk.last);

        if (first > last) {
            return false;
        }

        for (size_t i=0; i < first; i++) {
            v1.data[i] = 0;
        }

        bool set_first = true;
        for (size_t i=first; i <= last; i++) {
            const uint64_t tmp = v1.data[i] & v2.data[i];
            v1.data[i] = tmp;
            if (tmp != 0) {
                if (set_first) {
                    v1.non_empty_chunk.first = i;
                    set_first = false;
                }
                v1.non_empty_chunk.last = i;
            }
        }

        const size_t n = chunks_count(v1.m_size);
        for (size_t i=last + 1; i < n; i++) {
            v1.data[i] = 0;
        }

        return set_first == false;
    }
};

