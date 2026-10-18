#include <iostream>
#include "lob/types.hpp"

static_assert(lob::Price{100} < lob::Price{101});
static_assert(lob::Price{100} == lob::Price{100});
static_assert(lob::Qty{5} == lob::Qty{5});
static_assert(lob::Qty{7} - lob::Qty{5} == lob::Qty{2});

int main() {
    std::cout << "Size of Price: " << sizeof(lob::Price) << '\n';
    std::cout << "Size of Qty: " << sizeof(lob::Qty) << '\n';
    std::cout << "Size of Order: " << sizeof(lob::Order) << '\n';
    std::cout << "Size of Trade: " << sizeof(lob::Trade) << '\n';
    return 0;
}