#include <vector>
#include <string>
#include <cassert>
#include <optional>
#include <fstream>

#include "Builder.h"
#include "DB.h"
#include "NaiveDB.h"
#include "IndexedDB.h"
#include "IndexedDB2.h"

#include "bitvector_tracking.h"

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
        f >> str;
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
    const auto t1 = gettime();
    while (k--) {
        for (const auto& word: words) {
            result += db.matches(word);
        }
    }
    const auto t2 = gettime();
    printf("%d match(es), %d ms\n", result, t2 - t1);
}


template <typename BITVECTOR>
IndexedDB<BITVECTOR> create(const Collection& collection) {

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
IndexedDB2<BITVECTOR> create2(const Collection& collection) {

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

    const NaiveDB naive_db(input);
    const auto indexed_db = create<bitvector_tracking>(input);
    const auto indexed_db_2 = create2<bitvector_tracking>(input);

    //test_performance("NaiveDB", naive_db, words);
    test_performance("IndexedDB<bitvector_tracking>", indexed_db, words, repeat_count);
    test_performance("IndexedDB2<bitvector_tracking>", indexed_db_2, words, repeat_count);
    //compare(naive_db, indexed_db, words);

    return EXIT_SUCCESS;
}
