#pragma once

#include <bitset>
#include <limits>

namespace ECS {
    template <uint16_t MaxComponents>
    struct SignatureBitset {
        std::bitset<MaxComponents> bitset;
        uint16_t lowestBit = std::numeric_limits<uint16_t>::max();
        uint16_t highestBit = 0;

        void set(std::size_t bit) {
            bitset.set(bit);
            if (bit < lowestBit) {
                lowestBit = static_cast<uint16_t>(bit);
            }
            if (bit > highestBit) {
                highestBit = static_cast<uint16_t>(bit);
            }
        }

        void reset(std::size_t bit) {
            bitset.reset(bit);
            if (bit == lowestBit) {
                bool found = false;
                for (uint16_t i = static_cast<uint16_t>(bit) + 1; i < MaxComponents; ++i) {
                    if (bitset.test(i)) {
                        lowestBit = i;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    lowestBit = std::numeric_limits<uint16_t>::max();
                    highestBit = 0;
                }
            }

            if (bit == highestBit) {
                bool found = false;
                for (int i = static_cast<int>(bit) - 1; i >= 0; --i) {
                    if (bitset.test(static_cast<size_t>(i))) {
                        highestBit = static_cast<uint16_t>(i);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    highestBit = 0;
                    if (!bitset.any()) {
                        lowestBit = std::numeric_limits<uint16_t>::max();
                    }
                }
            }
        }

        bool none() const { return bitset.none(); }

        template<typename Func>
        void forEachSetBit(Func f) const {
            if (none()) return;

            for (uint16_t i = lowestBit; i <= highestBit; ++i) {
                if (bitset.test(i)) {
                    f(i);
                }
            }
        }

        bool operator[](std::size_t pos) const { return bitset.test(pos); }
        typename std::bitset<MaxComponents>::reference operator[](std::size_t pos) { return bitset[pos]; }

        size_t count() const { return bitset.count(); }
        bool test(std::size_t pos) const { return bitset.test(pos); }

        void recalcBounds() {
            if (bitset.none()) {
                lowestBit = std::numeric_limits<uint16_t>::max();
                highestBit = 0;
                return;
            }

            for (uint16_t i = 0; i < MaxComponents; ++i) {
                if (bitset.test(i)) {
                    lowestBit = i;
                    break;
                }
            }

            for (int i = MaxComponents - 1; i >= 0; --i) {
                if (bitset.test(i)) {
                    highestBit = static_cast<uint16_t>(i);
                    break;
                }
            }
        }

        SignatureBitset& operator&=(const SignatureBitset& other) {
            bitset &= other.bitset;
            recalcBounds();
            return *this;
        }

        SignatureBitset& operator|=(const SignatureBitset& other) {
            bitset |= other.bitset;
            lowestBit = std::min(lowestBit, other.lowestBit);
            highestBit = std::max(highestBit, other.highestBit);
            return *this;
        }

        friend SignatureBitset operator&(const SignatureBitset& lhs, const SignatureBitset& rhs) {
            SignatureBitset result;
            result.bitset = lhs.bitset & rhs.bitset;
            result.recalcBounds();
            return result;
        }

        friend SignatureBitset operator|(const SignatureBitset& lhs, const SignatureBitset& rhs) {
            SignatureBitset result;
            result.bitset = lhs.bitset | rhs.bitset;
            result.recalcBounds();
            return result;
        }

        friend bool operator==(const SignatureBitset& lhs, const SignatureBitset& rhs) {
            return lhs.bitset == rhs.bitset;
        }

        bool operator<(const SignatureBitset& other) const {
            return bitset < other.bitset;
        }
    };
}

namespace std {
    template <uint16_t MaxComponents>
    struct hash<ECS::SignatureBitset<MaxComponents>> {
        size_t operator()(const ECS::SignatureBitset<MaxComponents>& wrapper) const {
            return std::hash<std::bitset<MaxComponents>>{}(wrapper.bitset);
        }
    };
}