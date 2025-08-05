/*
  ==============================================================================

    modules.h
    Created: 21 Jun 2025 3:04:12pm
    Author:  anony

Credit to https://github.com/klmr/cpp11-range for the range implementation. Content
copied from master/range.hpp (c3b7a35)
  ==============================================================================
*/

#ifndef UTIL_LANG_RANGE_HPP
#define UTIL_LANG_RANGE_HPP

#include <cmath>
#include <iterator>
#include <type_traits>

namespace util {
    namespace lang {
        namespace detail {
            template<typename T>
            struct range_iter_base : std::iterator<std::input_iterator_tag, T> {
                range_iter_base(T current) : current(current) {
                }

                T operator *() const { return current; }

                T const *operator ->() const { return &current; }

                range_iter_base &operator ++() {
                    ++current;
                    return *this;
                }

                range_iter_base operator ++(int) {
                    auto copy = *this;
                    ++*this;
                    return copy;
                }

                bool operator ==(range_iter_base const &other) const {
                    return current == other.current;
                }

                bool operator !=(range_iter_base const &other) const {
                    return not(*this == other);
                }

            protected:
                T current;
            };
        } // namespace detail

        template<typename T>
        struct step_range_proxy {
            struct iterator : detail::range_iter_base<T> {
                iterator(T current, T step)
                    : detail::range_iter_base<T>(current), step_(step) {
                }

                using detail::range_iter_base<T>::current;

                iterator &operator ++() {
                    current += step_;
                    return *this;
                }

                iterator operator ++(int) {
                    auto copy = *this;
                    ++*this;
                    return copy;
                }

                // Loses commutativity. Iterator-based ranges are simply broken. :-(
                bool operator ==(iterator const &other) const {
                    return step_ > 0
                               ? current >= other.current
                               : current < other.current;
                }

                bool operator !=(iterator const &other) const {
                    return not(*this == other);
                }

                T step_;
            };

            step_range_proxy(T begin, T end, T step)
                : begin_(begin, step), end_(end, step) {
            }

            iterator begin() const { return begin_; }

            iterator end() const { return end_; }

            std::size_t size() const {
                if (*end_ >= *begin_) {
                    // Increasing and empty range
                    if (begin_.step_ < T{0}) return 0;
                } else {
                    // Decreasing range
                    if (begin_.step_ > T{0}) return 0;
                }
                return std::ceil(std::abs(static_cast<double>(*end_ - *begin_) / begin_.step_));
            }

        private:
            iterator begin_;
            iterator end_;
        };

        template<typename T>
        struct range_proxy {
            struct iterator : detail::range_iter_base<T> {
                iterator(T current) : detail::range_iter_base<T>(current) {
                }
            };

            range_proxy(T begin, T end) : begin_(begin), end_(end) {
            }

            step_range_proxy<T> step(T step) {
                return {*begin_, *end_, step};
            }

            iterator begin() const { return begin_; }

            iterator end() const { return end_; }

            std::size_t size() const { return *end_ - *begin_; }

        private:
            iterator begin_;
            iterator end_;
        };

        template<typename T>
        struct step_inf_range_proxy {
            struct iterator : detail::range_iter_base<T> {
                iterator(T current = T(), T step = T())
                    : detail::range_iter_base<T>(current), step(step) {
                }

                using detail::range_iter_base<T>::current;

                iterator &operator ++() {
                    current += step;
                    return *this;
                }

                iterator operator ++(int) {
                    auto copy = *this;
                    ++*this;
                    return copy;
                }

                bool operator ==(iterator const &) const { return false; }

                bool operator !=(iterator const &) const { return true; }

            private:
                T step;
            };

            step_inf_range_proxy(T begin, T step) : begin_(begin, step) {
            }

            iterator begin() const { return begin_; }

            iterator end() const { return iterator(); }

        private:
            iterator begin_;
        };

        template<typename T>
        struct infinite_range_proxy {
            struct iterator : detail::range_iter_base<T> {
                iterator(T current = T()) : detail::range_iter_base<T>(current) {
                }

                bool operator ==(iterator const &) const { return false; }

                bool operator !=(iterator const &) const { return true; }
            };

            infinite_range_proxy(T begin) : begin_(begin) {
            }

            step_inf_range_proxy<T> step(T step) {
                return {*begin_, step};
            }

            iterator begin() const { return begin_; }

            iterator end() const { return iterator(); }

        private:
            iterator begin_;
        };

