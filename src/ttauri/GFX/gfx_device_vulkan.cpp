// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gfx_device_vulkan.hpp"
#include "gfx_system_vulkan.hpp"
#include "gfx_surface_vulkan.hpp"
#include "pipeline_image.hpp"
#include "pipeline_image_device_shared.hpp"
#include "../GUI/gui_window.hpp"
#include "../resource_view.hpp"
#include <span>

namespace tt {

#define QUEUE_CAPABILITY_GRAPHICS 1
#define QUEUE_CAPABILITY_COMPUTE 2
#define QUEUE_CAPABILITY_PRESENT 4
#define QUEUE_CAPABILITY_GRAPHICS_AND_PRESENT (QUEUE_CAPABILITY_GRAPHICS | QUEUE_CAPABILITY_PRESENT)
#define QUEUE_CAPABILITY_ALL (QUEUE_CAPABILITY_GRAPHICS | QUEUE_CAPABILITY_COMPUTE | QUEUE_CAPABILITY_PRESENT)

static bool hasRequiredExtensions(const vk::PhysicalDevice &physicalDevice, const std::vector<const char *> &requiredExtensions)
{
    auto availableExtensions = std::unordered_set<std::string>();
    for (auto availableExtensionProperties : physicalDevice.enumerateDeviceExtensionProperties()) {
        availableExtensions.insert(std::string(availableExtensionProperties.extensionName.data()));
    }

    for (auto requiredExtension : requiredExtensions) {
        if (availableExtensions.count(requiredExtension) == 0) {
            return false;
        }
    }
    return true;
}

static bool meetsRequiredLimits(const vk::PhysicalDevice &physicalDevice, const vk::PhysicalDeviceLimits &requiredLimits)
{
    return true;
}

static bool hasRequiredFeatures(const vk::PhysicalDevice &physicalDevice, const vk::PhysicalDeviceFeatures &requiredFeatures)
{
    ttlet availableFeatures = physicalDevice.getFeatures();
    auto meetsRequirements = true;

    meetsRequirements &=
        (requiredFeatures.robustBufferAccess == VK_TRUE) ? (availableFeatures.robustBufferAccess == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.fullDrawIndexUint32 == VK_TRUE) ? (availableFeatures.fullDrawIndexUint32 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.imageCubeArray == VK_TRUE) ? (availableFeatures.imageCubeArray == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.independentBlend == VK_TRUE) ? (availableFeatures.independentBlend == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.geometryShader == VK_TRUE) ? (availableFeatures.geometryShader == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.tessellationShader == VK_TRUE) ? (availableFeatures.tessellationShader == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.sampleRateShading == VK_TRUE) ? (availableFeatures.sampleRateShading == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.dualSrcBlend == VK_TRUE) ? (availableFeatures.dualSrcBlend == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.logicOp == VK_TRUE) ? (availableFeatures.logicOp == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.multiDrawIndirect == VK_TRUE) ? (availableFeatures.multiDrawIndirect == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.drawIndirectFirstInstance == VK_TRUE) ? (availableFeatures.drawIndirectFirstInstance == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.depthClamp == VK_TRUE) ? (availableFeatures.depthClamp == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.depthBiasClamp == VK_TRUE) ? (availableFeatures.depthBiasClamp == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.fillModeNonSolid == VK_TRUE) ? (availableFeatures.fillModeNonSolid == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.depthBounds == VK_TRUE) ? (availableFeatures.depthBounds == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.wideLines == VK_TRUE) ? (availableFeatures.wideLines == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.largePoints == VK_TRUE) ? (availableFeatures.largePoints == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.alphaToOne == VK_TRUE) ? (availableFeatures.alphaToOne == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.multiViewport == VK_TRUE) ? (availableFeatures.multiViewport == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.samplerAnisotropy == VK_TRUE) ? (availableFeatures.samplerAnisotropy == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.textureCompressionETC2 == VK_TRUE) ? (availableFeatures.textureCompressionETC2 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.textureCompressionASTC_LDR == VK_TRUE) ?
        (availableFeatures.textureCompressionASTC_LDR == VK_TRUE) :
        true;
    meetsRequirements &=
        (requiredFeatures.textureCompressionBC == VK_TRUE) ? (availableFeatures.textureCompressionBC == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.occlusionQueryPrecise == VK_TRUE) ? (availableFeatures.occlusionQueryPrecise == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.pipelineStatisticsQuery == VK_TRUE) ? (availableFeatures.pipelineStatisticsQuery == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.vertexPipelineStoresAndAtomics == VK_TRUE) ?
        (availableFeatures.vertexPipelineStoresAndAtomics == VK_TRUE) :
        true;
    meetsRequirements &=
        (requiredFeatures.fragmentStoresAndAtomics == VK_TRUE) ? (availableFeatures.fragmentStoresAndAtomics == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderTessellationAndGeometryPointSize == VK_TRUE) ?
        (availableFeatures.shaderTessellationAndGeometryPointSize == VK_TRUE) :
        true;
    meetsRequirements &=
        (requiredFeatures.shaderImageGatherExtended == VK_TRUE) ? (availableFeatures.shaderImageGatherExtended == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageExtendedFormats == VK_TRUE) ?
        (availableFeatures.shaderStorageImageExtendedFormats == VK_TRUE) :
        true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageMultisample == VK_TRUE) ?
        (availableFeatures.shaderStorageImageMultisample == VK_TRUE) :
        true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageReadWithoutFormat == VK_TRUE) ?
        (availableFeatures.shaderStorageImageReadWithoutFormat == VK_TRUE) :
        true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageWriteWithoutFormat == VK_TRUE) ?
        (availableFeatures.shaderStorageImageWriteWithoutFormat == VK_TRUE) :
        true;
    meetsRequirements &= (requiredFeatures.shaderUniformBufferArrayDynamicIndexing == VK_TRUE) ?
        (availableFeatures.shaderUniformBufferArrayDynamicIndexing == VK_TRUE) :
        true;
    meetsRequirements &= (requiredFeatures.shaderSampledImageArrayDynamicIndexing == VK_TRUE) ?
        (availableFeatures.shaderSampledImageArrayDynamicIndexing == VK_TRUE) :
        true;
    meetsRequirements &= (requiredFeatures.shaderStorageBufferArrayDynamicIndexing == VK_TRUE) ?
        (availableFeatures.shaderStorageBufferArrayDynamicIndexing == VK_TRUE) :
        true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageArrayDynamicIndexing == VK_TRUE) ?
        (availableFeatures.shaderStorageImageArrayDynamicIndexing == VK_TRUE) :
        true;
    meetsRequirements &=
        (requiredFeatures.shaderClipDistance == VK_TRUE) ? (availableFeatures.shaderClipDistance == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.shaderCullDistance == VK_TRUE) ? (availableFeatures.shaderCullDistance == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderFloat64 == VK_TRUE) ? (availableFeatures.shaderFloat64 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderInt64 == VK_TRUE) ? (availableFeatures.shaderInt64 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderInt16 == VK_TRUE) ? (availableFeatures.shaderInt16 == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.shaderResourceResidency == VK_TRUE) ? (availableFeatures.shaderResourceResidency == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.shaderResourceMinLod == VK_TRUE) ? (availableFeatures.shaderResourceMinLod == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseBinding == VK_TRUE) ? (availableFeatures.sparseBinding == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.sparseResidencyBuffer == VK_TRUE) ? (availableFeatures.sparseResidencyBuffer == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.sparseResidencyImage2D == VK_TRUE) ? (availableFeatures.sparseResidencyImage2D == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.sparseResidencyImage3D == VK_TRUE) ? (availableFeatures.sparseResidencyImage3D == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.sparseResidency2Samples == VK_TRUE) ? (availableFeatures.sparseResidency2Samples == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.sparseResidency4Samples == VK_TRUE) ? (availableFeatures.sparseResidency4Samples == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.sparseResidency8Samples == VK_TRUE) ? (availableFeatures.sparseResidency8Samples == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.sparseResidency16Samples == VK_TRUE) ? (availableFeatures.sparseResidency16Samples == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.sparseResidencyAliased == VK_TRUE) ? (availableFeatures.sparseResidencyAliased == VK_TRUE) : true;
    meetsRequirements &=
        (requiredFeatures.variableMultisampleRate == VK_TRUE) ? (availableFeatures.variableMultisampleRate == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.inheritedQueries == VK_TRUE) ? (availableFeatures.inheritedQueries == VK_TRUE) : true;

    return meetsRequirements;
}

