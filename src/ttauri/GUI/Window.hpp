// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/os_detect.hpp"

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "TTauri/GUI/Window_vulkan_win32.hpp"
namespace tt {
using Window = Window_vulkan_win32;
}

#elif  TT_OPERATING_SYSTEM == TT_OS_MACOS
#include "TTauri/GUI/Window_vulkan_macos.hpp"
namespace tt {
using Window = Window_vulkan_macos;
}

#else
#error "Window not implemented for this os."
#endif
