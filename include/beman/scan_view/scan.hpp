// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_SCAN_VIEW_SCAN_HPP
#define BEMAN_SCAN_VIEW_SCAN_HPP

#include <algorithm>
#include <concepts>
#include <functional>
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>

namespace beman::scan_view {

namespace detail {

// until C++23, `__movable_box` was named `__copyable_box` and required the stored type to be copy-constructible, not
// just move-constructible; we preserve the old behavior in pre-C++23 modes.
template <class Tp>
concept movable_box_object =
#if __cpp_lib_ranges >= 202207L
    std::move_constructible<Tp>
#else
    std::copy_constructible<Tp>
#endif
    && std::is_object_v<Tp>;

// Primary template - uses std::optional and introduces an empty state in case assignment fails.
template <movable_box_object Tp>
class movable_box {
    [[no_unique_address]] std::optional<Tp> val_;

  public:
    template <class... Args>
        requires std::is_constructible_v<Tp, Args...>
    constexpr explicit movable_box(std::in_place_t,
                                   Args&&... args) noexcept(std::is_nothrow_constructible_v<Tp, Args...>)
        : val_(std::in_place, std::forward<Args>(args)...) {}

    constexpr movable_box() noexcept(std::is_nothrow_default_constructible_v<Tp>)
        requires std::default_initializable<Tp>
        : val_(std::in_place) {}

    movable_box(const movable_box&) = default;
    movable_box(movable_box&&)      = default;

    constexpr movable_box& operator=(const movable_box& other) noexcept(std::is_nothrow_copy_constructible_v<Tp>)
#if __cpp_lib_ranges >= 202207L
        requires std::copy_constructible<Tp>
#endif
    {
        if (this != std::addressof(other)) {
            if (other.has_value())
                val_.emplace(*other);
            else
                val_.reset();
        }
        return *this;
    }

    movable_box& operator=(movable_box&&)
        requires std::movable<Tp>
    = default;

    constexpr movable_box& operator=(movable_box&& other) noexcept(std::is_nothrow_move_constructible_v<Tp>) {
        if (this != std::addressof(other)) {
            if (other.has_value())
                val_.emplace(std::move(*other));
            else
                val_.reset();
        }
        return *this;
    }

    constexpr const Tp& operator*() const noexcept { return *val_; }
    constexpr Tp&       operator*() noexcept { return *val_; }

    constexpr const Tp* operator->() const noexcept { return val_.operator->(); }
    constexpr Tp*       operator->() noexcept { return val_.operator->(); }

    [[nodiscard]] constexpr bool has_value() const noexcept { return val_.has_value(); }
};

template <bool Const, class Tp>
using maybe_const = std::conditional_t<Const, const Tp, Tp>;

template <class Op, class Indices, class... BoundArgs>
struct perfect_forward_impl;

template <class Op, size_t... Idx, class... BoundArgs>
struct perfect_forward_impl<Op, std::index_sequence<Idx...>, BoundArgs...> {
  private:
    std::tuple<BoundArgs...> bound_args_;

  public:
    template <class... Args, class = std::enable_if_t<std::is_constructible_v<std::tuple<BoundArgs...>, Args&&...>>>
    explicit constexpr perfect_forward_impl(Args&&... bound_args) : bound_args_(std::forward<Args>(bound_args)...) {}

    perfect_forward_impl(const perfect_forward_impl&) = default;
    perfect_forward_impl(perfect_forward_impl&&)      = default;

    perfect_forward_impl& operator=(const perfect_forward_impl&) = default;
    perfect_forward_impl& operator=(perfect_forward_impl&&)      = default;

