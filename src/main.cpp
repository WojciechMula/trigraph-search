#include <vector>
#include <string>
#include <cassert>
#include <optional>
#include <fstream>

#include "Builder.h"
#include "DB.h"
#include "NaiveDB.h"
#include "IndexedDB.h"

#include "bitvector.h"

#include <sys/time.h>

unsigned gettime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (tv.tv_sec * 1000000 + tv.tv_usec)/1000;
}


Collection load(const char* path) {
    Collection coll;

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

    return coll;
}


void test_performance(const char* name, const DB& db, const Collection& words) {

    printf("searching in %s... ", name); fflush(stdout);
    volatile int k = 10;
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


void compare(const DB& db1, const DB& db2, Collection& words) {

    for (const auto& word: words) {
        const auto db1_res = db1.matches(word);
        const auto db2_res = db2.matches(word);
        if (db1_res != db2_res) {
            printf("mismatch for '%s'\n", word.c_str());
        }
    }
}


int main() {

    Collection input;
    Collection words;

    {
        printf("loading... "); fflush(stdout);
        const auto t1 = gettime();
        input = load("data.txt"); 
        const auto t2 = gettime();
        printf("%lu rows, %d ms\n", input.size(), t2 - t1);
    }

    {
        printf("loading... "); fflush(stdout);
        const auto t1 = gettime();
        words = load("words.txt"); 
        const auto t2 = gettime();
        printf("%lu rows, %d ms\n", words.size(), t2 - t1);
    }

    const NaiveDB naive_db(input);
    const auto indexed_db = create<bitvector>(input);

    //test_performance("NaiveDB", naive_db, words);
    test_performance("IndexedDB<bitvector>", indexed_db, words);
    //compare(naive_db, indexed_db, words);
}
