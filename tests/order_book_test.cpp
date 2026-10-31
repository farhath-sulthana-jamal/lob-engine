#include <gtest/gtest.h>
#include <cstdint>
#include <optional>
#include <vector>
#include "lob/order_book.hpp"
#include "lob/types.hpp"

using namespace lob;

namespace {
    Order buy(std::uint64_t id, std::int64_t price, std::int64_t qty) {
        return Order{OrderId{id}, Price{price}, Qty{qty}, Side::Buy};
    }

    Order sell(std::uint64_t id, std::int64_t price, std::int64_t qty) {
        return Order{OrderId{id}, Price{price}, Qty{qty}, Side::Sell};
    }

    void expectTrade(const Trade& t, std::uint64_t buyId, std::uint64_t sellId,
                     std::int64_t price, std::int64_t qty) {
        EXPECT_EQ(t.buy_order_id, OrderId{buyId});
        EXPECT_EQ(t.sell_order_id, OrderId{sellId});
        EXPECT_EQ(t.price, Price{price});
        EXPECT_EQ(t.qty, Qty{qty});
    }
}  // namespace

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

TEST(OrderBookMatchingTest, NonCrossingOrdersBothRest) {
    OrderBook book;
    EXPECT_TRUE(book.add(buy(1, 99, 10)).empty());
    EXPECT_TRUE(book.add(sell(2, 101, 10)).empty());

    ASSERT_TRUE(book.bestBid().has_value());
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(*book.bestBid(), Price{99});
    EXPECT_EQ(*book.bestAsk(), Price{101});
}

TEST(OrderBookMatchingTest, ExactFillEmptiesBothSides) {
    OrderBook book;
    book.add(sell(1, 101, 5));
    auto trades = book.add(buy(2, 101, 5));

    ASSERT_EQ(trades.size(), 1u);
    expectTrade(trades[0], 2, 1, 101, 5);
    EXPECT_FALSE(book.bestAsk().has_value());
    EXPECT_FALSE(book.bestBid().has_value());
}

TEST(OrderBookMatchingTest, IncomingLargerRestsRemainder) {
    OrderBook book;
    book.add(sell(1, 101, 5));
    auto trades = book.add(buy(2, 101, 8));

    ASSERT_EQ(trades.size(), 1u);
    expectTrade(trades[0], 2, 1, 101, 5);
    EXPECT_FALSE(book.bestAsk().has_value());
    ASSERT_TRUE(book.bestBid().has_value());
    EXPECT_EQ(*book.bestBid(), Price{101});

    auto more = book.add(sell(3, 101, 10));
    ASSERT_EQ(more.size(), 1u);
    expectTrade(more[0], 2, 3, 101, 3);
}

TEST(OrderBookMatchingTest, RestingLargerKeepsReducedQty) {
    OrderBook book;
    book.add(sell(1, 101, 10));
    auto trades = book.add(buy(2, 101, 4));

    ASSERT_EQ(trades.size(), 1u);
    expectTrade(trades[0], 2, 1, 101, 4);
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(*book.bestAsk(), Price{101});
    EXPECT_FALSE(book.bestBid().has_value());

    auto more = book.add(buy(3, 101, 10));
    ASSERT_EQ(more.size(), 1u);
    expectTrade(more[0], 3, 1, 101, 6);
    ASSERT_TRUE(book.bestBid().has_value());
    EXPECT_EQ(*book.bestBid(), Price{101});
}

TEST(OrderBookMatchingTest, SweepsMultipleLevels) {
    OrderBook book;
    book.add(sell(1, 101, 5));
    book.add(sell(2, 102, 5));
    auto trades = book.add(buy(3, 102, 8));

    ASSERT_EQ(trades.size(), 2u);
    expectTrade(trades[0], 3, 1, 101, 5);
    expectTrade(trades[1], 3, 2, 102, 3);
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(*book.bestAsk(), Price{102});
    EXPECT_FALSE(book.bestBid().has_value());
}

TEST(OrderBookMatchingTest, SweepStopsAtLimitPrice) {
    OrderBook book;
    book.add(sell(1, 101, 5));
    book.add(sell(2, 103, 5));
    auto trades = book.add(buy(3, 102, 8));

    ASSERT_EQ(trades.size(), 1u);
    expectTrade(trades[0], 3, 1, 101, 5);
    ASSERT_TRUE(book.bestAsk().has_value());
    ASSERT_TRUE(book.bestBid().has_value());
    EXPECT_EQ(*book.bestAsk(), Price{103});
    EXPECT_EQ(*book.bestBid(), Price{102});
}