gfx_device_vulkan::gfx_device_vulkan(gfx_system &system, vk::PhysicalDevice physicalDevice) :
    gfx_device(system), physicalIntrinsic(std::move(physicalDevice))
{
    auto result = physicalIntrinsic.getProperties2KHR<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceIDProperties>(
        narrow_cast<gfx_system_vulkan &>(system).loader());

    auto resultDeviceProperties2 = result.get<vk::PhysicalDeviceProperties2>();
    auto resultDeviceIDProperties = result.get<vk::PhysicalDeviceIDProperties>();

    requiredExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);

    deviceID = resultDeviceProperties2.properties.deviceID;
    vendorID = resultDeviceProperties2.properties.vendorID;
    deviceName = std::string(resultDeviceProperties2.properties.deviceName.data());
    deviceUUID = uuid::from_big_endian(resultDeviceIDProperties.deviceUUID);

    physicalProperties = physicalIntrinsic.getProperties();

    initialize_device();
}

gfx_device_vulkan::~gfx_device_vulkan()
{
    try {
        ttlet lock = std::scoped_lock(gfx_system_mutex);

        toneMapperPipeline->destroy(this);
        toneMapperPipeline = nullptr;
        SDFPipeline->destroy(this);
        SDFPipeline = nullptr;
        imagePipeline->destroy(this);
        imagePipeline = nullptr;
        boxPipeline->destroy(this);
        boxPipeline = nullptr;
        flatPipeline->destroy(this);
        flatPipeline = nullptr;

        destroy_quad_index_buffer();

        vmaDestroyAllocator(allocator);

        for (ttlet &queue : _queues) {
            intrinsic.destroy(queue.command_pool);
        }

        intrinsic.destroy();

    } catch (std::exception const &e) {
        tt_log_fatal("Could not properly destruct gfx_device_vulkan. '{}'", e.what());
    }
}

