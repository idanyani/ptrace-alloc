#include <regex>
#include <fstream>
#include <iostream>
#include "base_test_first_fit_allocator.h"

using namespace std;

int main(int argc, char**argv) {
    assert(1);

    if (argc == 1) {
        printf("**********   %s was executed with no trace file   **********\n", __FILE__);
        return 0;
    }
    if (argc != 2) {
        printf("**********   %s FAILED: invalid command line!   **********\n", __FILE__);
        return 1;
    }

    FirstFitAllocator ffa;

    std::ifstream trace_file(argv[1]);
    std::smatch m;
    std::string line;
    int line_num = 0;
    bool is_initialized = false;

    while (std::getline(trace_file, line)) {
        line_num++;
        // Parse trace file.
        // Valid lines in trace file are such:
        // Initialize - len: <len> , start: <address> , end: <address>
        //  Allocate - size: 6966743040 --> 0x2b58c0000000
        //  Free - start: 0x2b5bfe903000 , size: 2147487744
        if (std::regex_search(line, m,
                              std::regex("(?:Initialize - len: )(.*)(?: , start: )(.*)(?: , end: )(.*)"))) {
            unsigned int len = stoi(m[1]);
            void *start = (void *) std::stoul(m[2], nullptr, 16);
            void *end = (void *) std::stoul(m[3], nullptr, 16);
            ffa.Initialize(len, start, end);
            is_initialized = true;
        }
        if (std::regex_search(line, m,
                              std::regex("(?:Allocate - size: )(.*)(?: --> )(.*)"))) {
            size_t size = stoul(m[1]);
            void *addr = (void *) std::stoul(m[2], nullptr, 16);
            void *res = ffa.Allocate(size);
            if (addr != res) {
                cerr << "Allocate returned unexpected address (at line: " 
                    << line_num << ")" << endl
                    << "\taddress : \t" << res << endl
                    << "\texpected : \t" << addr << endl;
            }
        }
        if (std::regex_search(line, m,
                              std::regex("(?:Free - start: )(.*)(?: , size: )(.*)"))) {
            void *start = (void *) std::stoul(m[1], nullptr, 16);
            size_t size = stoul(m[2]);
            int res = ffa.Free(start, size);
            if (res != 0) {
                cerr << "Free returned non-zero status code: " << res 
                    << " (at line: " << line_num << ")" << endl;
            }
        }
        if (std::regex_search(line, m,
                              std::regex("(?:break)"))) {
            cout << "Press any key to continue..." << endl;
            getchar();
        }
        if (is_initialized && !ffa.IsValidDataStructure()) {
            cerr << "Validate failed at line: " << line_num << endl;
            return -1;
        }
    }

    printf("**********   %s has succeeded!   **********\n", __FILE__);
    return 0;

}