        template<typename T, typename U>
        auto range(T begin, U end) -> range_proxy<typename std::common_type<T, U>::type> {
            using C = typename std::common_type<T, U>::type;
            return {static_cast<C>(begin), static_cast<C>(end)};
        }

        template<typename T>
        infinite_range_proxy<T> range(T begin) {
            return {begin};
        }

        namespace traits {
            template<typename C>
            struct has_size {
                template<typename T>
                static auto check(T *) ->
                    typename std::is_integral<
                        decltype(std::declval<T const>().size())>::type;

                template<typename>
                static auto check(...) -> std::false_type;

                using type = decltype(check<C>(0));
                static constexpr bool value = type::value;
            };
        } // namespace traits

        template<typename C, typename = typename std::enable_if<traits::has_size<C>::value> >
        auto indices(C const &cont) -> range_proxy<decltype(cont.size())> {
            return {0, cont.size()};
        }

        template<typename T, std::size_t N>
        range_proxy<std::size_t> indices(T (&)[N]) {
            return {0, N};
        }

        template<typename T>
        range_proxy<typename std::initializer_list<T>::size_type>
        indices(std::initializer_list<T> &&cont) {
            return {0, cont.size()};
        }
    }
} // namespace util::lang

#endif // ndef UTIL_LANG_RANGE_HPP


// Copied from https://stackoverflow.com/questions/15278343/c11-thread-safe-queue
#ifndef SAFE_QUEUE
#define SAFE_QUEUE
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>

// Thread-safe queue
template<typename T>
class TSQueue {
private:
    // Underlying queue
    std::queue<T> m_queue;

    // mutex for thread synchronization
    std::mutex m_mutex;

    // Condition variable for signaling
    std::condition_variable m_cond;

public:
    // Pushes an element to the queue
    void push(T item) {
        // Acquire lock
        std::unique_lock<std::mutex> lock(m_mutex);

        // Add item
        m_queue.push(item);

        // Notify one thread that
        // is waiting
        m_cond.notify_one();
    }

    // Pops an element off the queue
    T pop() {
        // acquire lock
        std::unique_lock<std::mutex> lock(m_mutex);

        // wait until queue is not empty
        m_cond.wait(lock,
                    [this]() { return !m_queue.empty(); });

        // retrieve item
        T item = m_queue.front();
        m_queue.pop();

        // return item
        return item;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }
};
#endif


#ifndef ROUND
#define ROUND

template<class T>
static T Round(T a) {
    static_assert(std::is_floating_point<T>::value, "Round<T>: T must be floating point");

    return (a > 0) ? ::floor(a + static_cast<T>(0.5)) : ::ceil(a - static_cast<T>(0.5));
}


template<typename T>
static T roundTo(T value, int digits) {
    static_assert(std::is_floating_point<T>::value, "Round<T>: T must be floating point");

    const T shift = pow(static_cast<T>(10.0), digits);

    return Round(value * shift) / shift;
}
#endif


#ifndef UUID
#define UUID

// The below code was generated by Gemini 2.5 Flash. Thanks!
#include <string>      // For std::string
#include <random>      // For std::random_device, std::mt19937, std::uniform_int_distribution
#include <array>       // For std::array
#include <algorithm>   // For std::generate (used in seeding)
#include <functional>  // For std::ref (used in seeding)

// --- Helper for fast byte-to-hexadecimal conversion ---
// This static array holds hexadecimal characters '0'-'9' and 'a'-'f'.
// It's used for efficient lookup during byte-to-hex conversion.
static const char hex_chars_lookup[] = "0123456789abcdef";

/**
 * @brief Converts a single byte into two hexadecimal characters and writes them
 * directly into the provided character buffer.
 * @param byte The unsigned 8-bit integer (byte) to convert.
 * @param out_buf A pointer to a character buffer where the two hex characters
 * will be written. It must have space for at least 2 characters.
 */
inline void byteToHexChars(uint8_t byte, char *out_buf) {
    // Extract the higher nibble (4 bits) and use it as an index into hex_chars_lookup.
    out_buf[0] = hex_chars_lookup[(byte >> 4) & 0x0F];
    // Extract the lower nibble (4 bits) and use it as an index.
    out_buf[1] = hex_chars_lookup[byte & 0x0F];
}

