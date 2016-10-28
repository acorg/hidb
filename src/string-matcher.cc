#include <iostream>
#include <vector>
#include <numeric>

#include "string-matcher.hh"

// ----------------------------------------------------------------------

typedef std::string::const_iterator SP;

inline void substring_match(SP master_b, SP master_e, SP input_b, SP input_e, std::vector<size_t>& match_len)
{
    while (master_b != master_e && input_b != input_e) {
        auto found = std::find(master_b, master_e, *input_b);
        if (found != master_e) {
            ++input_b;
            for (master_b = found + 1; master_b != master_e && input_b != input_e && *master_b == *input_b; ++master_b, ++input_b)
                ;
            const size_t len = static_cast<size_t>(master_b - found);
            if (len > 1)
                match_len.push_back(len);
        }
        else {
            ++input_b;
        }
    }
}

// ----------------------------------------------------------------------

size_t string_match(std::string master, std::string input)
{
    std::vector<size_t> match_len;
    substring_match(master.begin(), master.end(), input.begin(), input.end(), match_len);
    const size_t mult = input.size();
    const size_t level = std::accumulate(match_len.begin(), match_len.end(), 0U, [mult](auto a, auto b) { return a + (b - 2) * mult + b; });
    // if (level > mult) {
    //     std::cout << master << " [";
    //     std::copy(match_len.begin(), match_len.end(), std::ostream_iterator<decltype(match_len)::value_type>(std::cout, " "));
    //     std::cout << "] " << level << std::endl;
    // }
    return level;

} // string_match

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
