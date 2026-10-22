//
// Created by zanpisotek on 10/23/26.
//
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>

#include <vector>

struct Object {
    glm::vec3 Position;
    glm::vec3 Rotation;
    glm::vec3 Scale;

    std::vector<VkBuffer> UniformBuffers;
    std::vector<VkDeviceMemory> UniformBuffersMemory;
    std::vector<void*> UniformBuffersMapped;

    // Descriptor set for this object - one per frame in flight
    std::vector<VkDescriptorSet> DescriptorSets;

    glm::mat4 GetModelMatrix() {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, Position);
        model = glm::rotate(model, Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, Scale);
        return model;
    }
};

