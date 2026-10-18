#include <gtest/gtest.h>
#include <optional>
#include "lob/order_book.hpp"
#include "lob/types.hpp"

using namespace lob;

TEST(OrderBookTest, BestBidIsHighestPrice) {
    OrderBook book;
    book.add(Order{OrderId{1}, Price{99}, Qty{10}, Side::Buy});
    book.add(Order{OrderId{2}, Price{100}, Qty{10}, Side::Buy});
    book.add(Order{OrderId{3}, Price{98}, Qty{10}, Side::Buy});
    auto bestBid = book.bestBid();
    ASSERT_TRUE(bestBid.has_value());
    EXPECT_EQ(*bestBid, Price{100});
}

TEST(OrderBookTest, BestAskIsLowestPrice) {
    OrderBook book;
    book.add(Order{OrderId{1}, Price{103}, Qty{10}, Side::Sell});
    book.add(Order{OrderId{2}, Price{101}, Qty{10}, Side::Sell});
    book.add(Order{OrderId{3}, Price{102}, Qty{10}, Side::Sell});
    auto bestAsk = book.bestAsk();
    ASSERT_TRUE(bestAsk.has_value());
    EXPECT_EQ(*bestAsk, Price{101});
}

TEST(OrderBookTest, RestingOrderProducesNoTrades) {
    OrderBook book;
    auto trades = book.add(Order{OrderId{1}, Price{103}, Qty{10}, Side::Sell});
    EXPECT_TRUE(trades.empty());
}

TEST(OrderBookTest, EmptyBookHasNoBestPrices) {
    OrderBook book;
    auto bestAsk = book.bestAsk();
    EXPECT_FALSE(bestAsk.has_value());
    auto bestBid = book.bestBid();
    EXPECT_FALSE(bestBid.has_value());
}