    template <class... Args, class = std::enable_if_t<std::is_invocable_v<Op, BoundArgs&..., Args...>>>
    constexpr auto
    operator()(Args&&... args) & noexcept(noexcept(Op()(std::get<Idx>(bound_args_)..., std::forward<Args>(args)...)))
        -> decltype(Op()(std::get<Idx>(bound_args_)..., std::forward<Args>(args)...)) {
        return Op()(std::get<Idx>(bound_args_)..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<!std::is_invocable_v<Op, BoundArgs&..., Args...>>>
    auto operator()(Args&&...) & = delete;

    template <class... Args, class = std::enable_if_t<std::is_invocable_v<Op, const BoundArgs&..., Args...>>>
    constexpr auto operator()(Args&&... args) const& noexcept(noexcept(Op()(std::get<Idx>(bound_args_)...,
                                                                            std::forward<Args>(args)...)))
        -> decltype(Op()(std::get<Idx>(bound_args_)..., std::forward<Args>(args)...)) {
        return Op()(std::get<Idx>(bound_args_)..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<!std::is_invocable_v<Op, const BoundArgs&..., Args...>>>
    auto operator()(Args&&...) const& = delete;

    template <class... Args, class = std::enable_if_t<std::is_invocable_v<Op, BoundArgs..., Args...>>>
    constexpr auto operator()(Args&&... args) && noexcept(noexcept(Op()(std::get<Idx>(std::move(bound_args_))...,
                                                                        std::forward<Args>(args)...)))
        -> decltype(Op()(std::get<Idx>(std::move(bound_args_))..., std::forward<Args>(args)...)) {
        return Op()(std::get<Idx>(std::move(bound_args_))..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<!std::is_invocable_v<Op, BoundArgs..., Args...>>>
    auto operator()(Args&&...) && = delete;

    template <class... Args, class = std::enable_if_t<std::is_invocable_v<Op, const BoundArgs..., Args...>>>
    constexpr auto operator()(Args&&... args) const&& noexcept(noexcept(Op()(std::get<Idx>(std::move(bound_args_))...,
                                                                             std::forward<Args>(args)...)))
        -> decltype(Op()(std::get<Idx>(std::move(bound_args_))..., std::forward<Args>(args)...)) {
        return Op()(std::get<Idx>(std::move(bound_args_))..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<!std::is_invocable_v<Op, const BoundArgs..., Args...>>>
    auto operator()(Args&&...) const&& = delete;
};

// perfect_forward implements a perfect-forwarding call wrapper as explained in [func.require].
template <class Op, class... Args>
using perfect_forward = perfect_forward_impl<Op, std::index_sequence_for<Args...>, Args...>;

struct compose_op {
    template <class Fn1, class Fn2, class... Args>
    constexpr auto operator()(Fn1&& f1, Fn2&& f2, Args&&... args) const
        noexcept(noexcept(std::invoke(std::forward<Fn1>(f1),
                                      std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...))))
            -> decltype(std::invoke(std::forward<Fn1>(f1),
                                    std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...))) {
        return std::invoke(std::forward<Fn1>(f1), std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...));
    }
};

template <class Fn1, class Fn2>
struct compose_t : perfect_forward<compose_op, Fn1, Fn2> {
    using perfect_forward<compose_op, Fn1, Fn2>::perfect_forward;
};

template <class Fn1, class Fn2>
constexpr auto compose(Fn1&& f1, Fn2&& f2) noexcept(
    noexcept(compose_t<std::decay_t<Fn1>, std::decay_t<Fn2>>(std::forward<Fn1>(f1), std::forward<Fn2>(f2))))
    -> decltype(compose_t<std::decay_t<Fn1>, std::decay_t<Fn2>>(std::forward<Fn1>(f1), std::forward<Fn2>(f2))) {
    return compose_t<std::decay_t<Fn1>, std::decay_t<Fn2>>(std::forward<Fn1>(f1), std::forward<Fn2>(f2));
}

// CRTP base that one can derive from in order to be considered a range adaptor closure
// by the library. When deriving from this class, a pipe operator will be provided to
// make the following hold:
// - `x | f` is equivalent to `f(x)`
// - `f1 | f2` is an adaptor closure `g` such that `g(x)` is equivalent to `f2(f1(x))`
template <class Tp>
struct range_adaptor_closure;

// Type that wraps an arbitrary function object and makes it into a range adaptor closure,
// i.e. something that can be called via the `x | f` notation.
template <class Fn>
struct range_adaptor_closure_t : Fn, range_adaptor_closure<range_adaptor_closure_t<Fn>> {
    constexpr explicit range_adaptor_closure_t(Fn&& f) : Fn(std::move(f)) {}
};

template <class Tp>
concept RangeAdaptorClosure =
    std::derived_from<std::remove_cvref_t<Tp>, range_adaptor_closure<std::remove_cvref_t<Tp>>>;

template <class Tp>
struct range_adaptor_closure {
    template <std::ranges::viewable_range View, RangeAdaptorClosure Closure>
        requires std::same_as<Tp, std::remove_cvref_t<Closure>> && std::invocable<Closure, View>
    [[nodiscard]] friend constexpr decltype(auto)
    operator|(View&& view, Closure&& closure) noexcept(std::is_nothrow_invocable_v<Closure, View>) {
        return std::invoke(std::forward<Closure>(closure), std::forward<View>(view));
    }

    template <RangeAdaptorClosure Closure, RangeAdaptorClosure OtherClosure>
        requires std::same_as<Tp, std::remove_cvref_t<Closure>> &&
                 std::constructible_from<std::decay_t<Closure>, Closure> &&
                 std::constructible_from<std::decay_t<OtherClosure>, OtherClosure>
    [[nodiscard]] friend constexpr auto
    operator|(Closure&&      c1,
              OtherClosure&& c2) noexcept(std::is_nothrow_constructible_v<std::decay_t<Closure>, Closure> &&
                                          std::is_nothrow_constructible_v<std::decay_t<OtherClosure>, OtherClosure>) {
        return range_adaptor_closure_t(compose(std::forward<OtherClosure>(c2), std::forward<Closure>(c1)));
    }
};

template <size_t NBound, class = std::make_index_sequence<NBound>>
struct bind_back_op;

template <size_t NBound, size_t... Ip>
struct bind_back_op<NBound, std::index_sequence<Ip...>> {
    template <class Fn, class BoundArgs, class... Args>
    constexpr auto operator()(Fn&& f, BoundArgs&& bound_args, Args&&... args) const noexcept(noexcept(std::invoke(
        std::forward<Fn>(f), std::forward<Args>(args)..., std::get<Ip>(std::forward<BoundArgs>(bound_args))...)))
        -> decltype(std::invoke(std::forward<Fn>(f),
                                std::forward<Args>(args)...,
                                std::get<Ip>(std::forward<BoundArgs>(bound_args))...)) {
        return std::invoke(
            std::forward<Fn>(f), std::forward<Args>(args)..., std::get<Ip>(std::forward<BoundArgs>(bound_args))...);
    }
};

template <class Fn, class BoundArgs>
struct bind_back_t : perfect_forward<bind_back_op<std::tuple_size_v<BoundArgs>>, Fn, BoundArgs> {
    using perfect_forward<bind_back_op<std::tuple_size_v<BoundArgs>>, Fn, BoundArgs>::perfect_forward;
};

template <class Fn, class... Args>
    requires std::is_constructible_v<std::decay_t<Fn>, Fn> && std::is_move_constructible_v<std::decay_t<Fn>> &&
             (std::is_constructible_v<std::decay_t<Args>, Args> && ...) &&
             (std::is_move_constructible_v<std::decay_t<Args>> && ...)
constexpr auto
bind_back(Fn&& f, Args&&... args) noexcept(noexcept(bind_back_t<std::decay_t<Fn>, std::tuple<std::decay_t<Args>...>>(
    std::forward<Fn>(f), std::forward_as_tuple(std::forward<Args>(args)...))))
    -> decltype(bind_back_t<std::decay_t<Fn>, std::tuple<std::decay_t<Args>...>>(
        std::forward<Fn>(f), std::forward_as_tuple(std::forward<Args>(args)...))) {
    return bind_back_t<std::decay_t<Fn>, std::tuple<std::decay_t<Args>...>>(
        std::forward<Fn>(f), std::forward_as_tuple(std::forward<Args>(args)...));
}

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

    // Workaround for older versions of compilers
    constexpr auto __get_base_end() const { return std::ranges::end(parent_->base_); }

    friend constexpr bool operator==(const iterator& x, const iterator& y)
        requires std::equality_comparable<std::ranges::iterator_t<Base>>
    {
        return x.current_ == y.current_;
    }
    friend constexpr bool operator==(const iterator& x, std::default_sentinel_t) {
        return x.current_ == x.__get_base_end();
    }
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

#endif // BEMAN_SCAN_VIEW_SCAN_HPP
