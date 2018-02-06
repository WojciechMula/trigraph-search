#pragma once

#include <memory>
#include <optional>

class bitvector_naive {

protected:
    size_t m_size;
    uint64_t* data;

    size_t chunks_count() const noexcept {
        return (m_size + 63) / 64;
    }

public:
    bitvector_naive(size_t n) : bitvector_naive(n, true) {
        memset(data, 0, chunks_count() * sizeof(uint64_t));
    }

    bitvector_naive(const bitvector_naive& bv)
        : bitvector_naive(bv.m_size, true) {

        memcpy(data, bv.data, chunks_count() * sizeof(uint64_t));
    }

    bitvector_naive(bitvector_naive&& bv)
        : m_size(bv.m_size)
        , data(bv.data) {

        bv.data = nullptr;
    }

    bitvector_naive& operator=(bitvector_naive&& bv) {
        delete[] data;

        m_size  = bv.m_size;
        data    = bv.data;
        bv.data = nullptr;

        return *this;
    }

    ~bitvector_naive() {
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
        for (size_t i=0; i < chunks_count(); i++) {
            k += __builtin_popcountll(data[i]);
        }

        return k;
    }

    template <typename CALLBACK>
    void visit(CALLBACK callback) const {
        for (size_t i=0; i < chunks_count(); i++) {
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

    void update_internal_structures() {}

private:
    bitvector_naive(size_t size, bool /**/)
        : m_size(size)
        , data(new uint64_t[chunks_count()]) {}

public:
    static std::optional<bitvector_naive> bit_and(const bitvector_naive& v1, const bitvector_naive& v2) {
        assert(v1.size() == v2.size());

        bitvector_naive result(v1.size(), true);

        for (size_t i=0; i < result.chunks_count(); i++) {
            result.data[i] = v1.data[i] & v2.data[i];
        }

        return result;
    }

    static bool bit_and_inplace(bitvector_naive& v1, const bitvector_naive& v2) {
        assert(v1.size() == v2.size());

        for (size_t i=0; i < v1.chunks_count(); i++) {
            v1.data[i] &= v2.data[i];
        }

        return true;
    }
};

