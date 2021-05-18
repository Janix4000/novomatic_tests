#include <iostream>
#include <string>
#include <random>
#include <functional>
#include <ctime>

#include "string_compr.hpp"

std::string generate_test_string(std::mt19937 &rng, size_t n_chars = 15lu, std::pair<int, int> count_range = {1, 20})
{
    auto char_gen = std::bind(std::uniform_int_distribution<char>('a', 'z'), rng);
    auto count_gen = std::bind(std::uniform_int_distribution<size_t>(count_range.first, count_range.second), rng);
    std::vector<size_t> counts(n_chars);
    std::generate(counts.begin(), counts.end(), count_gen);
    const size_t size = std::reduce(counts.begin(), counts.end());

    std::string result;
    result.resize(size);
    auto it = result.begin();
    for (auto count : counts)
    {
        const char c = char_gen();
        std::fill(it, it + count, c);
        it += count;
    }
    return result;
}

int main(int argc, char const *argv[])
{
    const size_t n_tests = 1000;
    std::mt19937 rng(std::time(nullptr));
    bool all_tests_passed = true;
    for (size_t i = 0; i < n_tests; i++)
    {
        const auto str = generate_test_string(rng);
        const auto encrypted = encrypt_string(str);
        const auto decrypted = decrypt_string(encrypted);
        if (str != decrypted)
        {
            all_tests_passed = false;
            break;
        }
    }
    if (all_tests_passed)
    {
        std::cout << "All tests passed\n";
    }
    else
    {
        std::cout << "Tests failed\n";
    }

    return 0;
}
