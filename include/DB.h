#pragma once

#include <string>

class DB {
public:
    virtual int matches(const std::string& word) const = 0;
};
