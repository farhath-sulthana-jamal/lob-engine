#include "lob/order_book.hpp"
#include "lob/types.hpp"
#include <optional>

namespace lob {

    std::optional<Price> OrderBook::bestBid() const noexcept {
        if (bids.empty()) {
            return std::nullopt;
        }
         return bids.begin()->first;
    }

    std::optional<Price> OrderBook::bestAsk() const noexcept {
        if (asks.empty()) {
            return std::nullopt;
        }
         return asks.begin()->first;
    }

    std::vector<Trade> OrderBook::add(Order order) {
        if (order.side == Side::Buy) {
            bids[order.price].push_back(order);
        } else {
            asks[order.price].push_back(order);
        }
        return {};
    }

} // namespace lob