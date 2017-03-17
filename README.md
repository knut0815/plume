# Spectrum

![Logo](https://github.com/mwalczyk/VulkanToolkit/blob/master/logo.png)

A work-in-progress framework and abstraction layer around the Vulkan graphics API. Currently being developed with version 1.0.39.1.

## Cloning

Spectrum uses several submodules (GLM, SPIRV-Cross, STB, etc.). After cloning the main
repository, make sure to initialize submodules by executing the following command from the top-level
directory:

`git submodule update --init --recursive`

More information on working with submodules can be found [here](https://github.com/blog/2104-working-with-submodules).

## Inspiration

Spectrum's syntax, structure, and design patterns were greatly influenced by several major
open source projects:

- [Cinder](https://github.com/cinder/Cinder)
- [Nyorain's VPP Toolkit](https://github.com/nyorain/vpp)
- [Dynamic Static's Vulkan Toolkit](https://github.com/DynamicStatic/Dynamic_Static_Graphics)
- [Alexander Overvoorde's Vulkan Tutorial](https://vulkan-tutorial.com/)
- [gan74's Yave Engine](https://github.com/gan74/Yave)
- [Jian Ru's Laugh Engine](https://github.com/jian-ru/laugh_engine)

Other resources that were helpful throughout the development of Spectrum:

- [Nvidia Vulkan Renderer Example](https://github.com/nvpro-samples/gl_vk_chopper)

See the [Spectrum C++ style guide](https://github.com/mwalczyk/spectrum_cpp_style) for more information.

## Roadmap:

- [ ] Add support for headless rendering and multiple windows
- [ ] Add a "notes" section to each header file, explaining each Vulkan object and its usage
- [ ] Implement a standard material model for physically based shading with IBL
- [ ] Implement forward / deferred rendering modes
