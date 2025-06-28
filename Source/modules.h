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

namespace util { namespace lang {

namespace detail {

template <typename T>
struct range_iter_base : std::iterator<std::input_iterator_tag, T> {
    range_iter_base(T current) : current(current) { }

    T operator *() const { return current; }

    T const* operator ->() const { return &current; }

    range_iter_base& operator ++() {
        ++current;
        return *this;
    }

    range_iter_base operator ++(int) {
        auto copy = *this;
        ++*this;
        return copy;
    }

    bool operator ==(range_iter_base const& other) const {
        return current == other.current;
    }

    bool operator !=(range_iter_base const& other) const {
        return not (*this == other);
    }

protected:
    T current;
};

} // namespace detail

template <typename T>
struct step_range_proxy {
    struct iterator : detail::range_iter_base<T> {
        iterator(T current, T step)
            : detail::range_iter_base<T>(current), step_(step) { }

        using detail::range_iter_base<T>::current;

        iterator& operator ++() {
            current += step_;
            return *this;
        }

        iterator operator ++(int) {
            auto copy = *this;
            ++*this;
            return copy;
        }

        // Loses commutativity. Iterator-based ranges are simply broken. :-(
        bool operator ==(iterator const& other) const {
            return step_ > 0 ? current >= other.current
                             : current < other.current;
        }

        bool operator !=(iterator const& other) const {
            return not (*this == other);
        }

        T step_;
    };

    step_range_proxy(T begin, T end, T step)
        : begin_(begin, step), end_(end, step) { }

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

template <typename T>
struct range_proxy {
    struct iterator : detail::range_iter_base<T> {
        iterator(T current) : detail::range_iter_base<T>(current) { }
    };

    range_proxy(T begin, T end) : begin_(begin), end_(end) { }

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

template <typename T>
struct step_inf_range_proxy {
    struct iterator : detail::range_iter_base<T> {
        iterator(T current = T(), T step = T())
            : detail::range_iter_base<T>(current), step(step) { }

        using detail::range_iter_base<T>::current;

        iterator& operator ++() {
            current += step;
            return *this;
        }

        iterator operator ++(int) {
            auto copy = *this;
            ++*this;
            return copy;
        }

        bool operator ==(iterator const&) const { return false; }

        bool operator !=(iterator const&) const { return true; }

    private:
        T step;
    };

    step_inf_range_proxy(T begin, T step) : begin_(begin, step) { }

    iterator begin() const { return begin_; }

    iterator end() const { return  iterator(); }

private:
    iterator begin_;
};

template <typename T>
struct infinite_range_proxy {
    struct iterator : detail::range_iter_base<T> {
        iterator(T current = T()) : detail::range_iter_base<T>(current) { }

        bool operator ==(iterator const&) const { return false; }

        bool operator !=(iterator const&) const { return true; }
    };

    infinite_range_proxy(T begin) : begin_(begin) { }

    step_inf_range_proxy<T> step(T step) {
        return {*begin_, step};
    }

    iterator begin() const { return begin_; }

    iterator end() const { return iterator(); }

private:
    iterator begin_;
};

template <typename T, typename U>
auto range(T begin, U end) -> range_proxy<typename std::common_type<T, U>::type> {
    using C = typename std::common_type<T, U>::type;
    return {static_cast<C>(begin), static_cast<C>(end)};
}

template <typename T>
infinite_range_proxy<T> range(T begin) {
    return {begin};
}

namespace traits {

template <typename C>
struct has_size {
    template <typename T>
    static auto check(T*) ->
        typename std::is_integral<
            decltype(std::declval<T const>().size())>::type;

    template <typename>
    static auto check(...) -> std::false_type;

    using type = decltype(check<C>(0));
    static constexpr bool value = type::value;
};

} // namespace traits

template <typename C, typename = typename std::enable_if<traits::has_size<C>::value>>
auto indices(C const& cont) -> range_proxy<decltype(cont.size())> {
    return {0, cont.size()};
}

template <typename T, std::size_t N>
range_proxy<std::size_t> indices(T (&)[N]) {
    return {0, N};
}

template <typename T>
range_proxy<typename std::initializer_list<T>::size_type>
indices(std::initializer_list<T>&& cont) {
    return {0, cont.size()};
}

} } // namespace util::lang

#endif // ndef UTIL_LANG_RANGE_HPP


// Copied from https://stackoverflow.com/questions/15278343/c11-thread-safe-queue


#ifndef SAFE_QUEUE
#define SAFE_QUEUE
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>

// Thread-safe queue
template <typename T>
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
    void push(T item)
    {

        // Acquire lock
        std::unique_lock<std::mutex> lock(m_mutex);

        // Add item
        m_queue.push(item);

        // Notify one thread that
        // is waiting
        m_cond.notify_one();
    }

    // Pops an element off the queue
    T pop()
    {

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
};
#endif


#ifndef ROUND
#define ROUND
template<class T>
static T Round(T a)
{
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