/*
*
* MIT License
*
* Copyright(c) 2017 Michael Walczyk
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files(the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions :
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

#include "Buffer.h"

namespace plume
{

	namespace graphics
	{

		Buffer::Buffer(const Device& device, vk::BufferUsageFlags buffer_usage_flags, size_t size, const void* data, const std::vector<QueueType> queues) :

			m_device_ptr(&device),
			m_buffer_usage_flags(buffer_usage_flags),
			m_requested_size(size)
		{
			vk::SharingMode sharing_mode = vk::SharingMode::eExclusive;
			if (queues.size() > 1)
			{
				PL_LOG_DEBUG("This buffer is used by multiple queue families: setting its share mode to vk::SharingMode::eConcurrent\n");
				sharing_mode = vk::SharingMode::eConcurrent;
			}

			// Gather all of the queue family indices based on the requested queue types.
			std::vector<uint32_t> queue_family_indices(queues.size());
			std::transform(queues.begin(), queues.end(), queue_family_indices.begin(), [&](QueueType type) { return m_device_ptr->get_queue_family_index(type); });

			vk::BufferCreateInfo buffer_create_info;
			buffer_create_info.pQueueFamilyIndices = queue_family_indices.data();	// Ignored if the sharing mode is exclusive.
			buffer_create_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size());
			buffer_create_info.sharingMode = sharing_mode;
			buffer_create_info.size = m_requested_size;
			buffer_create_info.usage = m_buffer_usage_flags;

			m_buffer_handle = m_device_ptr->get_handle().createBufferUnique(buffer_create_info);

			// Store the memory requirements for this buffer object.
			m_memory_requirements = m_device_ptr->get_handle().getBufferMemoryRequirements(m_buffer_handle.get());

			// Allocate device memory.
			// TODO: these flags should be parameters. Currently, all buffer objects are marked as CPU-accessible.
			m_device_memory = std::make_unique<DeviceMemory>(device, m_memory_requirements, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			// Fill the buffer with the data that was passed into the constructor.
			if (data)
			{
				void* mapped_ptr = m_device_memory->map(0, m_device_memory->get_allocation_size());
				memcpy(mapped_ptr, data, static_cast<size_t>(m_requested_size));
				m_device_memory->unmap();
			}

			// Associate the device memory with this buffer object.
			m_device_ptr->get_handle().bindBufferMemory(m_buffer_handle.get(), m_device_memory->get_handle(), 0);
		}

		vk::DescriptorBufferInfo Buffer::build_descriptor_info(vk::DeviceSize offset, vk::DeviceSize range) const
		{
			if (range != VK_WHOLE_SIZE &&
				range < 0 ||
				range <= m_device_memory->get_allocation_size() - offset)
			{
				throw std::runtime_error("Invalid value for `range` parameter of `build_descriptor_info()`");
			}

			return{ m_buffer_handle.get(), offset, range };
		}

	} // namespace graphics

} // namespace plume