#include "rendering/lve_model.hpp"
#include "rendering/lve_utils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjectloader.h"

#include "math.hpp"



#include <iostream>
#include <cassert>
#include <cstring>
#include <unordered_map>

namespace std {
	template<>
	struct hash<lve::LveModel::Vertex> {
		size_t operator()(lve::LveModel::Vertex const &vertex) const {
			size_t seed = 0;
			lve::hashCombine(seed, vertex.position, vertex.normal, vertex.color, vertex.uv);
			lve::hashCombine(seed, vertex.position, vertex.normal, vertex.color, vertex.uv);
			return seed;
		}
	};
}


namespace lve {
	LveModel::LveModel(LveDevice &device, const Builder& builder) : lveDevice{device} {
		createVertexBuffers(builder.vertices);
		createIndexBuffer(builder.indices);
	}
	LveModel::~LveModel() {}

	void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3u && "Vertex count must be at least 3");
		uint32_t vertexSize = sizeof(vertices[0]);
		VkDeviceSize bufferSize = vertexSize * vertexCount;

		// Create staging buffer on GPU -- because VK_..._HOST_VISIBLE_BIT not preferable, host-visible memory is slower than local device-memory
		LveBuffer stagingBuffer {
			lveDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // VK_..._HOST_COHERENT_BIT tells vulkan to flush memory in the CPU at `data` to the GPU
		};

		//* Having a buffer accessable by the CPU is slow. Create a staging buffer
		//* from CPU->GPU, and copy from GPU->GPU's local memory to speed things up

		// Map, Copy, and Unmap vertex data into staging buffer
		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());
		
		
		vertexBuffer = std::make_unique<LveBuffer>(
			lveDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT // Use local GPU memory; Super fast
		);

		lveDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize); // Flush memory from the staging buffer to the vertex buffer
	}
	void LveModel::createIndexBuffer(const std::vector<uint32_t> &indices) {
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0u;
		if (!hasIndexBuffer)
			return;

		uint32_t indexSize = sizeof(indices[0]);
		VkDeviceSize bufferSize = indexSize * indexCount;

		//* Same as `createVertexBuffers`. See there for comments

		LveBuffer stagingBuffer{
			lveDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};


		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());
		
		indexBuffer = std::make_unique<LveBuffer>(
			lveDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		lveDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize); // Flush memory from the staging buffer to the vertex buffer
	}
	void LveModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = {vertexBuffer->getBuffer()};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}
	void LveModel::draw(VkCommandBuffer commandBuffer) {
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
			return;
		}
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}
	std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
		attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
		attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
		attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex, uv)});
		
		return attributeDescriptions;
	}

	std::unique_ptr<LveModel> LveModel::createModelFromFile(LveDevice& device, const std::string filePath, glm::vec3 offset) {
		Builder builder{};
		builder.loadModel(filePath);
		std::string fileName = filePath.substr(filePath.find_last_of("/") + 1);
		std::cout << fileName << " Vertex Count: " << builder.vertices.size() << std::endl;
		for( auto& vertex : builder.vertices) { vertex.position += offset; }

		return std::make_unique<LveModel>(device, builder);
	}

	void LveModel::Builder::loadModel(const std::string& filePath) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;
		std::string err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str()) || !err.empty())
			throw std::runtime_error("Warn: \n" + warn + "\nError: \n" + err);

		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				if (index.vertex_index >= 0) {
					vertex.position = {
						attrib.vertices[3*index.vertex_index + 0],
						attrib.vertices[3*index.vertex_index + 1],
						attrib.vertices[3*index.vertex_index + 2]
					};

					vertex.color = {
						attrib.colors[3*index.vertex_index + 0],
						attrib.colors[3*index.vertex_index + 1],
						attrib.colors[3*index.vertex_index + 2]
					};
				}
				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3*index.normal_index + 0],
						attrib.normals[3*index.normal_index + 1],
						attrib.normals[3*index.normal_index + 2]
					};
				}
				if (index.texcoord_index >= 0) {
					vertex.uv = {
						attrib.texcoords[3*index.texcoord_index + 0],
						attrib.texcoords[3*index.texcoord_index + 1]
					};
				}

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}



}