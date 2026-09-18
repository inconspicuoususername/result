module;
#include <iostream>
#include <string_view>
#include <print>
#include "result/macros.h"
export module main;

import result;

using result::Result;

Result<int> parse(std::string_view str) {
    if (str.empty())
        return result::fail("empty input");
    return std::stoi(std::string(str));
}

Result<int> doubled(std::string_view s) {
    // Unwrap or return the error with extra context attached
    const int num = TRY(parse(s), "while parsing count");
    return num * 2;
}

Result<std::string> excited_doubled(std::string_view s) {
    const int num = TRY(doubled(s), "while doubling input");
    return  std::format("{}!", num);
}

extern "C++" int main() {
    if (auto result = excited_doubled("23"); result)
        std::println("Got: {}", *result);


    if (auto result = excited_doubled(""); !result)
        std::println(std::cerr, "{}", result.error().report());
        // alternatively: std::cerr << result.error().report() << "\n";
}