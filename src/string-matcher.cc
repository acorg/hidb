#include <iostream>
#include <vector>
#include <numeric>

#include "string-matcher.hh"

// ----------------------------------------------------------------------

// (len1 - 2) * level_k + len1
//  (len2 - 2) * level_k + len2

// len1 * level_k - 2 * level_k + len1 + len2 * level_k - 2 * level_k + len2
// (len1 + len2) * level_k - 4*level_k +(len1+len2)

typedef std::string::const_iterator SP;
typedef decltype(SP()-SP()) SP_LEN;

inline size_t level_for_len(size_t len, size_t level_k)
{
    return len > 1 ? (len - 2) * level_k + len : 0;
}

inline SP_LEN match_len_at(SP master_b, SP master_e, SP input_b, SP input_e)
{
    const auto beg = master_b;
    while (master_b != master_e && input_b != input_e && *master_b == *input_b) {
        ++master_b;
        ++input_b;
    }
    return master_b - beg;
}

inline SP_LEN substring_match(SP master_b, SP master_e, SP input_b, SP input_e, size_t level_k)
{
    SP_LEN max_len = 0;
    while (input_b != input_e) {
        auto found = std::find(master_b, master_e, *input_b);
          //std::cout << *input_b << " " << (found - master_b) << std::endl;
        while (found != master_e) {
            auto len = match_len_at(found, master_e, input_b, input_e);
            len += substring_match(found + len, master_e, input_b + len, input_e, level_k);
            if (len > max_len)
                max_len = len;
            found  = std::find(found + 1, master_e, *input_b);
        }
        ++input_b;
    }
      // std::cout << "ML: " << max_len << std::endl;
    return max_len;
}

    // if (input_b != input_e) {
    //     auto found = std::find(master_b, master_e, *input_b);
    //     auto len = match_len_at(found, master_e, input_b, input_e);
    //     if (len > 0) {
    //         const size_t level_here = level_for_len(len, level_k);

    //     }
    //     else {
    //     }

    //     for (auto found = std::find(master_b, master_e, *input_b); found != master_e; ) {
    //         size_t lev = level_at(found, master_e, input_b, input_e, level_k);
    //         if (lev > level)
    //             level = lev;
    //     }
    //     if (found != master_e) {
    //         return level_at(found, master_e, input_b, input_e, level_k);
    //     }
    //     else {
    //         return substring_match(master_b, master_e, input_b + 1, input_e, level_k);
    //     }
    // }
    // else {
    //     return 0;
    // }


// ----------------------------------------------------------------------

size_t string_match(std::string master, std::string input)
{
    const size_t level = static_cast<size_t>(substring_match(master.begin(), master.end(), input.begin(), input.end(), input.size()));
      // std::cout << master << " -- " << level << std::endl;
    return level;

} // string_match

// ----------------------------------------------------------------------

// inline size_t substring_match(SP master_b, SP master_e, SP input_b, SP input_e, size_t level_k)
// {
//     size_t level = 0;
//     while (master_b != master_e && input_b != input_e) {
//         auto found = std::find(master_b, master_e, *input_b);
//         if (found != master_e) {
//             ++input_b;
//             for (master_b = found + 1; master_b != master_e && input_b != input_e && *master_b == *input_b; ++master_b, ++input_b)
//                 ;
//             const size_t len = static_cast<size_t>(master_b - found);
//             if (len > 1)
//                 level += (len - 2) * level_k + len;
//         }
//         else {
//             ++input_b;
//         }
//     }
//     return level;
// }

// // ----------------------------------------------------------------------
// // ----------------------------------------------------------------------

// inline void substring_match(SP master_b, SP master_e, SP input_b, SP input_e, std::vector<size_t>& match_len)
// {
//     while (master_b != master_e && input_b != input_e) {
//         auto found = std::find(master_b, master_e, *input_b);
//         if (found != master_e) {
//             ++input_b;
//             for (master_b = found + 1; master_b != master_e && input_b != input_e && *master_b == *input_b; ++master_b, ++input_b)
//                 ;
//             const size_t len = static_cast<size_t>(master_b - found);
//             if (len > 1)
//                 match_len.push_back(len);
//         }
//         else {
//             ++input_b;
//         }
//     }
// }

// // ----------------------------------------------------------------------

// size_t string_match(std::string master, std::string input)
// {
//     std::vector<size_t> match_len;
//     substring_match(master.begin(), master.end(), input.begin(), input.end(), match_len);
//     const size_t mult = input.size();
//     const size_t level = std::accumulate(match_len.begin(), match_len.end(), 0U, [mult](auto a, auto b) { return a + (b - 2) * mult + b; });
//     // if (level > mult) {
//     //     std::cout << master << " [";
//     //     std::copy(match_len.begin(), match_len.end(), std::ostream_iterator<decltype(match_len)::value_type>(std::cout, " "));
//     //     std::cout << "] " << level << std::endl;
//     // }
//     return level;

// } // string_match

// // ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
