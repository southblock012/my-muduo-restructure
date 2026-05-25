#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

class Timestamp
{
private:
    int64_t microSecondsSinceEpoch_;
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    static Timestamp Now();
    std::string ToString() const;

    ~Timestamp()=default;
};
