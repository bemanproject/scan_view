// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SCAN_VIEW_SCAN_HPP
#define BEMAN_SCAN_VIEW_SCAN_HPP

#include <beman/scan_view/config.hpp>

#if BEMAN_SCAN_VIEW_USE_MODULES() && !defined(BEMAN_SCAN_VIEW_INCLUDED_FROM_INTERFACE_UNIT)

import beman.scan_view;

#else

    #if !BEMAN_SCAN_VIEW_USE_MODULES()

        #include <algorithm>
        #include <concepts>
        #include <functional>
        #include <ranges>
        #include <type_traits>
        #include <utility>

    #endif // !BEMAN_SCAN_VIEW_USE_MODULES()

    #include "detail/expo_only.hpp"

namespace beman::scan_view {

enum class scan_view_kind : bool { unseeded, seeded };

template <typename V, typename F, typename T, typename U>
concept scannable_impl = // exposition only
    std::movable<U> && std::convertible_to<T, U> && std::invocable<F&, U, std::ranges::range_reference_t<V> > &&
    std::assignable_from<U&, std::invoke_result_t<F&, U, std::ranges::range_reference_t<V> > >;

template <typename V, typename F, typename T>
concept scannable = // exposition only
    std::invocable<F&, T, std::ranges::range_reference_t<V> > &&
    std::convertible_to<std::invoke_result_t<F&, T, std::ranges::range_reference_t<V> >,
                        std::decay_t<std::invoke_result_t<F&, T, std::ranges::range_reference_t<V> > > > &&
    scannable_impl<V, F, T, std::decay_t<std::invoke_result_t<F&, T, std::ranges::range_reference_t<V> > > >;

template <std::ranges::input_range V,
          std::move_constructible  F,
          std::move_constructible  T,
          scan_view_kind           K = scan_view_kind::unseeded>
    requires std::ranges::view<V> && std::is_object_v<F> && std::is_object_v<T> && scannable<V, F, T>
class scan_view : public std::ranges::view_interface<scan_view<V, F, T, K> > {
  private:
    // [range.scan.iterator], class template scan_view::iterator
    template <bool>
    class iterator; // exposition only

    template <bool>
    friend class iterator; // exposition only

    V                      base_ = V(); // exposition only
    detail::movable_box<F> fun_;        // exposition only
    detail::movable_box<T> init_;       // exposition only

  public:
    scan_view()
        requires std::default_initializable<V> && std::default_initializable<F>
    = default;
    constexpr explicit scan_view(V base, F fun)
        requires(K == scan_view_kind::unseeded)
        : base_{std::move(base)}, fun_{std::in_place, std::move(fun)} {}
    constexpr explicit scan_view(V base, F fun, T init)
        requires(K == scan_view_kind::seeded)
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

    [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept { return std::default_sentinel; }

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

    #if __cpp_lib_ranges_reserve_hint >= 202502L
    constexpr auto reserve_hint()
        requires std::ranges::approximately_sized_range<V>
    {
        return std::ranges::reserve_hint(base_);
    }

    constexpr auto reserve_hint() const
        requires std::ranges::approximately_sized_range<const V>
    {
        return std::ranges::reserve_hint(base_);
    }
    #endif
};

template <class R, class F>
scan_view(R&&, F) -> scan_view<std::views::all_t<R>, F, std::ranges::range_value_t<R> >;
template <class R, class F, class T>
scan_view(R&&, F, T) -> scan_view<std::views::all_t<R>, F, T, scan_view_kind::seeded>;

template <std::ranges::input_range V, std::move_constructible F, std::move_constructible T, scan_view_kind K>
    requires std::ranges::view<V> && std::is_object_v<F> && std::is_object_v<T> && scannable<V, F, T>
template <bool Const>
class scan_view<V, F, T, K>::iterator {
  private:
    using Parent     = detail::maybe_const<Const, scan_view>; // exposition only
    using Base       = detail::maybe_const<Const, V>;         // exposition only
    using Func       = detail::maybe_const<Const, F>;         // exposition only
    using ResultType = std::decay_t<                          // exposition only
        std::invoke_result_t<Func&, T, std::ranges::range_reference_t<Base> > >;

    struct Holder {                                                           // exposition only
        std::ranges::sentinel_t<Base> end_ = std::ranges::sentinel_t<Base>(); // exposition only
        detail::movable_box<F>        fun_;                                   // exposition only
        detail::movable_box<T>        init_;                                  // exposition only
    };
    using HolderType = std::conditional_t<detail::tidy_func<F>, Holder, Parent*>; // exposition only