/** Get a graphics queue.
 */
[[nodiscard]] gfx_queue_vulkan const &gfx_device_vulkan::get_graphics_queue() const noexcept
{
    for (auto &queue : _queues) {
        if (queue.flags & vk::QueueFlagBits::eGraphics) {
            return queue;
        }
    }
    tt_no_default();
}

[[nodiscard]] gfx_queue_vulkan const &gfx_device_vulkan::get_graphics_queue(gfx_surface const &surface) const noexcept
{
    ttlet &surface_ = narrow_cast<gfx_surface_vulkan const &>(surface).intrinsic;

    // First try to find a graphics queue which can also present.
    gfx_queue_vulkan const *graphics_queue = nullptr;
    for (auto &queue : _queues) {
        if (queue.flags & vk::QueueFlagBits::eGraphics) {
            if (physicalIntrinsic.getSurfaceSupportKHR(queue.family_queue_index, surface_)) {
                return queue;
            }
            if (not graphics_queue) {
                graphics_queue = &queue;
            }
        }
    }

    tt_axiom(graphics_queue);
    return *graphics_queue;
}

[[nodiscard]] gfx_queue_vulkan const &gfx_device_vulkan::get_present_queue(gfx_surface const &surface) const noexcept
{
    ttlet &surface_ = narrow_cast<gfx_surface_vulkan const &>(surface).intrinsic;

    // First try to find a graphics queue which can also present.
    gfx_queue_vulkan const *present_queue = nullptr;
    for (auto &queue : _queues) {
        if (physicalIntrinsic.getSurfaceSupportKHR(queue.family_queue_index, surface_)) {
            if (queue.flags & vk::QueueFlagBits::eGraphics) {
                return queue;
            }
            if (not present_queue) {
                present_queue = &queue;
            }
        }
    }

    tt_axiom(present_queue);
    return *present_queue;
}

