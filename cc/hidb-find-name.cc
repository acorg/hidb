#include <cstdlib>
#include <cmath>

#include "acmacs-base/timeit.hh"
#include "hidb.hh"
using namespace hidb;
using namespace std::string_literals;

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        if (argc < 3)
            throw std::runtime_error("Usage: "s + argv[0] + " <hidb.json.xz> <name> ...");

        HiDb hidb;
        hidb.importFrom(argv[1], report_time::Yes);
        hidb.importLocDb(std::getenv("HOME") + "/AD/data/locationdb.json.xz"s, report_time::Yes);

        for (auto arg = 2; arg < argc; ++arg) {
            Timeit timeit("looking: ");
            const auto look_for = string::upper(argv[arg]);
            const auto results = hidb.find_antigens_with_score(look_for);
            if (!results.empty()) {
                const auto num_digits = static_cast<int>(std::log10(results.size())) + 1;
                size_t result_no = 1;
                for (const auto& result: results) {
                    std::cout << std::setw(num_digits) << result_no << ' ' << result.second << ' ' << result.first->full_name() << '\n';
                    ++result_no;
                }
                if ((arg + 1) < argc)
                    std::cout << '\n';
            }
        }

        return 0;
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        return 1;
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
