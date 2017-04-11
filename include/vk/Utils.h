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

#include "Platform.h"

namespace graphics
{

	namespace utils
	{

		//! Determine whether or not an image format contains a depth component.
		inline bool is_depth_format(vk::Format format)
		{
			return (format == vk::Format::eD16Unorm ||
				format == vk::Format::eD16UnormS8Uint ||
				format == vk::Format::eD24UnormS8Uint ||
				format == vk::Format::eD32Sfloat ||
				format == vk::Format::eD32SfloatS8Uint);
		}

		//! Determine whether or not an image format contains a stencil component.
		inline bool is_stencil_format(vk::Format format)
		{
			return (format == vk::Format::eD16UnormS8Uint ||
				format == vk::Format::eD24UnormS8Uint ||
				format == vk::Format::eD32SfloatS8Uint);
		}

		//! Translate an image format into the appropriate aspect mask flags.
		inline vk::ImageAspectFlags format_to_aspect_mask(vk::Format format)
		{
			vk::ImageAspectFlags image_aspect_flags;

			if (is_depth_format(format))
			{
				image_aspect_flags = vk::ImageAspectFlagBits::eDepth;
				if (is_stencil_format(format))
				{
					image_aspect_flags |= vk::ImageAspectFlagBits::eStencil;
				}
			}
			else
			{
				image_aspect_flags = vk::ImageAspectFlagBits::eColor;
			}

			return image_aspect_flags;
		}
	
	} // namespace utils

} // namespace graphics