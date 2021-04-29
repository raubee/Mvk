# mvk 
mvk is a library for building vulkan based samples written in C++ 20</br>

Features : 
- Loading textures
- Loading 3D model (vertex & faces)
- Depth test
- Multiple graphic pipelines support

Textures supported : (all stb_image formats) **.jpg**, **.png**, **.tga**, **.bmp**, **.psd**, **.gif**, **.hdr**, **.pic** </br>
3D models extensions supported : **.obj**

Next steps :
- **.gltf** loader
- Work on a simple brdf rendering
- Navigation in scene view with mouse and keyboard

# Libraries
Vulkan hpp c++ bindings for Vulkan : [Vulkan-hpp](https://github.com/KhronosGroup/Vulkan-Hpp)</br>
Vulkan memory allocator : [Vma](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)</br>
Vulkan memory allocator bindings for hpp and C++ 20 support : [Vma-hpp c++ 20 support](https://github.com/Cvelth/VulkanMemoryAllocator-Hpp_fork.git)</br>
Windows diplay : [GLFW](https://www.glfw.org/)</br>
Maths : [GLM](https://glm.g-truc.net/0.9.9/index.html)</br>
Image loading : [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h)</br>
Model loading : [tiny_obj_loader.h](https://github.com/tinyobjloader/tinyobjloader)</br>

## Simple Viewer (SimpleViewer)

<img src="/captures/simpleviewer.png" style="display:block; margin:auto"/>

## Obj Viewer (ObjViewer)

<img src="/captures/objviewer.png" style="display:block; margin:auto"/>

## Multi Viewer (MultiViewer)

<img src="/captures/multiviewer.png" style="display:block; margin:auto"/>