TEST(OrderBookMatchingTest, FifoWithinPriceLevel) {
    OrderBook book;
    book.add(sell(1, 101, 5));
    book.add(sell(2, 101, 5));
    auto trades = book.add(buy(3, 101, 1));

    ASSERT_EQ(trades.size(), 1u);
    expectTrade(trades[0], 3, 1, 101, 1);
}

TEST(OrderBookMatchingTest, IncomingSellTradesWithBids) {
    OrderBook book;
    book.add(buy(1, 99, 5));
    auto trades = book.add(sell(2, 99, 5));

    ASSERT_EQ(trades.size(), 1u);
    expectTrade(trades[0], 1, 2, 99, 5);
    EXPECT_FALSE(book.bestBid().has_value());
    EXPECT_FALSE(book.bestAsk().has_value());
}

TEST(OrderBookMatchingTest, BuyTradesAtRestingPrice) {
    OrderBook book;
    book.add(sell(1, 100, 5));
    auto trades = book.add(buy(2, 105, 5));

    ASSERT_EQ(trades.size(), 1u);
    expectTrade(trades[0], 2, 1, 100, 5);
}

TEST(OrderBookMatchingTest, SellTradesAtRestingPrice) {
    OrderBook book;
    book.add(buy(1, 100, 5));
    auto trades = book.add(sell(2, 95, 5));

    ASSERT_EQ(trades.size(), 1u);
    expectTrade(trades[0], 1, 2, 100, 5);
}

TEST(OrderBookMatchingTest, WorkedExampleFromNotes) {
    OrderBook book;
    book.add(sell(10, 101, 5));
    book.add(sell(11, 101, 3));
    book.add(sell(12, 102, 5));
    auto trades = book.add(buy(20, 102, 10));

    ASSERT_EQ(trades.size(), 3u);
    expectTrade(trades[0], 20, 10, 101, 5);
    expectTrade(trades[1], 20, 11, 101, 3);
    expectTrade(trades[2], 20, 12, 102, 2);
    EXPECT_FALSE(book.bestBid().has_value());
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(*book.bestAsk(), Price{102});

    auto more = book.add(buy(21, 102, 10));
    ASSERT_EQ(more.size(), 1u);
    expectTrade(more[0], 21, 12, 102, 3);
}

TEST(OrderIndexTest, RestingOrderReportsFullQty) {
    OrderBook book;
    book.add(buy(1, 99, 10));

    auto qty = book.restingQty(OrderId{1});
    ASSERT_TRUE(qty.has_value());
    EXPECT_EQ(*qty, Qty{10});
    EXPECT_EQ(book.orderCount(), 1u);
}

TEST(OrderIndexTest, UnknownIdHasNoQty) {
    OrderBook book;
    book.add(buy(1, 99, 10));

    EXPECT_FALSE(book.restingQty(OrderId{42}).has_value());
}

TEST(OrderIndexTest, FullyFilledOrdersAreRemoved) {
    OrderBook book;
    book.add(sell(1, 101, 5));
    book.add(buy(2, 101, 5));

    EXPECT_FALSE(book.restingQty(OrderId{1}).has_value());
    EXPECT_FALSE(book.restingQty(OrderId{2}).has_value());
    EXPECT_EQ(book.orderCount(), 0u);
}

TEST(OrderIndexTest, PartiallyFilledRestingOrderShowsRemainder) {
    OrderBook book;
    book.add(sell(1, 101, 10));
    book.add(buy(2, 101, 4));

    auto qty = book.restingQty(OrderId{1});
    ASSERT_TRUE(qty.has_value());
    EXPECT_EQ(*qty, Qty{6});
}

TEST(OrderIndexTest, IncomingRemainderIsIndexed) {
    OrderBook book;
    book.add(sell(1, 101, 5));
    book.add(buy(2, 101, 8));

    auto qty = book.restingQty(OrderId{2});
    ASSERT_TRUE(qty.has_value());
    EXPECT_EQ(*qty, Qty{3});
    EXPECT_EQ(book.orderCount(), 1u);
}