    std::ranges::iterator_t<Base>   current_ = std::ranges::iterator_t<Base>(); // exposition only
    HolderType                      parent_  = {};                              // exposition only
    detail::movable_box<ResultType> sum_;                                       // exposition only

    constexpr std::ranges::sentinel_t<Base> get_end() const { // exposition only
        if constexpr (detail::tidy_func<F>)
            return parent_.end_;
        else
            return std::ranges::end(parent_->base_);
    }
    constexpr Func& get_fun() { // exposition only
        if constexpr (detail::tidy_func<F>)
            return *parent_.fun_;
        else
            return *parent_->fun_;
    }
    constexpr T& get_init() { // exposition only
        if constexpr (detail::tidy_func<F>)
            return *parent_.init_;
        else
            return *parent_->init_;
    }
    static constexpr HolderType init(Parent& parent) { // exposition only
        if constexpr (detail::tidy_func<F>)
            return {std::ranges::end(parent.base_), detail::movable_box<F>{std::in_place}, parent.init_};
        else
            return std::addressof(parent);
    }

  public:
    using iterator_concept = std::input_iterator_tag;
    using value_type       = ResultType;
    using difference_type  = std::ranges::range_difference_t<Base>;

    iterator()
        requires std::default_initializable<std::ranges::iterator_t<Base> >
    = default;
    constexpr iterator(Parent& parent, std::ranges::iterator_t<Base> current)
        : current_{std::move(current)}, parent_{init(parent)} {
        if (current_ == get_end())
            return;
        if constexpr (K == scan_view_kind::seeded) {
            sum_ = detail::movable_box<ResultType>{std::in_place, std::invoke(get_fun(), get_init(), *current_)};
        } else {
            sum_ = detail::movable_box<ResultType>{std::in_place, *current_};
        }
    }
    constexpr iterator(iterator<!Const> i)
        requires Const && std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<Base> >
        : current_{std::move(i.current_)}, parent_{std::move(i.parent_)}, sum_{std::move(i.sum_)} {}

    constexpr const std::ranges::iterator_t<Base>& base() const& noexcept { return current_; }
    constexpr std::ranges::iterator_t<Base>        base() && { return std::move(current_); }

    constexpr const value_type& operator*() const { return *sum_; }

    constexpr iterator& operator++() {
        if (++current_ != get_end()) {
            sum_ = detail::movable_box<ResultType>{std::in_place, std::invoke(get_fun(), std::move(*sum_), *current_)};
        }
        return *this;
    }
    constexpr void operator++(int) { ++*this; }

    friend constexpr bool operator==(const iterator& x, const iterator& y)
        requires std::equality_comparable<std::ranges::iterator_t<Base> >
    {
        return x.current_ == y.current_;
    }
    friend constexpr bool operator==(const iterator& x, std::default_sentinel_t) { return x.current_ == x.get_end(); }
};

namespace detail {

struct scan_t {
    constexpr scan_t() = default;
    constexpr auto operator()(std::ranges::input_range auto&& E, auto&& F) const {
        return scan_view{std::forward<decltype(E)>(E), std::forward<decltype(F)>(F)};
    }
    constexpr auto operator()(std::ranges::input_range auto&& E, auto&& F, auto&& G) const {
        return scan_view{std::forward<decltype(E)>(E), std::forward<decltype(F)>(F), std::forward<decltype(G)>(G)};
    }

    constexpr auto operator()(auto&& E) const {
        return detail::range_adaptor_closure_t(detail::bind_back(*this, std::forward<decltype(E)>(E)));
    }
    constexpr auto operator()(auto&& E, auto&& F) const {
        return detail::range_adaptor_closure_t(
            detail::bind_back(*this, std::forward<decltype(E)>(E), std::forward<decltype(F)>(F)));
    }
};

} // namespace detail

inline constexpr detail::scan_t scan{};

} // namespace beman::scan_view

// Conditionally borrowed range (P3117)
template <std::ranges::input_range         V,
          std::move_constructible          F,
          std::move_constructible          T,
          beman::scan_view::scan_view_kind K>
constexpr bool std::ranges::enable_borrowed_range<beman::scan_view::scan_view<V, F, T, K> > =
    std::ranges::enable_borrowed_range<V> && beman::scan_view::detail::tidy_func<F>;

#endif // #if BEMAN_SCAN_VIEW_USE_MODULES() &&
       // !defined(BEMAN_SCAN_VIEW_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_SCAN_VIEW_SCAN_HPP
