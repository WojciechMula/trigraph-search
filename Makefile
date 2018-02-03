.PHONY: clean

FLAGS=$(CXXFLAGS) -Wall -Wextra -pedantic -std=c++17 -O3 -g -Iinclude

HEADERS=include/*.h
SRC=

test: src/main.cpp $(HEADERS) $(SRC)
	$(CXX) $(FLAGS) src/main.cpp $(SRC) -o $@

clean:
	$(RM) test
