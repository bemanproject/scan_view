module;

#include <version>

export module beman.scan_view;

import std;

#define BEMAN_SCAN_VIEW_INCLUDED_FROM_INTERFACE_UNIT
export {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <beman/scan_view/scan.hpp>
#pragma clang diagnostic pop
}
