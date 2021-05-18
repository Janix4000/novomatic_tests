#include <string>
#include <vector>
#include <tuple>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <iostream>
// #include <format>

using CharCount = std::pair<char, size_t>;
using CharCountVector = std::vector<CharCount>;

CharCountVector get_number_of_cons_substrings(const std::string &text)
{
    CharCountVector counts;
    char last_char = text[0];
    int n_cons = 1;
    for (size_t i = 1; i < text.size() + 1; i++)
    {
        const char curr_char = text[i];
        if (curr_char != last_char)
        {
            counts.emplace_back(last_char, n_cons);
            last_char = curr_char;
            n_cons = 1;
        }
        else
        {
            n_cons++;
        }
    }
    return counts;
}

template <class T>
size_t numDigits(T number)
{
    size_t digits = 0;
    while (number)
    {
        number /= 10;
        digits++;
    }
    return digits;
}

size_t get_encrypt_size(const CharCountVector &char_counts)
{
    return std::accumulate(char_counts.begin(), char_counts.end(), 0lu,
                           [](const size_t last, const CharCount &pair) {
                               return last + 1 + numDigits(pair.second);
                           });
}

std::string encrypt_string(const std::string &to_encrypt)
{
    if (to_encrypt.empty())
    {
        return "";
    }
    const auto char_counts = get_number_of_cons_substrings(to_encrypt);
    std::string encrypted;
    encrypted.reserve(get_encrypt_size(char_counts));
    for (const auto &[character, count] : char_counts)
    {
        encrypted += std::to_string(count) + character;
    }
    return encrypted;
}

size_t get_decrypt_size(const CharCountVector &char_counts)
{
    return std::accumulate(char_counts.begin(), char_counts.end(), 0lu,
                           [](const size_t last, const CharCount &pair) {
                               return last + pair.second;
                           });
}

std::string decrypt_string(const std::string &to_decrypt)
{
    if (to_decrypt.empty())
    {
        return "";
    }
    std::istringstream ss(to_decrypt);
    CharCountVector char_counts;
    while (true)
    {
        size_t count = 0;
        char character = '\0';
        ss >> count >> character;
        if (ss.eof())
        {
            break;
        }
        char_counts.emplace_back(character, count);
    }
    std::string decrypted;
    decrypted.resize(get_decrypt_size(char_counts), '\0');
    auto it = decrypted.begin();
    for (const auto &[character, count] : char_counts)
    {
        std::fill(it, it + count, character);
        it += count;
    }
    return decrypted;
}