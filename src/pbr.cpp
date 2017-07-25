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

#include <chrono>

#include "Vk.h"
#include "Geometry.h"

#include "glm/glm/gtc/matrix_transform.hpp"

struct UniformBufferData
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

float get_elapsed_seconds()
{
	static auto start = std::chrono::high_resolution_clock::now();
	auto current = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current - start).count() / 1000.0f;
	return elapsed;
}

static const uint32_t width = 800;
static const uint32_t height = 800;
static const auto msaa = vk::SampleCountFlagBits::e8;

int main()
{
	/// vk::Instance
	///
	///
	auto instance = graphics::Instance::create();
	auto physical_devices = instance->get_physical_devices();

	/// vk::Window and vk::Surface
	///
	///
	auto window = graphics::Window::create(instance, width, height);
	auto surface = window->create_surface();

	/// vk::Device
	///
	///
	auto device_options = graphics::Device::Options().required_queue_flags(vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer);
	auto device = graphics::Device::create(physical_devices[0], device_options);
	auto queue_family_properties = device->get_physical_device_queue_family_properties();
	for (size_t i = 0; i < queue_family_properties.size(); ++i)
	{
		vk::Bool32 support = device->get_physical_device_handle().getSurfaceSupportKHR(static_cast<uint32_t>(i), surface->get_handle());
		if (support) { /* TODO: move this check into the device class */ }
	}
	std::cout << device << std::endl;

	/// vk::Swapchain
	///
	///
	auto swapchain = graphics::Swapchain::create(device, surface, width, height);
	auto swapchain_image_views = swapchain->get_image_view_handles();

	/// vk::RenderPass
	///
	///
	auto ms_attachment = graphics::RenderPass::create_multisample_attachment(vk::Format::eB8G8R8A8Unorm, 0, msaa);
	auto resolve_attachment = graphics::RenderPass::create_color_attachment(vk::Format::eB8G8R8A8Unorm, 1);
	auto depth_attachment = graphics::RenderPass::create_depth_stencil_attachment(vk::Format::eD32SfloatS8Uint, 2, msaa);
	
	vk::SubpassDescription subpass_description = {};
	subpass_description.colorAttachmentCount = 1;
	subpass_description.pColorAttachments = &ms_attachment.second;
	subpass_description.pDepthStencilAttachment = &depth_attachment.second;
	subpass_description.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass_description.pResolveAttachments = &resolve_attachment.second;

	auto render_pass_options = graphics::RenderPass::Options()
		.attachment_descriptions({ ms_attachment.first, resolve_attachment.first, depth_attachment.first })
		.attachment_references({ ms_attachment.second, resolve_attachment.second, depth_attachment.second })
		.subpass_descriptions({ subpass_description })
		.subpass_dependencies({ graphics::RenderPass::create_default_subpass_dependency() });

	auto render_pass = graphics::RenderPass::create(device, render_pass_options);

	/// geo::Geometry
	///
	///
	auto geometry = geo::Sphere();

	/// vk::Buffer
	///
	///
	auto vbo_0 = graphics::Buffer::create(device, vk::BufferUsageFlagBits::eVertexBuffer, geometry.get_positions());
	auto vbo_1 = graphics::Buffer::create(device, vk::BufferUsageFlagBits::eVertexBuffer, geometry.get_colors());
	auto vbo_2 = graphics::Buffer::create(device, vk::BufferUsageFlagBits::eVertexBuffer, geometry.get_normals());
	auto ibo = graphics::Buffer::create(device, vk::BufferUsageFlagBits::eIndexBuffer, geometry.get_indices());
	auto ubo = graphics::Buffer::create(device, vk::BufferUsageFlagBits::eUniformBuffer, sizeof(UniformBufferData), nullptr);
	
	UniformBufferData ubo_data = {
		glm::mat4(),
		glm::lookAt({ 0.0f, 0.0, 40.0f }, { 0.0f, 0.0, 0.0f }, glm::vec3(0.0f, 1.0f, 0.0f)),
		glm::perspective(45.0f, static_cast<float>(width / height), 0.1f, 1000.0f) 
	};

	void *data = ubo->get_device_memory()->map(0, ubo->get_device_memory()->get_allocation_size());
	memcpy(data, &ubo_data, sizeof(ubo_data));
	ubo->get_device_memory()->unmap();

	const size_t grid_x = 15;
	const size_t grid_y = 15;
	const float grid_size = 16.0f;
	std::vector<glm::vec3> instance_positions;
	instance_positions.resize(grid_x * grid_y);
	for (size_t x = 0; x < grid_x; ++x)	
	{
		float fx = static_cast<float>(x + 0.5f) / grid_x;
		fx = (fx * 2.0f - 1.0f) * grid_size;

		for (size_t y = 0; y < grid_y; ++y)
		{
			float fy = static_cast<float>(y + 0.5f) / grid_y;
			fy = (fy * 2.0f - 1.0f) * grid_size;

			instance_positions[x + grid_x * y] = glm::vec3(fx, fy, 0.0f);
		}
	}

	auto vbo_instance = graphics::Buffer::create(device, vk::BufferUsageFlagBits::eVertexBuffer, instance_positions);

	/// vk::Pipeline
	///
	///
	vk::VertexInputBindingDescription binding_0 = { 0, sizeof(float) * 3 }; // input rate vertex: 3 floats between each vertex
	vk::VertexInputBindingDescription binding_1 = { 1, sizeof(float) * 3 }; // input rate vertex: 3 floats between each vertex
	vk::VertexInputBindingDescription binding_2 = { 2, sizeof(float) * 3 }; // input rate vertex: 3 floats between each vertex
	vk::VertexInputBindingDescription binding_instance = { 3, sizeof(float) * 3, vk::VertexInputRate::eInstance }; // input rate instance: 3 floats between each instance
	vk::VertexInputAttributeDescription attr_0 = { 0, binding_0.binding, vk::Format::eR32G32B32Sfloat }; // 3 floats: position
	vk::VertexInputAttributeDescription attr_1 = { 1, binding_1.binding, vk::Format::eR32G32B32Sfloat }; // 3 floats: color
	vk::VertexInputAttributeDescription attr_2 = { 2, binding_2.binding, vk::Format::eR32G32B32Sfloat }; // 3 floats: color
	vk::VertexInputAttributeDescription attr_instance = { 3, binding_instance.binding, vk::Format::eR32G32B32Sfloat }; // 3 floats: instance position offset
	
	auto v_shader = graphics::ShaderModule::create(device, ResourceManager::load_file("../assets/shaders/vert.spv"));
	auto f_shader = graphics::ShaderModule::create(device, ResourceManager::load_file("../assets/shaders/frag.spv"));

	auto pipeline_options = graphics::Pipeline::Options()
		.vertex_input_binding_descriptions({ binding_0, binding_1, binding_2, binding_instance })
		.vertex_input_attribute_descriptions({ attr_0, attr_1, attr_2, attr_instance })
		.viewports({ window->get_fullscreen_viewport() })
		.scissors({ window->get_fullscreen_scissor_rect2d() })
		.attach_shader_stages({ v_shader, f_shader })
		.primitive_topology(geometry.get_topology())
		.depth_test()
		.samples(msaa)
		.min_sample_shading(0.25f);
	auto pipeline = graphics::Pipeline::create(device, render_pass, pipeline_options);
	std::cout << pipeline << std::endl;

	/// vk::CommandPool
	///
	///
	auto command_pool = graphics::CommandPool::create(device, device->get_queue_families_mapping().graphics().second, vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	/// vk::Image
	///
	///
	auto ms_options = graphics::Image::Options()
		.image_tiling(vk::ImageTiling::eOptimal)
		.sample_count(msaa);
	auto image_ms = graphics::Image::create(device,
		vk::ImageType::e2D,
		vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
		swapchain->get_image_format(),
		width, height, 1,
		ms_options);
	auto image_ms_view = image_ms->build_image_view();

	auto image_from_file = graphics::Image::create(device, 
		vk::ImageType::e2D, 
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled, 
		vk::Format::eR32G32B32A32Sfloat, 
		ResourceManager::load_image_hdr("../assets/textures/techgate_diffuse.hdr"));
	auto image_from_file_view = image_from_file->build_image_view();
	auto sampler = graphics::Sampler::create(device);

	auto image_depth = graphics::Image::create(device, 
		vk::ImageType::e2D, 
		vk::ImageUsageFlagBits::eDepthStencilAttachment, 
		device->get_supported_depth_format(), 
		width, height, 1, 
		ms_options);
	auto image_depth_view = image_depth->build_image_view();

	{
		auto temp_command_buffer = graphics::CommandBuffer::create(device, command_pool);
		auto temp_command_buffer_handle = temp_command_buffer->get_handle();
		temp_command_buffer->begin();
		temp_command_buffer->transition_image_layout(image_from_file, vk::ImageLayout::ePreinitialized, vk::ImageLayout::eShaderReadOnlyOptimal);
		temp_command_buffer->transition_image_layout(image_depth, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
		temp_command_buffer->end();

		vk::SubmitInfo submit_info;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &temp_command_buffer_handle;
		device->get_queue_families_mapping().graphics().first.submit(submit_info, {});
		device->get_queue_families_mapping().graphics().first.waitIdle();
	}

	/// vk::Framebuffer
	///
	///
	std::vector<graphics::FramebufferRef> framebuffers(swapchain_image_views.size());
	for (size_t i = 0; i < swapchain_image_views.size(); ++i)
	{
		std::vector<vk::ImageView> image_views = { 
			image_ms_view,				// attachment 0: multisample color 
			swapchain_image_views[i],	// attachment 1: resolve color (swapchain)
			image_depth_view			// attachment 2: depth-stencil
		};

		framebuffers[i] = graphics::Framebuffer::create(device, render_pass, image_views, width, height);
	}

	/// vk::DescriptorPool
	///
	///
	auto descriptor_pool = graphics::DescriptorPool::create(device, { 
		{ vk::DescriptorType::eUniformBuffer, 1 }, 
		{ vk::DescriptorType::eCombinedImageSampler, 1 } 
	});

	/// vk::DescriptorSetLayout
	///
	///
	std::array<vk::DescriptorSetLayoutBinding, 2> layout_bindings = {{
		{	
			0,											// binding (see shader code)
			vk::DescriptorType::eUniformBuffer, 1,		// type and count
			vk::ShaderStageFlagBits::eAll,				// shader usage stages
			nullptr										// immutable samplers
		},
		{
			1,
			vk::DescriptorType::eCombinedImageSampler, 1,
			vk::ShaderStageFlagBits::eAll,
			nullptr
		}
	}};

	vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info;
	descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(layout_bindings.size());
	descriptor_set_layout_create_info.pBindings = layout_bindings.data();

	vk::DescriptorSetLayout descriptor_set_layout = device->get_handle().createDescriptorSetLayout(descriptor_set_layout_create_info);

	/// vk::DescriptorSet
	///
	///
	vk::DescriptorSetAllocateInfo descriptor_set_allocate_info = { 
		descriptor_pool->get_handle(),	// descriptor pool
		1,								// number of sets to allocate
		&descriptor_set_layout			// descriptor set layout
	};
	vk::DescriptorSet descriptor_set = device->get_handle().allocateDescriptorSets(descriptor_set_allocate_info)[0];

	auto d_buffer_info = ubo->build_descriptor_info();														// ubo
	auto d_image_info = image_from_file->build_descriptor_info(sampler, image_from_file_view);				// sampler
	vk::WriteDescriptorSet w_descriptor_set_buffer = { descriptor_set, 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &d_buffer_info };		// ubo
	vk::WriteDescriptorSet w_descriptor_set_sampler = { descriptor_set, 1, 0, 1, vk::DescriptorType::eCombinedImageSampler, &d_image_info };		// sampler
	std::vector<vk::WriteDescriptorSet> w_descriptor_sets = { w_descriptor_set_buffer, w_descriptor_set_sampler };

	device->get_handle().updateDescriptorSets(w_descriptor_sets, {});

	/// vk::Semaphore
	///
	///
	auto image_available_sem = graphics::Semaphore::create(device);
	auto render_complete_sem = graphics::Semaphore::create(device);

	while (!window->should_close())
	{
		window->poll_events();

		// Set up data for push constants.
		float elapsed = get_elapsed_seconds();
		float metallic = glm::clamp(window->get_mouse_position().x / width, 0.001f, 1.0f);
		
		// Get the index of the next available image.
		uint32_t image_index = swapchain->acquire_next_swapchain_image(image_available_sem);

		// Set the clear values for each of this framebuffer's attachments.
		std::vector<vk::ClearValue> clear_vals(3);
		clear_vals[0].color = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f };	// multisample color attachment
		clear_vals[1].color = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f };	// resolve color attachment
		clear_vals[2].depthStencil = {1.0f, 0};									// depth-stencil attachment

		// Set up a new command buffer and record draw calls.
		auto command_buffer = graphics::CommandBuffer::create(device, command_pool);
		auto command_buffer_handle = command_buffer->get_handle();
		command_buffer->begin();
		command_buffer->begin_render_pass(render_pass, framebuffers[image_index], clear_vals);
		command_buffer->bind_pipeline(pipeline);
		command_buffer->bind_vertex_buffers({ vbo_0, vbo_1, vbo_2, vbo_instance });
		command_buffer->bind_index_buffer(ibo);
		command_buffer->update_push_constant_ranges(pipeline, "time", &elapsed);
		command_buffer->update_push_constant_ranges(pipeline, "metallic", &metallic);
		command_buffer->bind_descriptor_sets(pipeline, 0, { descriptor_set }, {});
		command_buffer->draw_indexed(static_cast<uint32_t>(geometry.get_indices().size()), instance_positions.size(), 0, 0, 0);
		command_buffer->end_render_pass();
		command_buffer->end();

		// Prepare semaphores.
		vk::Semaphore wait_sems[] = { image_available_sem->get_handle() };
		vk::Semaphore signal_sems[] = { render_complete_sem->get_handle() };
		vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		vk::SubmitInfo submit_info;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_sems;
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer_handle;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_sems;
		device->get_queue_families_mapping().graphics().first.submit(submit_info, {});

		// Submit the result back to the swapchain for presentation:  make sure to wait for rendering to finish before attempting to present.
		vk::SwapchainKHR swapchains[] = { swapchain->get_handle() };
		vk::PresentInfoKHR present_info;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_sems;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapchains;
		present_info.pImageIndices = &image_index;
		present_info.pResults = nullptr;

		// This is equivalent to submitting a fence to a queue and waiting with an infinite timeout for that fence to signal.
		device->get_queue_families_mapping().graphics().first.waitIdle();
		device->get_queue_families_mapping().presentation().first.presentKHR(present_info);
	}

	return 0;
}