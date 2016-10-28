#include <iostream>

#include "string-matcher.hh"

// ----------------------------------------------------------------------

inline std::pair<size_t, size_t> string_match(std::string master, size_t master_pos, std::string input, size_t input_pos, size_t input_size)
{
    size_t level = 0;
    size_t pos = master.find(input.substr(input_pos, input_size), master_pos);
    std::cout << master.substr(master_pos) << " -- " << input.substr(input_pos) << " -- " << pos << std::endl;
    if (pos != std::string::npos)
        ++level;
    return std::make_pair(level, pos);
}

// ----------------------------------------------------------------------

size_t string_match(std::string master, std::string input)
{
    size_t level = 0;
    auto lp = string_match(master, 0, input, 0, 2);
    if (lp.first) {
        level += lp.first;
        lp = string_match(master, lp.second + 2, input, 2, 2);
        if (lp.first) {
            level += lp.first;
        }
        std::cout << master << " " << lp.second << std::endl;
    }
    if (level > 1)
        std::cout << master << std::endl;
    return level;

} // string_match

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
