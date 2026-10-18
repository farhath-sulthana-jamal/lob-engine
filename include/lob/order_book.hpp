#pragma once

#include "lob/types.hpp"
#include <optional>
#include <functional>
#include <vector>
#include <list>
#include <map>

namespace lob {
    class OrderBook {
        std::map<Price, std::list<Order>, std::greater<Price>> bids;
        std::map<Price, std::list<Order>> asks;

        public:
        std::optional<Price> bestBid() const noexcept;
        std::optional<Price> bestAsk() const noexcept;
        std::vector<Trade> add(Order order);
    };
}  // namespace lob