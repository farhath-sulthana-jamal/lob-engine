#pragma once

#include <cstdint>

namespace lob {
    // Price in integer ticks (no floats: 0.1+0.2 != 0.3). Signed because real prices can go
    // negative (spreads, WTI oil Apr 2020). 64-bit gives headroom for tick-scaled values.
    class Price {
        std::int64_t value; // price can have negative values as well

        public:
            explicit Price(std::int64_t v) : value(v) {}
            std::int64_t getValue() const {
                return value;
            }
    };
    // Signed so an overfill bug shows up as a negative value an invariant check can catch,
    // instead of silently wrapping to ~4 billion. 64-bit so sums across a price level can't overflow.
    class Qty {
        std::int64_t value; // qty can have negative values as well

        public:
            explicit Qty(std::int64_t v) : value(v) {}
            std::int64_t getValue() const { 
                return value;
            }
    };
} // namespace lob.