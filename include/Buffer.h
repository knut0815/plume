#pragma once

#include <memory>
#include <vector>

#include "Platform.h"
#include "Device.h"

namespace vk
{

	class Buffer;
	using BufferRef = std::shared_ptr<Buffer>;

	class Buffer
	{

	public:

		struct Options
		{
			Options();

			Options& queueFamilyIndices(const std::vector<uint32_t> tQueueFamilyIndices) { mQueueFamilyIndices = tQueueFamilyIndices; return *this; }
			Options& bufferUsageFlags(VkBufferUsageFlags tBufferUsageFlags) { mBufferUsageFlags = tBufferUsageFlags; return *this; }
			Options& useStagingBuffer(bool tUseStagingBuffer) { mUseStagingBuffer = tUseStagingBuffer; return *this; }

			std::vector<uint32_t> mQueueFamilyIndices;
			VkBufferUsageFlags mBufferUsageFlags;
			bool mUseStagingBuffer;
		};

		//! Factory method for returning a new BufferRef.
		template<class T>
		static BufferRef create(const DeviceRef &tDevice, const std::vector<T> &tData, const Options &tOptions = Options())
		{
			return std::make_shared<Buffer>(tDevice, sizeof(T) * tData.size(), tData.data());
		}

		Buffer(const DeviceRef &tDevice, size_t tSize, const void *tData, const Options &tOptions = Options());
		~Buffer();

		inline VkBuffer getHandle() const { return mBufferHandle; }
		inline VkDeviceMemory getDeviceMemoryHandle() const { return mDeviceMemoryHandle; }

		//! Retrieve the size of the memory region that is attached to this buffer.
		inline size_t getSize() const { return mSize; }

		void* map(size_t tOffset, size_t tSize);
		void unmap();

	private:

		uint32_t findMemoryTypeIndex(uint32_t tMemoryTypeBits, VkMemoryPropertyFlags tMemoryPropertyFlags);

		//! Based on the memory requirements of this buffer, find the index of the memory heap that should be used to back this buffer.
		void allocateMemory(const VkMemoryRequirements &tMemoryRequirements);

		VkBuffer mBufferHandle;
		VkDeviceMemory mDeviceMemoryHandle;

		DeviceRef mDevice;

		size_t mSize;

	};

} // namespace vk
