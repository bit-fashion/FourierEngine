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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, e1ither express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ************************************************************************/

/* Creates on 2022/9/14. */

/*
  ===================================
    @author bit-fashion
  ===================================
*/
#include "Window/Window.h"
#include "Render/Drivers/Vulkan/VulkanContext.h"
#include <System.h>
#include "Editor/GameEditor.h"
// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Debug.h>
#include <engcnf.h>

struct UniformBufferObject {
    glm::mat4 m;
    glm::mat4 v;
    glm::mat4 p;
    float     t;
};

int main(int argc, const char **argv) {
    Window window("SportsEngine", 120, 90);
    std::unique_ptr<VulkanContext> vulkanContext = std::make_unique<VulkanContext>(&window);
    window.SetWindowHintVisible(true);

    VkRenderPipeline pipeline;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
    VkDeviceBuffer vertexBuffer;
    VkDeviceBuffer indexBuffer;
    VkTexture2D texture2D;
    VkDeviceBuffer uniformBuffer;

    std::vector<VkDescriptorSetLayoutBinding> uboDescriptorSetLayoutBinding = {
            { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, VK_NULL_HANDLE },
            { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE },
    };
    vulkanContext->CreateDescriptorSetLayout(uboDescriptorSetLayoutBinding, 0, &descriptorSetLayout);

    Vector<VkDescriptorSetLayout> layouts = { descriptorSetLayout };
    vulkanContext->AllocateDescriptorSet(layouts, &descriptorSet);
    vulkanContext->CreateRenderPipeline("../Engine/Binaries", "simple_shader", descriptorSetLayout, &pipeline);

    const std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f,  -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f,  0.5f,  0.0f},  {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f,  0.0f},  {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };

    const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
    };

    vulkanContext->AllocateVertexBuffer(ARRAY_SIZE(vertices), std::data(vertices), &vertexBuffer);
    vulkanContext->AllocateIndexBuffer(ARRAY_SIZE(indices), std::data(indices), &indexBuffer);
    /* 创建 Texture */
    vulkanContext->CreateTexture2D("../Engine/Resource/VulkanHomePage.png", VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture2D);
    /* 创建 Uniform buffer */
    vulkanContext->AllocateBuffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffer);

    GameEditor editor(&window, vulkanContext.get());

    VkFrameContext *frameContext;
    while (!window.is_close()) {
        vulkanContext->BeginRender(&frameContext);
        {
            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;
            UniformBufferObject ubo = {};
            ubo.m = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(1.0f, 0.5f, 2.0f));
            ubo.v = glm::lookAt(glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.p = glm::perspective(glm::radians(45.0f), window.GetWidth() / (float) window.GetHeight(), 0.1f, 10.0f);
            ubo.p[1][1] *= -1;
            ubo.t = (float) glfwGetTime();
            void* data;
            vulkanContext->MapMemory(uniformBuffer, 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vulkanContext->UnmapMemory(uniformBuffer);

            vulkanContext->BindRenderPipeline(pipeline);
            vulkanContext->BindDescriptorSets(pipeline, 1, &descriptorSet);
            vulkanContext->WriteDescriptorSet(uniformBuffer, texture2D, descriptorSet);

            /* bind vertex buffer */
            VkBuffer buffers[] = {vertexBuffer.buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(frameContext->commandBuffer, 0, 1, buffers, offsets);
            vkCmdBindIndexBuffer(frameContext->commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
            /* draw call */
            vulkanContext->DrawIndexed(std::size(indices));

            editor.BeginGameEditorFrame();
            editor.EndGameEditorFrame();
        }
        vulkanContext->EndRender();
        Window::PollEvents();
    }

    vulkanContext->FreeBuffer(uniformBuffer);
    vulkanContext->DestroyTexture2D(texture2D);
    vulkanContext->FreeBuffer(indexBuffer);
    vulkanContext->FreeBuffer(vertexBuffer);
    vulkanContext->FreeDescriptorSets(1, &descriptorSet);
    vulkanContext->DestroyDescriptorSetLayout(descriptorSetLayout);
    vulkanContext->DestroyRenderPipeline(pipeline);
}
