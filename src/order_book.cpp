#include "lob/order_book.hpp"
#include "lob/types.hpp"
#include <algorithm>
#include <cassert>
#include <iterator>
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

    std::optional<Qty> OrderBook::restingQty(OrderId id) const {
        auto entry = index.find(id);
        if (entry == index.end()) {
            return std::nullopt;
        }
        return entry->second.position->qty;
    }

    std::size_t OrderBook::orderCount() const noexcept {
        return index.size();
    }

    std::vector<Trade> OrderBook::add(Order order) {
        assert(order.qty > Qty{0});
        assert(!index.contains(order.id));

        std::vector<Trade> trades;
        if (order.side == Side::Buy) {
            while (order.qty > Qty{0} && !asks.empty() && asks.begin()->first <= order.price) {
                auto level = asks.begin();
                auto& orders = level->second;
                auto& best_ask = orders.front();
                auto qty_to_trade = std::min(order.qty, best_ask.qty);
                trades.push_back(Trade {order.id, best_ask.id, best_ask.price, qty_to_trade});
                order.qty -= qty_to_trade;
                best_ask.qty -= qty_to_trade;
                if (best_ask.qty == Qty{0}) {
                    index.erase(best_ask.id);
                    orders.pop_front();
                    if (orders.empty()) {
                        asks.erase(level);
                    }
                }
            }
            if (order.qty > Qty{0}) {
                auto& orders = bids[order.price];
                orders.push_back(order);
                index.emplace(order.id, Locator{Side::Buy, order.price, std::prev(orders.end())});
            }
        } else {
            while (order.qty > Qty{0} && !bids.empty() && bids.begin()->first >= order.price) {
                auto level = bids.begin();
                auto& orders = level->second;
                auto& best_bid = orders.front();
                auto qty_to_trade = std::min(order.qty, best_bid.qty);
                trades.push_back(Trade {best_bid.id, order.id, best_bid.price, qty_to_trade});
                order.qty -= qty_to_trade;
                best_bid.qty -= qty_to_trade;
                if (best_bid.qty == Qty{0}) {
                    index.erase(best_bid.id);
                    orders.pop_front();
                    if (orders.empty()) {
                        bids.erase(level);
                    }
                }
            }
            if (order.qty > Qty{0}) {
                auto& orders = asks[order.price];
                orders.push_back(order);
                index.emplace(order.id, Locator{Side::Sell, order.price, std::prev(orders.end())});
            }
        }
        return trades;
    }

    bool OrderBook::cancel(OrderId id) {
        auto entry = index.find(id);
        if (entry == index.end()) {
            return false;
        }
        auto& locator = entry->second;
        if (locator.side == Side::Buy) {
            auto level = bids.find(locator.price);
            assert(level != bids.end());
            auto& orders = level->second;
            orders.erase(locator.position);
            if (orders.empty()) {
                bids.erase(level);
            }
        } else {
            auto level = asks.find(locator.price);
            assert(level != asks.end());
            auto& orders = level->second;
            orders.erase(locator.position);
            if (orders.empty()) {
                asks.erase(level);
            }
        }
        index.erase(entry);
        return true;
    }

} // namespace lob
