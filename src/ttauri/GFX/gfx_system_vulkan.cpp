// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gfx_system_vulkan.hpp"
#include "gfx_device_vulkan.hpp"
#include "../metadata.hpp"
#include "../architecture.hpp"
#include <chrono>
#include <cstring>

namespace tt {

using namespace std;

static bool hasFoundationExtensions(const std::vector<const char *> &requiredExtensions)
{
    auto availableExtensions = std::unordered_set<std::string>();
    for (auto availableExtensionProperties : vk::enumerateInstanceExtensionProperties()) {
        availableExtensions.insert(std::string(availableExtensionProperties.extensionName.data()));
    }

    for (auto requiredExtension : requiredExtensions) {
        if (availableExtensions.count(requiredExtension) == 0) {
            return false;
        }
    }
    return true;
}

static std::vector<const char *> filter_available_layers(std::vector<const char *> const &requested_layers)
{
    auto available_layers = vk::enumerateInstanceLayerProperties();

    tt_log_info("Available vulkan layers:");
    auto r = std::vector<const char *>{};
    for (ttlet &available_layer : available_layers) {
        ttlet layer_name = std::string{available_layer.layerName.data()};

        ttlet it = std::find(std::begin(requested_layers), std::end(requested_layers), layer_name);

        if (it != std::end(requested_layers)) {
            // Use the *it, because the lifetime of its `char const *` is still available after the function call.
            r.push_back(*it);
            tt_log_info("  * {}", layer_name);
        } else {
            tt_log_info("    {}", layer_name);
        }
    }
    return r;
}

gfx_system_vulkan::gfx_system_vulkan() : gfx_system()
{
    if constexpr (operating_system::current == operating_system::windows) {
        requiredExtensions = {VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
    } else {
        tt_not_implemented();
        // XXX tt_static_not_implemented();
    }

    applicationInfo = vk::ApplicationInfo(
        metadata::application().name.c_str(),
        VK_MAKE_VERSION(
            metadata::application().version.major, metadata::application().version.minor, metadata::application().version.patch),
        metadata::library().name.c_str(),
        VK_MAKE_VERSION(metadata::library().version.major, metadata::library().version.minor, metadata::library().version.patch),
        VK_API_VERSION_1_2);

    // VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2 extension is needed to retrieve unique identifiers for
    // each GPU in the system, so that we can select the same one on each startup and so that the
    // user could select a different one.
    requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    // VK_KHR_SURFACE extension is needed to draw in a window.
    requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    if constexpr (build_type::current == build_type::debug) {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (!hasFoundationExtensions(requiredExtensions)) {
        throw gui_error("Vulkan instance does not have the required extensions");
    }

    auto instanceCreateInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &applicationInfo);
    instanceCreateInfo.setEnabledExtensionCount(narrow_cast<uint32_t>(requiredExtensions.size()));
    instanceCreateInfo.setPpEnabledExtensionNames(requiredExtensions.data());

    if constexpr (build_type::current == build_type::debug) {
        requiredFeatures.robustBufferAccess = VK_TRUE;
    }

    if constexpr (build_type::current == build_type::debug) {
        ttlet requested_layers = std::vector<char const *>{
            "VK_LAYER_KHRONOS_validation",
            //"VK_LAYER_LUNARG_api_dump"
        };

        requiredLayers = filter_available_layers(requested_layers);
    }

    instanceCreateInfo.setEnabledLayerCount(narrow_cast<uint32_t>(requiredLayers.size()));
    instanceCreateInfo.setPpEnabledLayerNames(requiredLayers.data());

    tt_log_info("Creating Vulkan instance.");
    intrinsic = vk::createInstance(instanceCreateInfo);

#if (VK_HEADER_VERSION == 97)
    _loader = vk::DispatchLoaderDynamic(intrinsic);
#else
    _loader = vk::DispatchLoaderDynamic(intrinsic, vkGetInstanceProcAddr);
#endif
}

gfx_system_vulkan::~gfx_system_vulkan()
{
    ttlet lock = std::scoped_lock(gfx_system_mutex);
    if constexpr (build_type::current == build_type::debug) {
        intrinsic.destroy(debugUtilsMessager, nullptr, loader());
    }
}

void gfx_system_vulkan::init() noexcept(false)
{
    ttlet lock = std::scoped_lock(gfx_system_mutex);

    if constexpr (build_type::current == build_type::debug) {
        debugUtilsMessager = intrinsic.createDebugUtilsMessengerEXT(
            {vk::DebugUtilsMessengerCreateFlagsEXT(),
             vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                 // vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                 vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
             vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                 vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
             debugUtilsMessageCallback,
             this},
            nullptr,
            loader());
    }

    for (auto _physicalDevice : intrinsic.enumeratePhysicalDevices()) {
        devices.push_back(std::make_shared<gfx_device_vulkan>(*this, _physicalDevice));
    }
}

VkBool32 gfx_system_vulkan::debugUtilsMessageCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    auto message = std::string_view(pCallbackData->pMessage);

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        tt_log_info("Vulkan: {}", message);

    } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        tt_log_warning("Vulkan: {}", message);

    } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        if (message.starts_with("Failed to open dynamic library")) {
            // Steelseries mouse driver will inject:
            // C:\ProgramData\obs-studio-hook\graphics-hook{32,64}.dll
            // One of them will always fail to load.
            tt_log_warning("Vulkan: {}", pCallbackData->pMessage);

        } else {
            tt_log_error("Vulkan: {}", pCallbackData->pMessage);
        }
    }

    return VK_FALSE;
}

} // namespace tt
