// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Window_base.hpp"
#include "PipelineImage.hpp"
#include <vulkan/vulkan.hpp>

namespace TTauri::GUI {

class Window_vulkan : public Window_base {
public:
    vk::SurfaceKHR intrinsic;

    vk::SwapchainCreateInfoKHR swapchainCreateInfo;

    vk::SwapchainKHR swapchain;

    std::optional<uint32_t> acquiredImageIndex;
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::ImageView> swapchainImageViews;
    std::vector<vk::Framebuffer> swapchainFramebuffers;

    vk::RenderPass firstRenderPass;
    vk::RenderPass followUpRenderPass;

    vk::Semaphore imageAvailableSemaphore;
    vk::Fence renderFinishedFence;

    std::shared_ptr<PipelineImage::PipelineImage> imagePipeline;

    Window_vulkan(const std::shared_ptr<WindowDelegate> delegate, const std::string title, vk::SurfaceKHR surface);
    ~Window_vulkan();

    Window_vulkan(const Window_vulkan &) = delete;
    Window_vulkan &operator=(const Window_vulkan &) = delete;
    Window_vulkan(Window_vulkan &&) = delete;
    Window_vulkan &operator=(Window_vulkan &&) = delete;

    void initialize() override;

    State buildForDeviceChange() override;
    void teardownForDeviceChange() override;
    State rebuildForSwapchainChange() override;

protected:
    void render() override;

private:
    void buildSemaphores();
    void teardownSemaphores();
    std::pair<vk::SwapchainKHR, Window_base::State> buildSwapchain(vk::SwapchainKHR oldSwapchain = {});
    void teardownSwapchain();
    void buildRenderPasses();
    void teardownRenderPasses();
    void buildFramebuffers();
    void teardownFramebuffers();
    void buildPipelines();
    void teardownPipelines();

    void waitIdle();
    std::tuple<uint32_t, vk::Extent2D, State> getImageCountExtentAndState();
};

}
