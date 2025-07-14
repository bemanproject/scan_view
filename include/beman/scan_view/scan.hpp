// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SCAN_VIEW_SCAN_HPP
#define BEMAN_SCAN_VIEW_SCAN_HPP

#include <algorithm>
#include <functional>
#include <ranges>
#include <utility>

namespace beman::scan_view {

namespace detail {

#if defined(_LIBCPP_VERSION)

template <typename T>
using movable_box = std::ranges::__movable_box<T>;

template <bool Const, typename T>
using maybe_const = std::__maybe_const<Const, T>;

#elif defined(__GLIBCXX__)

template <typename T>
using movable_box = std::ranges::__detail::__box<T>;

template <bool Const, typename T>
using maybe_const = std::ranges::__detail::__maybe_const_t<Const, T>;

#elif defined(_MSC_VER) && defined(_MSVC_STL_UPDATE)

template <typename T>
using movable_box = std::ranges::_Movable_box<T>;

template <bool Const, typename T>
using maybe_const = std::_Maybe_const<Const, T>;

#endif

} // namespace detail

template <typename V, typename F, typename T, typename U>
concept scannable_impl = // exposition only
    std::movable<U> && std::convertible_to<T, U> && std::invocable<F&, U, std::ranges::range_reference_t<V>> &&
    std::assignable_from<U&, std::invoke_result_t<F&, U, std::ranges::range_reference_t<V>>>;

template <typename V, typename F, typename T>
concept scannable = // exposition only
    std::invocable<F&, T, std::ranges::range_reference_t<V>> &&
    std::convertible_to<std::invoke_result_t<F&, T, std::ranges::range_reference_t<V>>,
                        std::decay_t<std::invoke_result_t<F&, T, std::ranges::range_reference_t<V>>>> &&
    scannable_impl<V, F, T, std::decay_t<std::invoke_result_t<F&, T, std::ranges::range_reference_t<V>>>>;

template <std::ranges::input_range V, std::move_constructible F, std::move_constructible T, bool IsInit = false>
    requires std::ranges::view<V> && std::is_object_v<F> && std::is_object_v<T> && scannable<V, F, T>
class scan_view : public std::ranges::view_interface<scan_view<V, F, T, IsInit>> {
  private:
    // [range.scan.iterator], class template scan_view::iterator
    template <bool>
    class iterator; // exposition only

    V                      base_ = V(); // exposition only
    detail::movable_box<F> fun_;        // exposition only
    detail::movable_box<T> init_;       // exposition only

  public:
    scan_view()
        requires std::default_initializable<V> && std::default_initializable<F>
    = default;
    constexpr explicit scan_view(V base, F fun)
        requires(!IsInit)
        : base_{std::move(base)}, fun_{std::in_place, std::move(fun)} {}
    constexpr explicit scan_view(V base, F fun, T init)
        requires IsInit
        : base_{std::move(base)}, fun_{std::in_place, std::move(fun)}, init_{std::in_place, std::move(init)} {}

    constexpr V base() const&
        requires std::copy_constructible<V>
    {
        return base_;
    }
    constexpr V base() && { return std::move(base_); }

    constexpr iterator<false> begin() { return iterator<false>{*this, std::ranges::begin(base_)}; }
    constexpr iterator<true>  begin() const
        requires std::ranges::range<const V> && scannable<const V, const F, T>
    {
        return iterator<true>{*this, std::ranges::begin(base_)};
    }

    constexpr std::default_sentinel_t end() const noexcept { return std::default_sentinel; }

    constexpr auto size()
        requires std::ranges::sized_range<V>
    {
        return std::ranges::size(base_);
    }
    constexpr auto size() const
        requires std::ranges::sized_range<const V>
    {
        return std::ranges::size(base_);
    }

