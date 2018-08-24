#include <iostream>
#include <string.h>

#include "base_test_huge_page_backed_region.h"

using namespace std;

int main() {
    test_huge_page_backed_region(2ULL * GB, 1 * GB, 2ULL * GB, 0, 0);

    cout << "**********   "
         << __FILE__
         << " has succeeded!   **********" << endl;
}