[[nodiscard]] vk::SurfaceFormatKHR
gfx_device_vulkan::get_surface_format(gfx_surface const &surface, int *score) const noexcept
{
    ttlet &surface_ = narrow_cast<gfx_surface_vulkan const &>(surface).intrinsic;

    auto best_surface_format = vk::SurfaceFormatKHR{};
    auto best_surface_format_score = 0;
    for (auto surface_format : physicalIntrinsic.getSurfaceFormatsKHR(surface_)) {
        auto surface_format_score = 0;

        switch (surface_format.colorSpace) {
        case vk::ColorSpaceKHR::eSrgbNonlinear: surface_format_score += 1; break;
        case vk::ColorSpaceKHR::eExtendedSrgbNonlinearEXT: surface_format_score += 10; break;
        default:;
        }

        switch (surface_format.format) {
        case vk::Format::eR16G16B16A16Sfloat: surface_format_score += 12; break;
        case vk::Format::eR16G16B16Sfloat: surface_format_score += 11; break;
        case vk::Format::eA2B10G10R10UnormPack32:
            // This is a wire format for HDR, the GPU will not automatically convert linear shader-space to this wire format.
            surface_format_score -= 100; break;
        case vk::Format::eR8G8B8A8Srgb: surface_format_score += 4; break;
        case vk::Format::eB8G8R8A8Srgb: surface_format_score += 4; break;
        case vk::Format::eR8G8B8Srgb: surface_format_score += 3; break;
        case vk::Format::eB8G8R8Srgb: surface_format_score += 3; break;
        case vk::Format::eB8G8R8A8Unorm: surface_format_score += 2; break;
        case vk::Format::eR8G8B8A8Unorm: surface_format_score += 2; break;
        case vk::Format::eB8G8R8Unorm: surface_format_score += 1; break;
        case vk::Format::eR8G8B8Unorm: surface_format_score += 1; break;
        default:;
        }

        if (score) {
            tt_log_info(
                "    - color-space={}, format={}, score={}",
                vk::to_string(surface_format.colorSpace),
                vk::to_string(surface_format.format),
                surface_format_score);
        }

        if (surface_format_score > best_surface_format_score) {
            best_surface_format_score = surface_format_score;
            best_surface_format = surface_format;
        }
    }

    if (score) {
        *score = best_surface_format_score;
    }
    return best_surface_format;
}

[[nodiscard]] vk::PresentModeKHR
gfx_device_vulkan::get_present_mode(gfx_surface const &surface, int *score) const noexcept
{
    ttlet &surface_ = narrow_cast<gfx_surface_vulkan const &>(surface).intrinsic;

    auto best_present_mode = vk::PresentModeKHR{};
    auto best_present_mode_score = 0;
    for (ttlet &present_mode : physicalIntrinsic.getSurfacePresentModesKHR(surface_)) {
        int present_mode_score = 0;

        switch (present_mode) {
        case vk::PresentModeKHR::eImmediate: present_mode_score += 1; break;
        case vk::PresentModeKHR::eFifoRelaxed: present_mode_score += 2; break;
        case vk::PresentModeKHR::eFifo: present_mode_score += 3; break;
        case vk::PresentModeKHR::eMailbox: present_mode_score += 1; break; // mailbox does not wait for vsync.
        default: continue;
        }

        if (score) {
            tt_log_info("    - present-mode={} score={}", vk::to_string(present_mode), present_mode_score);
        }

        if (present_mode_score > best_present_mode_score) {
            best_present_mode_score = present_mode_score;
            best_present_mode = present_mode;
        }
    }

    if (score) {
        *score = best_present_mode_score;
    }
    return best_present_mode;
}

