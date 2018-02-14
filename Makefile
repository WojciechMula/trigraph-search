.PHONY: clean
.PHONY: help

FLAGS=$(CXXFLAGS) -Wall -Wextra -pedantic -std=c++17 -O3 -g -Iinclude -Iroaring

HEADERS=include/*.h
SRC=
ROARING_ALL=roaring/roaring.h roaring/roaring.hh roaring/roaring.c 

URL=http://download.maxmind.com/download/worldcities/worldcitiespop.txt.gz
SHUF=./predictable_shuf.py

DATA_FILE_ALL=data-all.txt
DATA_FILE=data.txt
DATA_FILE_LIMIT=200000 # of around 3.2 million

QUERY_FILE_ALL=enwords-all.txt
QUERY_FILE_LIMIT=10000 # of around 90 thousand
QUERY_FILE=words.txt

REPEAT_COUNT=3

help:
	@echo "Targets"
	@echo "* run_unittests - unit tests"
	@echo "* run_perftest  - performance tests"

run_perftest: perftest $(DATA_FILE) $(QUERY_FILE)
	./$< $(DATA_FILE) $(QUERY_FILE) $(REPEAT_COUNT)

perftest: src/main.cpp $(HEADERS) $(SRC) $(ROARING_ALL)
	$(CXX) $(FLAGS) src/main.cpp $(SRC) -o $@

run_unittests: unittests
	./$<

unittests: $(HEADERS) tests/*.cpp
	$(CXX) $(FLAGS) tests/bitvector_sparse_tests.cpp -o $@

worldcitiespop.txt.gz:
	wget $(URL)

$(DATA_FILE_ALL): worldcitiespop.txt.gz
	zcat $^ > $@

$(QUERY_FILE_ALL):
	aspell dump master en | aspell expand | tr " " "\n" | grep -v "'" > $@

TMPFILE=/tmp/trigraph.tmp

$(DATA_FILE): $(DATA_FILE_ALL)
	$(SHUF) $^ $(DATA_FILE_LIMIT) > $(TMPFILE)
	mv $(TMPFILE) $@

$(QUERY_FILE): $(QUERY_FILE_ALL)
	$(SHUF) $^ $(QUERY_FILE_LIMIT) > $(TMPFILE)
	mv $(TMPFILE) $@

$(ROARING_ALL):
	cd roaring && ./amalgamation.sh

clean:
	$(RM) perftest unittests
