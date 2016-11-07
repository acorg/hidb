#pragma once

#include <string>

// ----------------------------------------------------------------------

namespace string_match
{
    namespace _internal {
        typedef std::string::const_iterator iter_t;
        typedef decltype(iter_t()-iter_t()) score_t;

        inline score_t match_len_at(iter_t master_b, iter_t master_e, iter_t input_b, iter_t input_e)
        {
            const auto beg = master_b;
            while (master_b != master_e && input_b != input_e && *master_b == *input_b) {
                ++master_b;
                ++input_b;
            }
            return master_b - beg;
        }

        inline score_t substring_match(iter_t master_b, iter_t master_e, iter_t input_b, iter_t input_e, size_t level_k = 0)
        {
            score_t max_score = 0;
            while (input_b != input_e) {
                auto found = std::find(master_b, master_e, *input_b);
                while (found != master_e) {
                    const score_t len = match_len_at(found, master_e, input_b, input_e);
                    const score_t score = len * len + substring_match(found + len, master_e, input_b + len, input_e, level_k + 1);
                    if (score > max_score)
                        max_score = score;
                    found  = std::find(found + 1, master_e, *input_b);
                }
                ++input_b;
            }
            return max_score;
        }
    }

      // ----------------------------------------------------------------------

    typedef _internal::score_t score_t;

    inline score_t match(std::string master, std::string input)
    {
        return _internal::substring_match(master.begin(), master.end(), input.begin(), input.end(), input.size());

    } // string_match
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