    /*constexpr auto reserve_hint() requires approximately_sized_range<V>;
    { return ranges::reserve_hint(base_); }

    constexpr auto reserve_hint() const requires approximately_sized_range<const V>;
    { return ranges::reserve_hint(base_); }*/
};

template <class R, class F>
scan_view(R&&, F) -> scan_view<std::views::all_t<R>, F, std::ranges::range_value_t<R>, false>;
template <class R, class F, class T>
scan_view(R&&, F, T) -> scan_view<std::views::all_t<R>, F, T, true>;

template <std::ranges::input_range V, std::move_constructible F, std::move_constructible T, bool IsInit>
    requires std::ranges::view<V> && std::is_object_v<F> && std::is_object_v<T> && scannable<V, F, T>
template <bool Const>
class scan_view<V, F, T, IsInit>::iterator {
  private:
    using Parent     = detail::maybe_const<Const, scan_view>; // exposition only
    using Base       = detail::maybe_const<Const, V>;         // exposition only
    using ResultType = std::decay_t<
        std::invoke_result_t<detail::maybe_const<Const, F>&, T, std::ranges::range_reference_t<Base>>>; // exposition
                                                                                                        // only

    std::ranges::iterator_t<Base>   current_ = std::ranges::iterator_t<Base>(); // exposition only
    Parent*                         parent_  = nullptr;                         // exposition only
    detail::movable_box<ResultType> sum_;                                       // exposition only

  public:
    using iterator_concept =
        std::conditional_t<std::ranges::forward_range<Base>, std::forward_iterator_tag, std::input_iterator_tag>;
    using iterator_category = std::conditional_t<
        std::derived_from<std::forward_iterator_tag,
                          typename std::iterator_traits<std::ranges::iterator_t<Base>>::iterator_category> &&
            std::is_reference_v<std::invoke_result_t<detail::maybe_const<Const, F>&,
                                                     detail::maybe_const<Const, T>&,
                                                     std::ranges::range_reference_t<Base>>>,
        std::forward_iterator_tag,
        std::input_iterator_tag>; // present only if Base models forward_range
    using value_type      = ResultType;
    using difference_type = std::ranges::range_difference_t<Base>;

    iterator()
        requires std::default_initializable<std::ranges::iterator_t<Base>>
    = default;
    constexpr iterator(Parent& parent, std::ranges::iterator_t<Base> current)
        : current_{std::move(current)}, parent_{std::addressof(parent)} {
        if (current_ == std::ranges::end(parent_->base_))
            return;
        if constexpr (IsInit) {
            sum_ = detail::movable_box<ResultType>{std::in_place,
                                                   std::invoke(*parent_->fun_, *parent_->init_, *current_)};
        } else {
            sum_ = detail::movable_box<ResultType>{std::in_place, *current_};
        }
    }
    constexpr iterator(iterator<!Const> i)
        requires Const && std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<Base>>
        : current_{std::move(i.current_)}, parent_{i.parent_}, sum_{std::move(i.sum_)} {}

    constexpr const std::ranges::iterator_t<Base>& base() const& noexcept { return current_; }
    constexpr std::ranges::iterator_t<Base>        base() && { return std::move(current_); }

    constexpr const value_type& operator*() const { return *sum_; }

    constexpr iterator& operator++() {
        if (++current_ != std::ranges::end(parent_->base_)) {
            sum_ = detail::movable_box<ResultType>{std::in_place,
                                                   std::invoke(*parent_->fun_, std::move(*sum_), *current_)};
        }
        return *this;
    }
    constexpr void     operator++(int) { ++*this; }
    constexpr iterator operator++(int)
        requires std::ranges::forward_range<Base>
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y)
        requires std::equality_comparable<std::ranges::iterator_t<Base>>
    {
        return x.current_ == y.current_;
    }
    friend constexpr bool operator==(const iterator& x, std::default_sentinel_t) {
        return x.current_ == std::ranges::end(x.parent_->base_);
    }
};

inline constexpr auto scan = [](auto&& E, auto&& F) {
    return scan_view{std::forward<decltype(E)>(E), std::forward<decltype(F)>(F)};
};
inline constexpr auto prescan = [](auto&& E, auto&& F, auto&& G) {
    return scan_view{std::forward<decltype(E)>(E), std::forward<decltype(F)>(F), std::forward<decltype(G)>(G)};
};
inline constexpr auto partial_sum = [](auto&& E) { return scan_view{std::forward<decltype(E)>(E), std::plus{}}; };

} // namespace beman::scan_view

#endif // BEMAN_SCAN_VIEW_SCAN_HPP
