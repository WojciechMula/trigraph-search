#pragma once

#include "types.h"

class NaiveDB : public DB {
protected:
    const Collection& rows;

public:
    NaiveDB(const Collection& rows_)
        : rows(rows_) {}

public:
    virtual int matches(const std::string& word) const override {
        int n = 0;
        int i = 0;
        for (const auto& row: rows) {
            if (row.find(word) != std::string::npos) {
                n += 1;
            }

            i += 1;
        }

        return n;
    }
};
