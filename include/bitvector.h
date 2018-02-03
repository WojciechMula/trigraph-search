#pragma once

#include <memory>
#include <optional>

class bitvector {

    size_t m_size;
    uint64_t* data;

    constexpr static size_t chunks_count(size_t size) {
        return (size + 63) / 64;
    }

public:
    bitvector(size_t n) : bitvector(n, true) {
        memset(data, 0, chunks_count(m_size) * sizeof(uint64_t));
    }

    bitvector(const bitvector& bv) : bitvector(bv.m_size, true) {
        memcpy(data, bv.data, chunks_count(m_size) * sizeof(uint64_t));
    }

    bitvector(bitvector&& bv)
        : m_size(bv.m_size)
        , data(bv.data) { 

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
        size_t n = chunks_count(m_size);
        size_t k = 0;
        for (size_t i=0; i < n; i++) {
            k += __builtin_popcountll(data[i]);
        }
        
        return k;
    }

    template <typename CALLBACK>
    void visit(CALLBACK callback) const {
        size_t n = chunks_count(m_size);
        for (size_t i=0; i < n; i++) {
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

private:
    bitvector(size_t size, bool /**/)
        : m_size(size)
        , data(new uint64_t[chunks_count(m_size)]) {}

public:
    static std::optional<bitvector> bit_and(const bitvector& v1, const bitvector& v2) {
        assert(v1.size() == v2.size());

        bitvector result(v1.size(), true);

        const size_t n = chunks_count(result.m_size);
        size_t zeros = 0;
        for (size_t i=0; i < n; i++) {
            const uint64_t tmp = v1.data[i] & v2.data[i];
            result.data[i] = tmp;
            zeros += (tmp == 0);
        }

        if (zeros == n) {
            return std::nullopt;
        }

        return result;
    }

    static bool bit_and_inplace(bitvector& v1, const bitvector& v2) {
        assert(v1.size() == v2.size());

        const size_t n = chunks_count(v1.m_size);
        size_t zeros = 0;
        for (size_t i=0; i < n; i++) {
            const uint64_t tmp = v1.data[i] & v2.data[i];
            v1.data[i] = tmp;
            zeros += (tmp == 0);
        }

        return (zeros != n);
    }
};