TEST(OrderIndexTest, OrderCountTracksRestsAndFills) {
    OrderBook book;
    book.add(buy(1, 99, 10));
    book.add(buy(2, 98, 10));
    book.add(sell(3, 101, 5));
    book.add(sell(4, 102, 5));
    EXPECT_EQ(book.orderCount(), 4u);

    book.add(sell(5, 99, 10));
    EXPECT_EQ(book.orderCount(), 3u);

    book.add(buy(6, 102, 7));
    EXPECT_EQ(book.orderCount(), 2u);
    EXPECT_FALSE(book.restingQty(OrderId{3}).has_value());
    ASSERT_TRUE(book.restingQty(OrderId{4}).has_value());
    EXPECT_EQ(*book.restingQty(OrderId{4}), Qty{3});
}

TEST(OrderIndexTest, LocatorSurvivesOtherBookChanges) {
    OrderBook book;
    book.add(sell(1, 105, 10));
    for (std::uint64_t id = 2; id <= 50; ++id) {
        book.add(sell(id, 101 + static_cast<std::int64_t>(id % 4), 1));
    }
    book.add(buy(100, 104, 30));

    auto qty = book.restingQty(OrderId{1});
    ASSERT_TRUE(qty.has_value());
    EXPECT_EQ(*qty, Qty{10});
}

TEST(OrderCancelTest, CancelRemovesRestingOrder) {
    OrderBook book;
    book.add(buy(1, 99, 10));

    EXPECT_TRUE(book.cancel(OrderId{1}));
    EXPECT_FALSE(book.restingQty(OrderId{1}).has_value());
    EXPECT_FALSE(book.bestBid().has_value());
    EXPECT_EQ(book.orderCount(), 0u);
}

TEST(OrderCancelTest, CancelUnknownIdReturnsFalse) {
    OrderBook book;
    book.add(buy(1, 99, 10));

    EXPECT_FALSE(book.cancel(OrderId{42}));
    EXPECT_EQ(book.orderCount(), 1u);
}

TEST(OrderCancelTest, CancelTwiceReturnsFalseSecondTime) {
    OrderBook book;
    book.add(sell(1, 101, 10));

    EXPECT_TRUE(book.cancel(OrderId{1}));
    EXPECT_FALSE(book.cancel(OrderId{1}));
}

TEST(OrderCancelTest, CancelFilledOrderReturnsFalse) {
    OrderBook book;
    book.add(sell(1, 101, 5));
    book.add(buy(2, 101, 5));

    EXPECT_FALSE(book.cancel(OrderId{1}));
    EXPECT_FALSE(book.cancel(OrderId{2}));
}

TEST(OrderCancelTest, CancelPartiallyFilledOrder) {
    OrderBook book;
    book.add(sell(1, 101, 10));
    book.add(buy(2, 101, 4));

    EXPECT_TRUE(book.cancel(OrderId{1}));
    EXPECT_FALSE(book.bestAsk().has_value());
    EXPECT_EQ(book.orderCount(), 0u);
}

TEST(OrderCancelTest, CancelMiddleOfQueueKeepsFifo) {
    OrderBook book;
    book.add(sell(1, 101, 5));
    book.add(sell(2, 101, 5));
    book.add(sell(3, 101, 5));

    EXPECT_TRUE(book.cancel(OrderId{2}));
    auto trades = book.add(buy(4, 101, 10));

    ASSERT_EQ(trades.size(), 2u);
    expectTrade(trades[0], 4, 1, 101, 5);
    expectTrade(trades[1], 4, 3, 101, 5);
}

TEST(OrderCancelTest, CancelOnlyOrderAtBestBidMovesBestBid) {
    OrderBook book;
    book.add(buy(1, 100, 10));
    book.add(buy(2, 99, 10));

    EXPECT_TRUE(book.cancel(OrderId{1}));
    auto bestBid = book.bestBid();
    ASSERT_TRUE(bestBid.has_value());
    EXPECT_EQ(*bestBid, Price{99});
}

TEST(OrderCancelTest, CancelOnlyOrderAtBestAskMovesBestAsk) {
    OrderBook book;
    book.add(sell(1, 101, 10));
    book.add(sell(2, 102, 10));

    EXPECT_TRUE(book.cancel(OrderId{1}));
    auto bestAsk = book.bestAsk();
    ASSERT_TRUE(bestAsk.has_value());
    EXPECT_EQ(*bestAsk, Price{102});
}

TEST(OrderCancelTest, CancelledOrderNoLongerTrades) {
    OrderBook book;
    book.add(sell(1, 101, 5));
    book.add(sell(2, 102, 5));

    EXPECT_TRUE(book.cancel(OrderId{1}));
    auto trades = book.add(buy(3, 102, 5));

    ASSERT_EQ(trades.size(), 1u);
    expectTrade(trades[0], 3, 2, 102, 5);
}
