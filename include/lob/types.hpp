#pragma once

#include <cstdint>
#include <compare>

namespace lob {
    // Price in integer ticks (no floats: 0.1+0.2 != 0.3). Signed because real prices can go
    // negative (spreads, WTI oil Apr 2020). 64-bit gives headroom for tick-scaled values.
    class Price {
        std::int64_t value;

        public:
            constexpr explicit Price(std::int64_t v) : value(v) {}
            constexpr std::int64_t getValue() const noexcept {
                return value;
            }
            auto operator<=>(const Price&) const = default;
    };
    // Signed so an overfill bug shows up as a negative value an invariant check can catch,
    // instead of silently wrapping to ~4 billion. 64-bit so sums across a price level can't overflow.
    class Qty {
        std::int64_t value;

        public:
            constexpr explicit Qty(std::int64_t v) : value(v) {}
            constexpr std::int64_t getValue() const noexcept {
                return value;
            }
            auto operator<=>(const Qty&) const = default;
    };

    class OrderId {
        std::uint64_t value;

        public:
            constexpr explicit OrderId(std::uint64_t v) : value(v) {}
            constexpr std::uint64_t getValue() const noexcept {
                return value;
            }
            auto operator<=>(const OrderId&) const = default;
    };

    enum class Side : std::uint8_t {Buy, Sell};

    static_assert(sizeof(Side) == 1);

    struct Order {
        OrderId id;
        Price price;
        Qty qty;
        Side side;
    };

    static_assert(sizeof(Order) == 32);

    struct Trade {
        OrderId buy_order_id;
        OrderId sell_order_id;
        Price price;
        Qty qty;
    };

    static_assert(sizeof(Trade) == 32);

} // namespace lob