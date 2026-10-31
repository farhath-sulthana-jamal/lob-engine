#pragma once

#include "lob/types.hpp"
#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

namespace lob {
    class OrderBook {
        struct Locator {
            Side side;
            Price price;
            std::list<Order>::iterator position;
        };

        std::map<Price, std::list<Order>, std::greater<Price>> bids;
        std::map<Price, std::list<Order>> asks;
        std::unordered_map<OrderId, Locator> index;

        public:
        std::optional<Price> bestBid() const noexcept;
        std::optional<Price> bestAsk() const noexcept;
        std::vector<Trade> add(Order order);
        bool cancel(OrderId id);
        std::optional<Qty> restingQty(OrderId id) const;
        std::size_t orderCount() const noexcept;
    };
}  // namespace lob
