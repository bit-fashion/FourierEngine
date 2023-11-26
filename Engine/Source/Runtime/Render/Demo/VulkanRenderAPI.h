/* ************************************************************************
 *
 * Copyright (C) 2022 Vincent Luo All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ************************************************************************/

/* Creates on 2023/11/21. */
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <Nanoriv.h>
#include <unordered_map>
#include <optional>
#include <array>
#include <glm.hpp>
#include <stddef.h>
#include <memory>

class NRIVwindow;

/**
 * Physical device struct.
 */
struct PhysicalDeviceProperties {
    VkPhysicalDevice handle;
    char deviceName[255];
};

/**
 * Queue family indices struct.
 */
struct QueueFamilyIndices {
    uint32_t graphicsFamily = 0;
    uint32_t presentFamily = 0;
    bool isComplete() const {
        return graphicsFamily > 0 && presentFamily > 0;
    }
};

/**
 * SwapChain supports details struct.
 */
struct FourierSwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static VkVertexInputBindingDescription GetVertexInputBindingDescription() {
        VkVertexInputBindingDescription vertexInputBindingDescription = {};
        vertexInputBindingDescription.binding = 0;
        vertexInputBindingDescription.stride = sizeof(Vertex);
        vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return vertexInputBindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> GetVertexInputAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributeDescriptions = {};

        vertexInputAttributeDescriptions[0].binding = 0;
        vertexInputAttributeDescriptions[0].location = 0;
        vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        vertexInputAttributeDescriptions[0].offset = offsetof(Vertex, position);

        vertexInputAttributeDescriptions[1].binding = 0;
        vertexInputAttributeDescriptions[1].location = 1;
        vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        vertexInputAttributeDescriptions[1].offset = offsetof(Vertex, color);

        return vertexInputAttributeDescriptions;
    }

};

const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

/**
 * Render API Interface.
 */
class VulkanRenderAPI {
public:
    /* Init vulkan render api. */
    explicit VulkanRenderAPI(NRIVwindow *pRIVwindow);
    ~VulkanRenderAPI();

public:
    void BeginRender();
    void Draw();
    void EndRender();

private:
    void BindVertexBuffer();
    void BindIndexBuffer();
    void BindTexture();
    void BindUniformBuffer();
    void UpdateUniformBuffer();

private:
    struct UniformBufferObject ubo;
    /* Handle object. */
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
    VkPipelineLayout m_GraphicsPipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
    VkRenderPass m_RenderPass;
    VkPresentModeKHR m_SurfacePresentModeKHR;
    VkSurfaceFormatKHR m_SurfaceFormatKHR;
    QueueFamilyIndices m_QueueFamilyIndices;
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    VkDescriptorSetLayout m_DescriptorSetLayout;
    /* memory */
    VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;
    VkBuffer m_UniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_UniformBufferMemory = VK_NULL_HANDLE;
    VkMemoryRequirements m_MemoryRequirements;
    /* semaphore */
    VkSemaphore m_ImageAvailableSemaphore;
    VkSemaphore m_RenderFinishedSemaphore;
    /* Queue */
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;
    /* SwapChain */
    uint32_t m_SwapChainImageSize;
    VkFormat m_SwapChainFormat;
    VkExtent2D m_SwapChainExtent;
    std::vector<VkImage> m_SwapChainImages;
    std::vector<VkImageView> m_SwapChainImageViews;
    std::vector<VkFramebuffer> m_SwapChainFramebuffers;
    /* Vectors. */
    std::vector<const char *> m_RequiredInstanceExtensions;
    std::vector<const char *> m_RequiredInstanceLayers;
    std::vector<const char *> m_RequiredDeviceExtensions;
    std::vector<PhysicalDeviceProperties> m_PhysicalDevices;
    /* Map */
    std::unordered_map<std::string, VkExtensionProperties> m_VkInstanceExtensionPropertiesSupports;
    std::unordered_map<std::string, VkLayerProperties> m_VkInstanceLayerPropertiesSupports;
    std::unordered_map<std::string, VkExtensionProperties> m_VkDeviceExtensionPropertiesSupports;
};