int gfx_device_vulkan::score(gfx_surface const &surface) const
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());
    ttlet &surface_ = narrow_cast<gfx_surface_vulkan const &>(surface).intrinsic;

    auto total_score = 0;

    tt_log_info("Scoring device: {}", string());
    if (!hasRequiredFeatures(physicalIntrinsic, narrow_cast<gfx_system_vulkan &>(system).requiredFeatures)) {
        tt_log_info(" - Does not have the required features.");
        return -1;
    }

    if (!meetsRequiredLimits(physicalIntrinsic, narrow_cast<gfx_system_vulkan &>(system).requiredLimits)) {
        tt_log_info(" - Does not meet the required limits.");
        return -1;
    }

    if (!hasRequiredExtensions(physicalIntrinsic, requiredExtensions)) {
        tt_log_info(" - Does not have the required extensions.");
        return -1;
    }
    
    bool device_has_graphics = false;
    bool device_has_present = false;
    bool device_has_compute = false;
    bool device_shares_graphics_and_present = false;
    for (ttlet &queue: _queues) {
        ttlet has_present = static_cast<bool>(physicalIntrinsic.getSurfaceSupportKHR(queue.family_queue_index, surface_));
        ttlet has_graphics = static_cast<bool>(queue.flags & vk::QueueFlagBits::eGraphics);
        ttlet has_compute = static_cast<bool>(queue.flags & vk::QueueFlagBits::eCompute);
        
        device_has_graphics |= has_graphics;
        device_has_present |= has_present;
        device_has_compute |= has_compute;
        if (has_present and has_graphics) {
            device_shares_graphics_and_present = true;
        }
    }

    if (not device_has_graphics) {
        tt_log_info(" - Does not have a graphics queue.");
        return -1;
    }

    if (not device_has_present) {
        tt_log_info(" - Does not have a present queue.");
        return -1;
    }

    if (device_has_compute) {
        tt_log_info(" - Device has compute queue.");
        total_score += 1;
    }

    if (device_shares_graphics_and_present) {
        tt_log_info(" - Device shares graphics and present on same queue.");
        total_score += 10;
    }

    tt_log_info(" - Surface formats:");
    int surface_format_score = 0;
    [[maybe_unused]] auto surface_format = get_surface_format(surface, &surface_format_score);
    if (surface_format_score <= 0) {
        tt_log_info(" - Does not have a suitable surface format.");
        return -1;
    }
    total_score += surface_format_score;

    tt_log_info(" - Present modes:");
    int present_mode_score = 0;
    [[maybe_unused]] auto present_mode = get_present_mode(surface, &present_mode_score);
    if (present_mode_score <= 0) {
        tt_log_info(" - Does not have a suitable present mode.");
        return -1;
    }
    total_score += present_mode_score;

    // Give score based on the performance of the device.
    auto device_type_score = 0;
    ttlet properties = physicalIntrinsic.getProperties();
    switch (properties.deviceType) {
    case vk::PhysicalDeviceType::eCpu: device_type_score = 1; break;
    case vk::PhysicalDeviceType::eOther: device_type_score = 1; break;
    case vk::PhysicalDeviceType::eVirtualGpu: device_type_score = 2; break;
    case vk::PhysicalDeviceType::eIntegratedGpu: device_type_score = 3; break;
    case vk::PhysicalDeviceType::eDiscreteGpu: device_type_score = 4; break;
    }
    tt_log_info(" - device-type={}, score={}", vk::to_string(properties.deviceType), device_type_score);
    total_score += device_type_score;

    tt_log_info(" - total score {}", total_score);
    return total_score;
}

std::vector<vk::DeviceQueueCreateInfo> gfx_device_vulkan::make_device_queue_create_infos() const noexcept
{
    ttlet default_queue_priority = std::array{1.0f};
    uint32_t queue_family_index = 0;

    auto r = std::vector<vk::DeviceQueueCreateInfo>{};
    for (auto queue_family_properties : physicalIntrinsic.getQueueFamilyProperties()) {
        ttlet num_queues = 1;
        tt_axiom(std::size(default_queue_priority) >= num_queues);
        r.emplace_back(vk::DeviceQueueCreateFlags(), queue_family_index++, num_queues, default_queue_priority.data());
    }
    return r;
}

void gfx_device_vulkan::initialize_queues(std::vector<vk::DeviceQueueCreateInfo> const &device_queue_create_infos) noexcept
{
    ttlet queue_family_properties = physicalIntrinsic.getQueueFamilyProperties();

    for (ttlet &device_queue_create_info : device_queue_create_infos) {
        ttlet queue_family_index = device_queue_create_info.queueFamilyIndex;
        ttlet &queue_family_property = queue_family_properties[queue_family_index];
        ttlet queue_flags = queue_family_property.queueFlags;

        for (uint32_t queue_index = 0; queue_index != device_queue_create_info.queueCount; ++queue_index) {
            ttlet queue = intrinsic.getQueue(queue_family_index, queue_index);
            ttlet command_pool = intrinsic.createCommandPool(
                {vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                 queue_family_index});

            _queues.emplace_back(queue_family_index, queue_index, queue_flags, std::move(queue), std::move(command_pool));
        }
    }
}