// --- UUIDGenerator Class ---
class UUIDGenerator {
public:
    /**
     * @brief Constructs a UUIDGenerator object.
     * The random number engine is seeded only once during construction
     * using std::random_device for cryptographic-quality initial entropy.
     */
    UUIDGenerator() {
        std::random_device rd; // Provides non-deterministic random numbers (for seeding)

        // Use a seed sequence to properly initialize std::mt19937 with multiple
        // random_device results for better initial entropy, especially important
        // if std::random_device is not truly high-quality or if many generators
        // are created in quick succession.
        std::array<unsigned int, std::mt19937::state_size> seed_data;
        std::generate(seed_data.begin(), seed_data.end(), std::ref(rd));
        std::seed_seq seq(seed_data.begin(), seed_data.end());
        rand_engine_.seed(seq);
    }

    /**
     * @brief Generates a new Version 4 (randomly generated) UUID string.
     * The format is "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx", where 'x' is a
     * random hexadecimal digit, '4' is the UUID version, and 'y' is a
     * hexadecimal digit representing the variant (8, 9, A, or B).
     * @return A std::string containing the newly generated UUID.
     */
    std::string generate() {
        // A UUID consists of 16 bytes.
        std::array<uint8_t, 16> uuid_bytes;

        // The C++ standard library does not allow uint8_t for uniform_int_distribution.
        // The fix is to use a larger integer type (like int) and cast the result.
        std::uniform_int_distribution<int> dist(0, 255);

        // Fill all 16 bytes of the UUID with random data.
        for (int i = 0; i < 16; ++i) {
            uuid_bytes[i] = static_cast<uint8_t>(dist(rand_engine_));
        }

        // --- Apply UUID version (4) and variant (RFC 4122) bits ---
        // For Version 4 UUIDs, the 4 bits of the 'time_hi_and_version' field
        // (which is byte 6 in the UUID byte array) must be set to `0100` (hex 4).
        // This is done by clearing the top 4 bits (AND with 0x0F) and then
        // setting them to 0x40 (OR with 0x40).
        uuid_bytes[6] = (uuid_bytes[6] & 0x0F) | 0x40;

        // For RFC 4122 compliant UUIDs, the 2 most significant bits of the
        // 'clock_seq_hi_and_reserved' field (byte 8 in the UUID byte array)
        // must be set to `10` (binary). This results in a hexadecimal value
        // of 8, 9, A, or B for that nibble.
        // This is done by clearing the top 2 bits (AND with 0x3F) and then
        // setting them to 0x80 (OR with 0x80).
        uuid_bytes[8] = (uuid_bytes[8] & 0x3F) | 0x80;

        // --- Construct the UUID string ---
        // A UUID string is always 36 characters long (32 hex digits + 4 hyphens).
        // Initialize with hyphens and pre-allocate the correct size for efficiency.
        std::string uuid_str(36, '-');

        // Populate the string segments by converting bytes to hex characters
        // and placing them at the correct positions.

        // Segment 1: Bytes 0-3 (8 characters)
        byteToHexChars(uuid_bytes[0], &uuid_str[0]);
        byteToHexChars(uuid_bytes[1], &uuid_str[2]);
        byteToHexChars(uuid_bytes[2], &uuid_str[4]);
        byteToHexChars(uuid_bytes[3], &uuid_str[6]);

        // Segment 2: Bytes 4-5 (4 characters)
        byteToHexChars(uuid_bytes[4], &uuid_str[9]);
        byteToHexChars(uuid_bytes[5], &uuid_str[11]);

        // Segment 3: Bytes 6-7 (4 characters) - Includes version '4'
        byteToHexChars(uuid_bytes[6], &uuid_str[14]);
        byteToHexChars(uuid_bytes[7], &uuid_str[16]);

        // Segment 4: Bytes 8-9 (4 characters) - Includes variant '8,9,a,b'
        byteToHexChars(uuid_bytes[8], &uuid_str[19]);
        byteToHexChars(uuid_bytes[9], &uuid_str[21]);

        // Segment 5: Bytes 10-15 (12 characters)
        byteToHexChars(uuid_bytes[10], &uuid_str[24]);
        byteToHexChars(uuid_bytes[11], &uuid_str[26]);
        byteToHexChars(uuid_bytes[12], &uuid_str[28]);
        byteToHexChars(uuid_bytes[13], &uuid_str[30]);
        byteToHexChars(uuid_bytes[14], &uuid_str[32]);
        byteToHexChars(uuid_bytes[15], &uuid_str[34]);

        return uuid_str;
    }

private:
    std::mt19937 rand_engine_; // Mersenne Twister pseudo-random number generator
};
#endif

#ifndef DraggableList
#define DraggableList
#pragma once
#include "JuceHeader.h"

