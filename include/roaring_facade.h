#pragma once

#include <roaring.hh>

#include <memory>
#include <optional>
#include <limits>


class roaring_facade {
    Roaring roaring;
    size_t m_size;

public:
    roaring_facade(size_t n) : m_size(n) {}

    void set(size_t index) {
        assert(index < std::numeric_limits<uint32_t>::max());
        roaring.add(index);
    }

    void update_internal_structures() {}

    size_t cardinality() const {
        return roaring.cardinality();
    }

    size_t size() const {
        return m_size;
    }

    template <typename CALLBACK>
    void visit(CALLBACK callback) const {
        // I don't know how to call a template-defined callable
        // inside a C function.
        // this works:   roaring.iterate([](uint32_t, void*){return true;}, nullptr);
        // this doesn't: roaring.iterate([&callback](uint32_t, void*){return true;}, nullptr);
    }

public:
    static std::optional<roaring_facade> bit_and(const roaring_facade& v1, const roaring_facade& v2) {
        assert(v1.size() == v2.size());
        
        roaring_facade result(v1.size());
        result.roaring = v1.roaring;
        result.roaring &= v2.roaring;

        return result;
    }

    static bool bit_and_inplace(roaring_facade& v1, const roaring_facade& v2) {

        assert(v1.size() == v2.size());
        
        v1.roaring &= v2.roaring;

        return !v1.roaring.isEmpty();
    }
};
