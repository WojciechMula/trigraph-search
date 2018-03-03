#include <vector>
#include <string>
#include <optional>
#include <limits>
#include <chrono>
#include <fstream>

#include <cassert>
#include <cstring>

#define ROARING

#ifdef ROARING
#   include <roaring.c>
#endif

#include "Builder.h"
#include "DB.h"
#include "NaiveDB.h"
#include "IndexedDB.h"
#include "combiner/all.h"

#include "bitvector_tracking.h"
#include "bitvector_naive.h"
#include "bitvector_sparse.h"
#include "vector_facade.h"
#include "deque_facade.h"
#include "list_facade.h"
#ifdef ROARING
#   include "roaring_facade.h"
#endif

using Clock = std::chrono::steady_clock;

auto elapsed(const Clock::time_point& t1, const Clock::time_point& t2) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
}


Collection load(const char* path) {
    Collection coll;

    printf("loading %s... ", path); fflush(stdout);
    const auto t1 = Clock::now();

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

    const auto t2 = Clock::now();
    printf("%lu rows, %lu ms\n", coll.size(), elapsed(t1, t2));

    return coll;
}


void test_performance(const DB& db, const Collection& words, int repeat_count) {

    printf("\tsearching (%d times)... ", repeat_count); fflush(stdout);
    volatile int k = repeat_count;
    volatile int result = 0;
    Clock::rep best_time = std::numeric_limits<Clock::rep>::max();
    while (k--) {
        const auto t1 = Clock::now();
        for (const auto& word: words) {
            result += db.matches(word);
        }
        const auto t2 = Clock::now();
        best_time = std::min(best_time, elapsed(t1, t2));
    }
    printf("%d match(es), %lu ms\n", result, best_time);
}


template <typename DBTYPE>
DBTYPE create(const Collection& collection) {

    Builder<typename DBTYPE::bitvector_type> builder(collection.size());

    {
        printf("\tbuilding..."); fflush(stdout);
        const auto t1 = Clock::now();
        builder.add(collection);
        const auto t2 = Clock::now();
        printf("%lu ms\n", elapsed(t1, t2));
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
        puts("Usage: test db-words.txt search-words.txt [repeats count [test name...]]");
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

    auto enabled = [&argc, &argv](const char* name) {
        if (argc <= 4) {
            return true; // no explicit options - all tests are enabled
        }
        for (int i = 3; i < argc; i++) {
            if (strstr(name, argv[i])) {
                return true;
            }
        }

        return false;

    };

#define TEST(KEYWORD, TYPE)                                 \
    if (enabled(KEYWORD)) {                                 \
        printf("%s\n", #TYPE);                              \
        const auto db = create<TYPE>(input);                \
        test_performance(db, words, repeat_count);          \
    }

    if (true) {
#ifdef ROARING
        using AndAll_Roaring = IndexedDB<AndAll<roaring_facade>>;
        TEST("roaring-all",  AndAll_Roaring);
#endif
        using AndAll_Vector = IndexedDB<AndAll<vector_facade>>;
        TEST("vector-all", AndAll_Vector);

        using AndAll_Deque = IndexedDB<AndAll<deque_facade>>;
        TEST("deque-all", AndAll_Deque);

        using AndAll_List = IndexedDB<AndAll<list_facade>>;
        TEST("deque-all", AndAll_List);

        using AndAll_Bitvector = IndexedDB<AndAll<bitvector_naive>>;
        TEST("naive-all", AndAll_Bitvector);

        using AndAll_BitvectorTracking = IndexedDB<AndAll<bitvector_tracking>>;
        TEST("tracking-all", AndAll_BitvectorTracking);

        using AndAll_BitvectorSparse = IndexedDB<AndAll<bitvector_sparse>>;
        TEST("sparse-all", AndAll_BitvectorSparse);
    }

    if (false) {
#ifdef ROARING
        using PickCheapest_Roaring = IndexedDB<PickCheapest<roaring_facade>>;
        TEST("roaring-cheapest",  PickCheapest_Roaring);
#endif
        using PickCheapest_Vector = IndexedDB<PickCheapest<vector_facade>>;
        TEST("vector-cheapest", PickCheapest_Vector);

        using PickCheapest_Bitvector = IndexedDB<PickCheapest<bitvector_naive>>;
        TEST("naive-cheapest", PickCheapest_Bitvector);

        using PickCheapest_BitvectorTracking = IndexedDB<PickCheapest<bitvector_tracking>>;
        TEST("tracking-cheapest", PickCheapest_BitvectorTracking);

        using PickCheapest_BitvectorSparse = IndexedDB<PickCheapest<bitvector_sparse>>;
        TEST("sparse-cheapest", PickCheapest_BitvectorSparse);
    }

    return EXIT_SUCCESS;
}
