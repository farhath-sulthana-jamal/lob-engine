#include <iostream>
#include "lob/types.hpp"

int main() {
    std::cout << "Size of Price: " << sizeof(lob::Price) << '\n';
    std::cout << "Size of Qty: " << sizeof(lob::Qty) << '\n';
    return 0;
}