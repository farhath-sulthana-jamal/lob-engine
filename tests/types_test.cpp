#include <gtest/gtest.h>
#include "lob/types.hpp"

#include <type_traits>
#include <concepts>
using namespace lob;

TEST(PriceTest, ComparisonOperators) {
    const Price price1{-10};
    const Price price2{5};
    const Price price3{5};

    EXPECT_LT(price1, price2);
    EXPECT_GT(price2, price1);
    EXPECT_EQ(price2, price3);

    // Negative prices
    EXPECT_LT(Price{-20}, Price{-10});
    EXPECT_GT(Price{-10}, Price{-20});
    EXPECT_EQ(Price{-10}, Price{-10});
}

TEST(QtyTest, EqualityAndInequality) {
    const Qty a{10};
    const Qty b{10};
    const Qty c{20};

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(OrderIdTest, EqualityAndInequality) {
    const OrderId a{10};
    const OrderId b{10};
    const OrderId c{20};

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(OrderTest, AggregateInitialization) {
    const Order order {
        OrderId{1},
        Price{100},
        Qty{10},
        Side{Side::Buy}
    };

    EXPECT_EQ(order.id, OrderId{1});
    EXPECT_EQ(order.price, Price{100});
    EXPECT_EQ(order.qty, Qty{10});
    EXPECT_EQ(order.side, Side::Buy);
}

static_assert(!std::is_convertible_v<int, Price>);
static_assert(!std::is_convertible_v<int, Qty>);
static_assert(!std::is_convertible_v<Price, Qty>);
static_assert(!std::equality_comparable_with<Price, Qty>);
