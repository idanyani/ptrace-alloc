#include <iostream>
#include <string.h>

#include "base_test_huge_page_backed_region.h"

using namespace std;

int main() {
    size_t size = 2ULL * GB;
    off_t start_1gb = 100 * MB;
    off_t end_1gb = start_1gb + 1 * GB;
    off_t start_2mb = end_1gb + 100 * MB;
    off_t end_2mb = start_2mb + 100 * MB;
    test_huge_page_backed_region(size, start_1gb, end_1gb, start_2mb, end_2mb);

    cout << "**********   "
         << __FILE__
         << " has succeeded!   **********" << endl;
}