void gfx_device_vulkan::initialize_device()
{
    ttlet device_queue_create_infos = make_device_queue_create_infos();

    intrinsic = physicalIntrinsic.createDevice(
        {vk::DeviceCreateFlags(),
         narrow_cast<uint32_t>(device_queue_create_infos.size()),
         device_queue_create_infos.data(),
         0,
         nullptr,
         narrow_cast<uint32_t>(requiredExtensions.size()),
         requiredExtensions.data(),
         &(narrow_cast<gfx_system_vulkan &>(system).requiredFeatures)});

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.physicalDevice = physicalIntrinsic;
    allocatorCreateInfo.device = intrinsic;
    allocatorCreateInfo.instance = narrow_cast<gfx_system_vulkan &>(system).intrinsic;
    vmaCreateAllocator(&allocatorCreateInfo, &allocator);

    VmaAllocationCreateInfo lazyAllocationInfo = {};
    lazyAllocationInfo.usage = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
    uint32_t typeIndexOut = 0;
    supportsLazyTransientImages =
        vmaFindMemoryTypeIndex(allocator, 0, &lazyAllocationInfo, &typeIndexOut) != VK_ERROR_FEATURE_NOT_PRESENT;

    if (supportsLazyTransientImages) {
        lazyMemoryUsage = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
        transientImageUsageFlags = vk::ImageUsageFlagBits::eTransientAttachment;
    }

    initialize_queues(device_queue_create_infos);
    initialize_quad_index_buffer();

    flatPipeline = std::make_unique<pipeline_flat::device_shared>(*this);
    boxPipeline = std::make_unique<pipeline_box::device_shared>(*this);
    imagePipeline = std::make_unique<pipeline_image::device_shared>(*this);
    SDFPipeline = std::make_unique<pipeline_SDF::device_shared>(*this);
    toneMapperPipeline = std::make_unique<pipeline_tone_mapper::device_shared>(*this);
}

void gfx_device_vulkan::initialize_quad_index_buffer()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    using vertex_index_type = uint16_t;
    constexpr ssize_t maximum_number_of_vertices = 1 << (sizeof(vertex_index_type) * CHAR_BIT);
    constexpr ssize_t maximum_number_of_quads = maximum_number_of_vertices / 4;
    constexpr ssize_t maximum_number_of_triangles = maximum_number_of_quads * 2;
    constexpr ssize_t maximum_number_of_indices = maximum_number_of_triangles * 3;

    // Create vertex index buffer
    {
        vk::BufferCreateInfo const bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof(vertex_index_type) * maximum_number_of_indices,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::SharingMode::eExclusive};
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        std::tie(quadIndexBuffer, quadIndexBufferAllocation) = createBuffer(bufferCreateInfo, allocationCreateInfo);
    }

    // Fill in the vertex index buffer, using a staging buffer, then copying.
    {
        // Create staging vertex index buffer.
        vk::BufferCreateInfo const bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof(vertex_index_type) * maximum_number_of_indices,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferSrc,
            vk::SharingMode::eExclusive};
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        ttlet[stagingvertexIndexBuffer, stagingvertexIndexBufferAllocation] =
            createBuffer(bufferCreateInfo, allocationCreateInfo);

        // Initialize indices.
        ttlet stagingvertexIndexBufferData = mapMemory<vertex_index_type>(stagingvertexIndexBufferAllocation);
        for (size_t i = 0; i < maximum_number_of_indices; i++) {
            ttlet vertexInRectangle = i % 6;
            ttlet rectangleNr = i / 6;
            ttlet rectangleBase = rectangleNr * 4;

            switch (vertexInRectangle) {
            case 0: stagingvertexIndexBufferData[i] = narrow_cast<vertex_index_type>(rectangleBase + 0); break;
            case 1: stagingvertexIndexBufferData[i] = narrow_cast<vertex_index_type>(rectangleBase + 1); break;
            case 2: stagingvertexIndexBufferData[i] = narrow_cast<vertex_index_type>(rectangleBase + 2); break;
            case 3: stagingvertexIndexBufferData[i] = narrow_cast<vertex_index_type>(rectangleBase + 2); break;
            case 4: stagingvertexIndexBufferData[i] = narrow_cast<vertex_index_type>(rectangleBase + 1); break;
            case 5: stagingvertexIndexBufferData[i] = narrow_cast<vertex_index_type>(rectangleBase + 3); break;
            default: tt_no_default();
            }
        }
        flushAllocation(stagingvertexIndexBufferAllocation, 0, VK_WHOLE_SIZE);
        unmapMemory(stagingvertexIndexBufferAllocation);

        // Copy indices to vertex index buffer.
        auto &queue = get_graphics_queue();
        auto commands = allocateCommandBuffers({queue.command_pool, vk::CommandBufferLevel::ePrimary, 1}).at(0);
        commands.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        commands.copyBuffer(
            stagingvertexIndexBuffer, quadIndexBuffer, {{0, 0, sizeof(vertex_index_type) * maximum_number_of_indices}});
        commands.end();

        std::vector<vk::CommandBuffer> const commandBuffersToSubmit = {commands};
        std::vector<vk::SubmitInfo> const submitInfo = {
            {0,
             nullptr,
             nullptr,
             narrow_cast<uint32_t>(commandBuffersToSubmit.size()),
             commandBuffersToSubmit.data(),
             0,
             nullptr}};
        queue.queue.submit(submitInfo, vk::Fence());
        queue.queue.waitIdle();

        freeCommandBuffers(queue.command_pool, {commands});
        destroyBuffer(stagingvertexIndexBuffer, stagingvertexIndexBufferAllocation);
    }
}

