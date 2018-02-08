#include "bitvector_sparse.h"

#include <cassert>

void test_basic_operations() {
    bitvector_sparse bv(1000);

    assert(bv.cardinality() == 0);
    for (size_t i=0; i < bv.size(); i++) {
        bv.set(i);
        assert(bv.cardinality() == (i + 1));
    }
}


void test_copy_constructor() {
    bitvector_sparse bv(1024);
    bv.set(1);
    bv.set(100);
    bv.set(1000);

    bitvector_sparse bv1 = bv;
    assert(bv.cardinality() == bv1.cardinality());
    assert(bv1.get(1) == true);
    assert(bv1.get(100) == true);
    assert(bv1.get(1000) == true);

    bitvector_sparse bv2(bv);
    assert(bv.cardinality() == bv2.cardinality());
    assert(bv2.get(1) == true);
    assert(bv2.get(100) == true);
    assert(bv2.get(1000) == true);
}


void test_move_constructor() {
    bitvector_sparse bv(4096);
    bv.set(1);
    bv.set(100);
    bv.set(2000);
    bv.set(3000);
    bv.set(4095);

    bitvector_sparse bv1(std::move(bv));
    assert(bv1.cardinality() == 5);
    assert(bv1.get(1) == true);
    assert(bv1.get(100) == true);
    assert(bv1.get(2000) == true);
    assert(bv1.get(3000) == true);
    assert(bv1.get(4095) == true);
}


void test_move_operator() {
    bitvector_sparse bv(4096);
    bv.set(1);
    bv.set(100);
    bv.set(2000);
    bv.set(3000);
    bv.set(4095);

    bitvector_sparse bv1 = std::move(bv);
    assert(bv1.cardinality() == 5);
    assert(bv1.get(1) == true);
    assert(bv1.get(100) == true);
    assert(bv1.get(2000) == true);
    assert(bv1.get(3000) == true);
    assert(bv1.get(4095) == true);
}


void test_visit() {
    bitvector_sparse bv(4096);
    bv.set(1);
    bv.set(100);
    bv.set(2000);
    bv.set(3000);
    bv.set(4095);

    std::vector<size_t> items;
    auto visitor = [&items](const size_t k) {
        items.push_back(k);
    };

    bv.visit(visitor);

    assert(items.size() == 5);
    assert(items[0] == 1);
    assert(items[1] == 100);
    assert(items[2] == 2000);
    assert(items[3] == 3000);
    assert(items[4] == 4095);
}


void test_and__case_1() {
    bitvector_sparse bv1(1024);
    bitvector_sparse bv2(1024);

    auto bv = bitvector_sparse::bit_and(bv1, bv2);
    assert(bv.has_value());
    assert(bv.value().cardinality() == 0);
}


void test_and__case_2() {
    bitvector_sparse bv1(1024);
    bitvector_sparse bv2(1024);

    bv1.set(1);
    bv1.set(100);
    bv1.set(500);

    bv2.set(10);
    bv2.set(600);
    bv2.set(1000);

    auto bv = bitvector_sparse::bit_and(bv1, bv2);
    assert(bv.has_value());
    assert(bv.value().cardinality() == 0);
}


void test_and__case_3() {
    bitvector_sparse bv1(1024);
    bitvector_sparse bv2(1024);

    bv1.set(1);
    bv1.set(10);
    bv1.set(50);
    bv1.set(99);
    bv1.set(1000);

    bv2.set(1);
    bv2.set(2);
    bv2.set(3);
    bv2.set(100);
    bv2.set(1000);

    auto bv = bitvector_sparse::bit_and(bv1, bv2).value();
    assert(bv.cardinality() == 2);
    assert(bv.get(1) == true);
    assert(bv.get(1000) == true);
}


void test_and_inplace__case_1() {
    bitvector_sparse bv1(1024);
    bitvector_sparse bv2(1024);

    bitvector_sparse::bit_and_inplace(bv1, bv2);
    assert(bv1.cardinality() == 0);
}


void test_and_inplace__case_2() {
    bitvector_sparse bv1(1024);
    bitvector_sparse bv2(1024);

    bv1.set(1);
    bv1.set(100);
    bv1.set(500);

    bv2.set(10);
    bv2.set(600);
    bv2.set(1000);

    bitvector_sparse::bit_and_inplace(bv1, bv2);
    assert(bv1.cardinality() == 0);
    assert(bv2.cardinality() == 3);
}


void test_and_inplace__case_3() {
    bitvector_sparse bv1(1024);
    bitvector_sparse bv2(1024);

    bv1.set(1);
    bv1.set(10);
    bv1.set(50);
    bv1.set(99);
    bv1.set(1000);

    bv2.set(1);
    bv2.set(2);
    bv2.set(3);
    bv2.set(100);
    bv2.set(1000);

    bitvector_sparse::bit_and_inplace(bv1, bv2);
    assert(bv1.cardinality() == 2);
    assert(bv1.get(1) == true);
    assert(bv1.get(1000) == true);
}


void test() {
    test_basic_operations();
    test_copy_constructor();
    test_move_constructor();
    test_move_operator();
    test_visit();

    test_and__case_1();
    test_and__case_2();
    test_and__case_3();

    test_and_inplace__case_1();
    test_and_inplace__case_2();
    test_and_inplace__case_3();
}



int main() {
    test();

    puts("All OK");
    return EXIT_SUCCESS;
}
