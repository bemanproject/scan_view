module;

#include <algorithm>
#include <concepts>
#include <functional>
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>

export module beman.scan_view;

#define BEMAN_SCAN_VIEW_INCLUDED_FROM_INTERFACE_UNIT
export {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <beman/scan_view/scan_view.hpp>
#pragma clang diagnostic pop
}
