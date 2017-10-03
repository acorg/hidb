#include <cstdlib>
#include <cmath>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/string.hh"
#include "hidb.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

constexpr const char* sUsage = " [options] <virus-type: B, H1, H3> <name> ...\n";

int main(int argc, char* const argv[])
{
    try {
        argc_argv args(argc, argv, {
                {"--db-dir", ""},
                {"-v", false},
                {"--verbose", false},
                {"-h", false},
                {"--help", false},
            });
        if (args["-h"] || args["--help"] || args.number_of_arguments() < 2) {
            throw std::runtime_error("Usage: "s + args.program() + sUsage + args.usage_options());
        }
          //const bool verbose = args["-v"] || args["--verbose"];
        hidb::setup(args["--db-dir"]);

        const auto& hidb = hidb::get(string::upper(args[0]), report_time::Yes);

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