void gfx_device_vulkan::destroy_quad_index_buffer()
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());
    destroyBuffer(quadIndexBuffer, quadIndexBufferAllocation);
}

std::pair<vk::Buffer, VmaAllocation> gfx_device_vulkan::createBuffer(
    const vk::BufferCreateInfo &bufferCreateInfo,
    const VmaAllocationCreateInfo &allocationCreateInfo) const
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    VkBuffer buffer;
    VmaAllocation allocation;

    ttlet bufferCreateInfo_ = static_cast<VkBufferCreateInfo>(bufferCreateInfo);
    ttlet result = static_cast<vk::Result>(
        vmaCreateBuffer(allocator, &bufferCreateInfo_, &allocationCreateInfo, &buffer, &allocation, nullptr));

    std::pair<vk::Buffer, VmaAllocation> const value = {buffer, allocation};

    return vk::createResultValue(result, value, "tt::gfx_device_vulkan::createBuffer");
}

void gfx_device_vulkan::destroyBuffer(const vk::Buffer &buffer, const VmaAllocation &allocation) const
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    vmaDestroyBuffer(allocator, buffer, allocation);
}

std::pair<vk::Image, VmaAllocation> gfx_device_vulkan::createImage(
    const vk::ImageCreateInfo &imageCreateInfo,
    const VmaAllocationCreateInfo &allocationCreateInfo) const
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    VkImage image;
    VmaAllocation allocation;

    ttlet imageCreateInfo_ = static_cast<VkImageCreateInfo>(imageCreateInfo);
    ttlet result = static_cast<vk::Result>(
        vmaCreateImage(allocator, &imageCreateInfo_, &allocationCreateInfo, &image, &allocation, nullptr));

    std::pair<vk::Image, VmaAllocation> const value = {image, allocation};

    return vk::createResultValue(result, value, "tt::gfx_device_vulkan::createImage");
}

void gfx_device_vulkan::destroyImage(const vk::Image &image, const VmaAllocation &allocation) const
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    vmaDestroyImage(allocator, image, allocation);
}

void gfx_device_vulkan::unmapMemory(const VmaAllocation &allocation) const
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    vmaUnmapMemory(allocator, allocation);
}

vk::CommandBuffer gfx_device_vulkan::beginSingleTimeCommands() const
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    ttlet &queue = get_graphics_queue();
    ttlet commandBuffers = intrinsic.allocateCommandBuffers({queue.command_pool, vk::CommandBufferLevel::ePrimary, 1});
    ttlet commandBuffer = commandBuffers.at(0);

    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    return commandBuffer;
}

void gfx_device_vulkan::endSingleTimeCommands(vk::CommandBuffer commandBuffer) const
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    commandBuffer.end();

    std::vector<vk::CommandBuffer> const commandBuffers = {commandBuffer};

    ttlet &queue = get_graphics_queue();
    queue.queue.submit(
        {{
            0,
            nullptr,
            nullptr, // wait semaphores, wait stages
            narrow_cast<uint32_t>(commandBuffers.size()),
            commandBuffers.data(),
            0,
            nullptr // signal semaphores
        }},
        vk::Fence());

    queue.queue.waitIdle();
    intrinsic.freeCommandBuffers(queue.command_pool, commandBuffers);
}

