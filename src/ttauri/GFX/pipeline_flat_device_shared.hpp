// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../rapid/sfloat_rgba16.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>

namespace tt {
class gfx_device_vulkan;
}

namespace tt::pipeline_flat {
struct Image;

struct device_shared final {
    gfx_device_vulkan const &device;

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    device_shared(gfx_device_vulkan const &device);
    ~device_shared();

    device_shared(device_shared const &) = delete;
    device_shared &operator=(device_shared const &) = delete;
    device_shared(device_shared &&) = delete;
    device_shared &operator=(device_shared &&) = delete;

    /*! Deallocate vulkan resources.
    * This is called in the destructor of gfx_device_vulkan, therefor we can not use our `device`.
    */
    void destroy(gfx_device_vulkan *vulkanDevice);

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

private:
    void buildShaders();
    void teardownShaders(gfx_device_vulkan *vulkanDevice);
};

}