// Your item-data container must inherit from this, and override at least the first
// four member functions.
struct DraggableListBoxItemData {
    virtual ~DraggableListBoxItemData() = 0;

    virtual int getNumItems() = 0;

    virtual void paintContents(int, Graphics &, Rectangle<int>) = 0;

    virtual void moveBefore(int indexOfItemToMove, int indexOfItemToPlaceBefore) = 0;

    virtual void moveAfter(int indexOfItemToMove, int indexOfItemToPlaceAfter) = 0;

    // If you need a dynamic list, override these functions as well.
    virtual void deleteItem(int /*indexOfItemToDelete*/) {
    };

    virtual void addItemAtEnd() {
    };
};

// DraggableListBox is basically just a ListBox, that inherits from DragAndDropContainer.
// Declare your list box using this type.
class DraggableListBox : public ListBox, public DragAndDropContainer {
public:
    DraggableListBox() { setColour(backgroundColourId, Colour(0.f, 0.f, 0.f, 0.f)); }

    void dragOperationEnded(const DragAndDropTarget::SourceDetails &src) override {
        repaint(); // Buttons are not always reset upon a drag end, so manually repaint().
    };
};

// Everything below this point should be generic.
class DraggableListBoxItem : public Component, public DragAndDropTarget {
public:
    DraggableListBoxItem(DraggableListBox &lb, DraggableListBoxItemData &data, int rn)
        : rowNum(rn), modelData(data), listBox(lb) {
    }

    // Component
    void paint(Graphics &g) override;

    void mouseEnter(const MouseEvent &) override;

    void mouseExit(const MouseEvent &) override;

    void mouseDrag(const MouseEvent &) override;

    // DragAndDropTarget
    bool isInterestedInDragSource(const SourceDetails &) override { return true; }

    void itemDragEnter(const SourceDetails &) override;

    void itemDragMove(const SourceDetails &) override;

    void itemDragExit(const SourceDetails &) override;

    void itemDropped(const SourceDetails &) override;

    bool shouldDrawDragImageWhenOver() override { return true; }

    // DraggableListBoxItem
protected:
    void updateInsertLines(const SourceDetails &dragSourceDetails);

    void hideInsertLines();

    int rowNum;
    DraggableListBoxItemData &modelData;
    DraggableListBox &listBox;

    MouseCursor savedCursor;
    bool insertAfter = false;
    bool insertBefore = false;
    Image prerendered;
};

class DraggableListBoxModel : public ListBoxModel {
public:
    DraggableListBoxModel(DraggableListBox &lb, DraggableListBoxItemData &md)
        : listBox(lb), modelData(md) {
    }

    int getNumRows() override { return modelData.getNumItems(); }

    void paintListBoxItem(int, Graphics &, int, int, bool) override {
    }

    Component *refreshComponentForRow(int, bool, Component *) override;

protected:
    // Draggable model has a reference to its owner ListBox, so it can tell it to update after DnD.
    DraggableListBox &listBox;

    // It also has a reference to the model data, which it uses to get the current items count,
    // and which it passes to the DraggableListBoxItem objects it creates/updates.
    DraggableListBoxItemData &modelData;
};
#endif


#ifndef Constants
#define Constants
#pragma once

namespace NumericLimits {
    constexpr int INTMAX = std::numeric_limits<int>::max();
    constexpr int INTMIN = std::numeric_limits<int>::min();
    constexpr float FLOATMAX = std::numeric_limits<float>::max();
    constexpr float FLOATMIN = std::numeric_limits<float>::min();
}
#endif

#ifndef miscellanouesutils
#define miscellaneousutils

template<typename KeyType, typename ValueType>
std::optional<KeyType> findKeyByValue(const std::map<KeyType, ValueType> &myMap, const ValueType &targetValue) {
    auto it = std::find_if(myMap.begin(), myMap.end(),
                           [&](const std::pair<const KeyType, ValueType> &pair) {
                               return pair.second == targetValue;
                           });

    if (it != myMap.end()) {
        return it->first;
    }
    return std::nullopt; // Value not found
}

template<typename KeyType, typename ValueType>
std::optional<KeyType> findKeyByValue(const std::unordered_map<KeyType, ValueType> &myMap,
                                      const ValueType &targetValue) {
    auto it = std::find_if(myMap.begin(), myMap.end(),
                           [&](const std::pair<const KeyType, ValueType> &pair) {
                               return pair.second == targetValue;
                           });

    if (it != myMap.end()) {
        return it->first;
    }
    return std::nullopt; // Value not found
}

#endif
