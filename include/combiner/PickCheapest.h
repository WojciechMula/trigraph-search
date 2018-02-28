#pragma once

// Choose the bitmap with the mininum cardinality.
// It requires a fast cardinality function.
template <typename BITVECTOR>
class PickCheapest {

public:
    using bitvector_type = BITVECTOR;
    
private:
    const bitvector_type* result = nullptr;
    size_t cardinality;

public:
    bool add(const bitvector_type& bv) {

        const size_t tmp = bv.cardinality();

        if (result == nullptr) {
            result = &bv;
            cardinality = tmp;
        } else if (tmp < cardinality) {
            result = &bv;
            cardinality = tmp;
        }

        return true;
    }

    bool has_value() const {
        return result != nullptr;
    }

    const bitvector_type& value() const {
        return *result;
    }
};
