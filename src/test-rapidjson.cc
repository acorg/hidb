#include <iostream>
#include <sstream>
#include <string>
#include <random>
#include <ctime>
#include <chrono>

#include "rapidjson/document.h"

#include "chart-rj.hh"
#include "chart.hh"

// ----------------------------------------------------------------------

class Timeit
{
 public:
    inline Timeit(std::string msg) : message(msg), start(std::chrono::steady_clock::now()) {}
    inline ~Timeit() { std::cout << message << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << std::endl; }
 private:
    std::string message;
    decltype(std::chrono::steady_clock::now()) start;
};

// ----------------------------------------------------------------------

void test_big_object();
void test_rj(int num_tests, std::string source);
void test_sax(int num_tests, std::string source);

// 0.00023 sec per /tmp/e.ace

int main()
{
    int N = 10000;
    std::string source = "/tmp/a.ace";
    test_rj(N, source);
    test_sax(N, source);

    return 0;
}

// ----------------------------------------------------------------------

void test_rj(int num_tests, std::string source)
{
    Timeit timeit("import " + std::to_string(num_tests) + " charts via RJ: ");

    for (int i = 0; i < num_tests; ++i) {
        RJ::Chart* chart = RJ::import_chart(source);
        delete chart;
    }
}

// ----------------------------------------------------------------------

void test_sax(int num_tests, std::string source)
{
    Timeit timeit("import " + std::to_string(num_tests) + " charts via SAX: ");

    for (int i = 0; i < num_tests; ++i) {
        Chart* chart = import_chart(source);
        delete chart;
    }
}

// ----------------------------------------------------------------------

void test_big_object()
{
    std::ostringstream data;
    {
        Timeit timeit("generated: ");
        data << '{';
        std::random_device rd;
        std::uniform_int_distribution<int> dis('a', 'z');
        std::string src(8, ' ');
        for (int i = 0; i < 100000; ++i) {
            std::generate(src.begin(), src.end(), [&] { return static_cast<char>(dis(rd)); });
            data << '"' << src << "\":" << i << ',';
        }
        data << "\"Z\": -1}";
    }

    rapidjson::Document d;
    {
        Timeit timeit("parsed: ");
        d.Parse(data.str().c_str());
    }

    for (auto it = d.MemberBegin(); it != d.MemberEnd(); ++it) {
        std::cout << it->name.GetString() << " " << it->value.GetInt() << std::endl;
        if ((it - d.MemberBegin()) > 10)
            break;
    }


} // test_big_object

// ----------------------------------------------------------------------
