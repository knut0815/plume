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

#pragma once

#include "Device.h"

namespace plume
{

	namespace graphics
	{

		//! Device memory is memory that is visible to the device. The memory properties of a physical device 
		//! describe the memory heaps and memory types available. Each heap describes a memory resource of
		//! a particular size, and each memory type describes a set of memory properties (e.g. host cached vs.
		//! uncached) that can be used with a given memory heap.
		//!
		//! Before freeing any object bound to device memory, the application must ensure that the memory 
		//! object is no longer in use by the device - for example, by command buffers queued for execution. 
		//!
		//! Memory objects created with the vk::MemoryPropertyFlagBits::eHostVisible bit are considered 
		//! mappable.
		class DeviceMemory
		{
		public:

			DeviceMemory() = default; 

			//! Construct a stack allocated, non-copyable container that manages a device memory allocation.
			DeviceMemory(const Device& device, const vk::MemoryRequirements& memory_requirements, vk::MemoryPropertyFlags required_memory_properties);

			~DeviceMemory();

			vk::DeviceMemory get_handle() const { return m_device_memory_handle.get(); }

			//! Returns the allocation size of the memory object.
			vk::DeviceSize get_allocation_size() const { return m_memory_requirements.size; }

			//! Returns the numeric index of the memory heap from which the memory object was allocated.
			uint32_t get_selected_memory_index() const { return m_selected_memory_index; }

			//! Retrieve a host virtual address pointer to a region of this memory allocation. Note that this
			//! function does not check whether any previously submitted commands that accessed the memory
			//! region have completed. While a range of device memory is mapped for host access, the application
			//! is responsible for synchronizing both device and host access to that memory range. If the memory 
			//! is not coherent, a flush command must be used to guarantee that host writes are visible to the
			//! device.
			//!
			//! Here, `offset` refers to the zero-based byte offset from the beginning of the memory object, while
			//! `size` refers to the size of the memory range to map. By, default, `size` is set to the special
			//! value VK_WHOLE_SIZE, which will map from `offset` to the end of the memory allocation.
			void* map(vk::DeviceSize offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE);

			//! Unmaps the memory object.
			void unmap();

			//! Returns `true` if the memory object is currently in use (i.e. mapped).
			bool is_in_use() const { return m_in_use; }

			//! Returns `true` if the memory object was created with the vk::MemoryPropertyFlagBits::eHostVisible flag set.
			bool is_host_visible() { return static_cast<bool>(m_memory_property_flags & vk::MemoryPropertyFlagBits::eHostVisible); }

			//! Returns `true` if the memory object was created with the vk::MemoryPropertyFlagBits::eHostCoherent flag set.
			//! If `true`, then the underlying memory object does not require an explicit flush after unmapping.
			bool is_host_coherent() { return static_cast<bool>(m_memory_property_flags & vk::MemoryPropertyFlagBits::eHostCoherent); }

			//! Returns `true` if the memory object was created with the vk::MemoryPropertyFlagBits::eDeviceLocal flag set.
			//! If `true`, then the underlying memory object cannot be mapped.
			bool is_device_local() { return static_cast<bool>(m_memory_property_flags & vk::MemoryPropertyFlagBits::eDeviceLocal); }

		private:

			//! Based on the memory requirements, find the index of the memory heap that should be used to allocate memory.
			void find_memory_index();

			const Device* m_device_ptr;
			vk::UniqueDeviceMemory m_device_memory_handle;

			vk::MemoryRequirements m_memory_requirements;
			vk::MemoryPropertyFlags m_memory_property_flags;
			uint32_t m_selected_memory_index;
			bool m_in_use;
		};

	} // namespace graphics

} // namespace plume