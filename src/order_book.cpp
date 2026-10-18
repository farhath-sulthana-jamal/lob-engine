#include "lob/order_book.hpp"
#include "lob/types.hpp"
#include <algorithm>
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
        if (order.qty <= Qty{0}) {
            return {};
        }
         std::vector<Trade> trades;
        if (order.side == Side::Buy) {
            while (order.qty > Qty{0} && !asks.empty() && asks.begin()->first <= order.price) {
                Price price = asks.begin()->first;
                auto& orders = asks.at(price);
                auto& best_ask = orders.front();
                auto qty_to_trade = Qty{std::min(order.qty.getValue(), best_ask.qty.getValue())};
                trades.push_back(Trade {order.id, best_ask.id, best_ask.price, qty_to_trade});
                order.qty -= qty_to_trade;
                best_ask.qty -= qty_to_trade;
                if (best_ask.qty == Qty{0}) {
                    orders.pop_front();
                    if (orders.empty()) {
                        asks.erase(price);
                    }
                }
            }
            if (order.qty > Qty{0}) {
                bids[order.price].push_back(order);
            }
        } else {
            while (order.qty > Qty{0} && !bids.empty() && bids.begin()->first >= order.price) {
                Price price = bids.begin()->first;
                auto& orders = bids.at(price);
                auto& best_bid = orders.front();
                auto qty_to_trade = Qty{std::min(order.qty.getValue(), best_bid.qty.getValue())};
                trades.push_back(Trade {best_bid.id, order.id, best_bid.price, qty_to_trade});
                order.qty -= qty_to_trade;
                best_bid.qty -= qty_to_trade;
                if (best_bid.qty == Qty{0}) {
                    orders.pop_front();
                    if (orders.empty()) {
                        bids.erase(price);
                    }
                }
            }
            if (order.qty > Qty{0}) {
                asks[order.price].push_back(order);
            }
        }
        return trades;
    }

} // namespace lob