#pragma once

#include <vector>
#include <memory>
#include <optional>

#include <cassert>
#include <cstring>


class bitvector_sparse {

public:
    static constexpr bool custom_filter = false;

private:
    static constexpr size_t block_size = 8; // in 64-bit units
    static constexpr size_t bits_in_chunk = 64;
    static constexpr size_t bits_in_block = bits_in_chunk * block_size;

    using block_type = uint64_t[block_size];

protected:
    size_t m_size;
    std::vector<std::unique_ptr<uint64_t[]>> blocks;

    size_t blocks_count() const noexcept {
        return (m_size + bits_in_block - 1) / bits_in_block;
    }

public:
    bitvector_sparse(size_t n)
        : m_size(n)
        , blocks(blocks_count()) {}

    bitvector_sparse(const bitvector_sparse& bv)
        : m_size(bv.m_size) {

        blocks.clear();
        blocks.resize(bv.blocks.size());
        for (size_t i=0; i < bv.blocks.size(); i++) {
            const uint64_t* ptr = bv.blocks[i].get();
            if (ptr) {
                blocks[i].reset(new block_type);
                memcpy(blocks[i].get(), ptr, sizeof(block_type));
            }
        }
    }

    bitvector_sparse(bitvector_sparse&& bv)
        : m_size(bv.m_size)
        , blocks(std::move(bv.blocks)) {}

    bitvector_sparse& operator=(bitvector_sparse&& bv) {

        m_size = bv.m_size;
        blocks.clear();
        blocks.resize(bv.blocks.size());
        for (size_t i=0; i < bv.blocks.size(); i++) {
            auto* ptr = bv.blocks[i].get();
            if (ptr) {
                blocks[i].reset(new block_type);
                memcpy(blocks[i].get(), ptr, sizeof(block_type));
            }
        }

        return *this;
    }

    void set(size_t index) {
    
        const size_t block_id = index / bits_in_block;
        if (blocks[block_id].get() == nullptr) {
            blocks[block_id].reset(new uint64_t[block_size]);
            memset(blocks[block_id].get(), 0, sizeof(block_type));
        }

        const size_t n = (index % bits_in_block) / bits_in_chunk;
        const size_t k = index % bits_in_chunk;

        uint64_t* data = blocks[block_id].get();
        data[n] |= uint64_t(1) << k;;
    }

    bool get(size_t index) const {
    
        const size_t block_id = index / bits_in_block;
        const uint64_t* data = blocks[block_id].get();
        if (data == nullptr) {
            return false;
        }

        const size_t n = (index % bits_in_block) / bits_in_chunk;
        const size_t k = index % bits_in_chunk;

        return (data[n] & (uint64_t(1) << k));
    }

    size_t size() const {
        return m_size;
    }

    size_t cardinality() const {
        size_t k = 0;
        for (const auto& ptr: blocks) {
            const uint64_t* data = ptr.get();
            if (data) {
                for (size_t j=0; j < block_size; j++) {
                    k += __builtin_popcountll(data[j]);
                }
            }

        }

        return k;
    }

    template <typename CALLBACK>
    void visit(CALLBACK callback) const {
        size_t block_id = 0;
        block_id -= bits_in_block;
        for (const auto& ptr: blocks) {
            block_id += bits_in_block;

            const uint64_t* data = ptr.get();
            if (data == nullptr) {
                continue;
            }

            for (size_t i=0; i < block_size; i++) {
                uint64_t tmp = ptr.get()[i];
                size_t k = block_id + i * bits_in_chunk;
                while (tmp) {
                    if (tmp & 0x1) {
                        callback(k);
                    }

                    tmp >>= 1;
                    k += 1;
                }
            }

        }
    }

    void update_internal_structures() {}

public:
    static std::optional<bitvector_sparse> bit_and(const bitvector_sparse& v1, const bitvector_sparse& v2) {
        assert(v1.size() == v2.size());

        bitvector_sparse result(v1.size());

        for (size_t i=0; i < result.blocks_count(); i++) {
            const uint64_t* data1 = v1.blocks[i].get();
            if (data1 == nullptr) {
                continue;
            }

            const uint64_t* data2 = v2.blocks[i].get();
            if (data2 == nullptr) {
                continue;
            }

            result.blocks[i].reset(new block_type);

            uint64_t* res = result.blocks[i].get();
            for (size_t j=0; j < block_size; j++) {
                res[j] = data1[j] & data2[j];
            }
        }

        return result;
    }

    static bool bit_and_inplace(bitvector_sparse& v1, const bitvector_sparse& v2) {
        assert(v1.size() == v2.size());

        for (size_t i=0; i < v1.blocks_count(); i++) {
            uint64_t* data1 = v1.blocks[i].get();
            const uint64_t* data2 = v2.blocks[i].get();
            if (data1 == nullptr || data2 == nullptr) {
                v1.blocks[i].release();
                continue;
            }

            for (size_t j=0; j < block_size; j++) {
                data1[j] &= data2[j];
            }
        }

        return true;
    }
};