static std::pair<vk::AccessFlags, vk::PipelineStageFlags> access_and_stage_from_layout(vk::ImageLayout layout) noexcept
{
    switch (layout) {
    case vk::ImageLayout::eUndefined: return {vk::AccessFlags(), vk::PipelineStageFlagBits::eTopOfPipe};

    // GPU Texture Maps
    case vk::ImageLayout::eTransferDstOptimal: return {vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTransfer};

    case vk::ImageLayout::eShaderReadOnlyOptimal:
        return {vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eFragmentShader};

    // CPU Staging texture maps
    case vk::ImageLayout::eGeneral: return {vk::AccessFlagBits::eHostWrite, vk::PipelineStageFlagBits::eHost};

    case vk::ImageLayout::eTransferSrcOptimal: return {vk::AccessFlagBits::eTransferRead, vk::PipelineStageFlagBits::eTransfer};

    // If we are explicitly transferring an image for ePresentSrcKHR, then we are doing this
    // because we want to reuse the swapchain images in subsequent rendering. Make sure it
    // is ready for the fragment shader.
    case vk::ImageLayout::ePresentSrcKHR:
        return {
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
            vk::PipelineStageFlagBits::eColorAttachmentOutput};

    default: tt_no_default();
    }
}

void gfx_device_vulkan::transition_layout(
    vk::CommandBuffer command_buffer,
    vk::Image image,
    vk::Format format,
    vk::ImageLayout srcLayout,
    vk::ImageLayout dstLayout)
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    ttlet[srcAccessMask, srcStage] = access_and_stage_from_layout(srcLayout);
    ttlet[dstAccessMask, dstStage] = access_and_stage_from_layout(dstLayout);

    std::vector<vk::ImageMemoryBarrier> barriers = {
        {srcAccessMask,
         dstAccessMask,
         srcLayout,
         dstLayout,
         VK_QUEUE_FAMILY_IGNORED,
         VK_QUEUE_FAMILY_IGNORED,
         image,
         {
             vk::ImageAspectFlagBits::eColor,
             0, // baseMipLevel
             1, // levelCount
             0, // baseArrayLayer
             1 // layerCount
         }}};

    command_buffer.pipelineBarrier(
        srcStage,
        dstStage,
        vk::DependencyFlags(),
        0,
        nullptr,
        0,
        nullptr,
        narrow_cast<uint32_t>(barriers.size()),
        barriers.data());
}

void gfx_device_vulkan::transition_layout(
    vk::Image image,
    vk::Format format,
    vk::ImageLayout src_layout,
    vk::ImageLayout dst_layout) const
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    ttlet command_buffer = beginSingleTimeCommands();

    transition_layout(command_buffer, image, format, src_layout, dst_layout);

    endSingleTimeCommands(command_buffer);
}

void gfx_device_vulkan::copyImage(
    vk::Image srcImage,
    vk::ImageLayout srcLayout,
    vk::Image dstImage,
    vk::ImageLayout dstLayout,
    vk::ArrayProxy<vk::ImageCopy const> regions) const
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    ttlet commandBuffer = beginSingleTimeCommands();

    commandBuffer.copyImage(srcImage, srcLayout, dstImage, dstLayout, regions);

    endSingleTimeCommands(commandBuffer);
}

void gfx_device_vulkan::clearColorImage(
    vk::Image image,
    vk::ImageLayout layout,
    vk::ClearColorValue const &color,
    vk::ArrayProxy<const vk::ImageSubresourceRange> ranges) const
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    ttlet commandBuffer = beginSingleTimeCommands();

    commandBuffer.clearColorImage(image, layout, color, ranges);

    endSingleTimeCommands(commandBuffer);
}

vk::ShaderModule gfx_device_vulkan::loadShader(uint32_t const *data, size_t size) const
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    tt_log_info("Loading shader");

    // Check uint32_t alignment of pointer.
    tt_axiom((reinterpret_cast<std::uintptr_t>(data) & 3) == 0);

    return intrinsic.createShaderModule({vk::ShaderModuleCreateFlags(), size, data});
}

vk::ShaderModule gfx_device_vulkan::loadShader(std::span<std::byte const> shaderObjectBytes) const
{
    // no lock, only local variable.

    // Make sure the address is aligned to uint32_t;
    ttlet address = reinterpret_cast<uintptr_t>(shaderObjectBytes.data());
    tt_assert((address & 2) == 0);

    ttlet shaderObjectBytes32 = reinterpret_cast<uint32_t const *>(shaderObjectBytes.data());
    return loadShader(shaderObjectBytes32, shaderObjectBytes.size());
}

vk::ShaderModule gfx_device_vulkan::loadShader(URL const &shaderObjectLocation) const
{
    // no lock, only local variable.

    return loadShader(*shaderObjectLocation.loadView());
}

} // namespace tt
