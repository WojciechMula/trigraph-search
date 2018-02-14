#include <vector>
#include <string>
#include <optional>
#include <limits>
#include <fstream>

#include <cassert>

#include <roaring.c>

#include "Builder.h"
#include "DB.h"
#include "NaiveDB.h"
#include "IndexedDBAll.h"
#include "IndexedDBSmallest.h"

#include "bitvector_tracking.h"
#include "bitvector_naive.h"
#include "bitvector_sparse.h"
#include "vector_facade.h"
#include "roaring_facade.h"

#include <sys/time.h>

unsigned gettime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (tv.tv_sec * 1000000 + tv.tv_usec)/1000;
}


Collection load(const char* path) {
    Collection coll;

    printf("loading %s... ", path); fflush(stdout);
    const auto t1 = gettime();

    std::fstream f;
    f.exceptions(std::ifstream::badbit);
    f.open(path);

    while (!f.eof()) {
        std::string str;
        std::getline(f, str);
        if (!str.empty()) {
            coll.emplace_back(std::move(str));
        }
    }

    const auto t2 = gettime();
    printf("%lu rows, %d ms\n", coll.size(), t2 - t1);

    return coll;
}


void test_performance(const char* name, const DB& db, const Collection& words, int repeat_count) {

    printf("searching in %s (%d times)... ", name, repeat_count); fflush(stdout);
    volatile int k = repeat_count;
    volatile int result = 0;
    uint64_t best_time = std::numeric_limits<uint64_t>::max();
    while (k--) {
        const uint64_t t1 = gettime();
        for (const auto& word: words) {
            result += db.matches(word);
        }
        const uint64_t t2 = gettime();
        best_time = std::min(best_time, t2 - t1);
    }
    printf("%d match(es), %lu ms\n", result, best_time);
}


template <typename BITVECTOR>
IndexedDBAll<BITVECTOR> create(const Collection& collection) {

    Builder<BITVECTOR> builder(collection.size());

    {
        printf("building..."); fflush(stdout);
        const auto t1 = gettime();
        builder.add(collection);
        const auto t2 = gettime();
        printf("OK, %d ms\n", t2 - t1);
    }

    return {collection, builder.capture()};
}


template <typename BITVECTOR>
IndexedDBSmallest<BITVECTOR> create2(const Collection& collection) {

    Builder<BITVECTOR> builder(collection.size());

    {
        printf("building..."); fflush(stdout);
        const auto t1 = gettime();
        builder.add(collection);
        const auto t2 = gettime();
        printf("OK, %d ms\n", t2 - t1);
    }

    return {collection, builder.capture()};
}


void compare(const DB& db1, const DB& db2, Collection& words) {

    for (const auto& word: words) {
        const auto db1_res = db1.matches(word);
        const auto db2_res = db2.matches(word);
        if (db1_res != db2_res) {
            printf("mismatch for '%s'\n", word.c_str());
        }
    }
}


int main(int argc, char* argv[]) {

    if (argc < 2) {
        puts("Usage: test db-words.txt search-words.txt");
        return EXIT_FAILURE;
    }

    Collection input;
    Collection words;

    input = load(argv[1]);
    words = load(argv[2]);
    int repeat_count = 1;
    if (argc >= 4) {
        repeat_count = std::max(repeat_count, atoi(argv[3]));
    }

    {
        const auto db = create<roaring_facade>(input);
        test_performance("IndexedDBAll<roaring_facade>", db, words, repeat_count);
    }
    {
        const auto db = create<vector_facade>(input);
        test_performance("IndexedDBAll<vector_facade>", db, words, repeat_count);
    }
    {
        const auto db = create<bitvector_tracking>(input);
        test_performance("IndexedDBAll<bitvector_tracking>", db, words, repeat_count);
    }
    {
        const auto db = create<bitvector_naive>(input);
        test_performance("IndexedDBAll<bitvector_naive>", db, words, repeat_count);
    }
    {
        const auto db = create<bitvector_sparse>(input);
        test_performance("IndexedDBAll<bitvector_sparse>", db, words, repeat_count);
    }

    {
        const auto db = create2<bitvector_tracking>(input);
        test_performance("IndexedDBSmallest<bitvector_tracking>", db, words, repeat_count);
    }
    {
        const auto db = create2<bitvector_naive>(input);
        test_performance("IndexedDBSmallest<bitvector_naive>", db, words, repeat_count);
    }
    {
        const auto db = create2<bitvector_sparse>(input);
        test_performance("IndexedDBSmallest<bitvector_sparse>", db, words, repeat_count);
    }

    return EXIT_SUCCESS;
}
