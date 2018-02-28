#pragma once

// Perform intersection on all incoming bitvectors.
template <typename BITVECTOR>
class AndAll {

public:
    using bitvector_type = BITVECTOR;
    
private:
    std::optional<bitvector_type> result;
    const bitvector_type* first = nullptr;

public:
    bool add(const bitvector_type& bv) {
        if (first == nullptr) {
            first = &bv;
        } else if (!result.has_value()) {
            result = bitvector_type::bit_and(*first, bv);
        } else {
            if (!bitvector_type::bit_and_inplace(result.value(), bv)) {
                result = std::nullopt;
                return false;
            }
        }
        return true;
    }

    bool has_value() const {
        return result.has_value();
    }

    const bitvector_type& value() const {
        return result.value();
    }
};
