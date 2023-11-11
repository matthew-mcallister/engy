project('engy', 'cpp')

gl = dependency('gl')
spng = dependency('spng')
sdl2 = dependency('sdl2')

executable(
    'engy',
    'src/asset.cpp',
    'src/block.cpp',
    'src/camera.cpp',
    #'src/chunk.cpp',
    'src/image.cpp',
    'src/main.cpp',
    'src/math/aabb.cpp',
    'src/math/matrix.cpp',
    'src/math/scene.cpp',
    'src/render/chunk_mesh.cpp',
    'src/vulkan/device.cpp',
    'src/vulkan/memory.cpp',
    'src/vulkan/mesh.cpp',
    'src/vulkan/renderer.cpp',
    'src/vulkan/staging.cpp',
    'src/vulkan/texture_map.cpp',
    'src/vulkan/vma.cpp',
    dependencies: [gl, spng, sdl2],
    include_directories: [include_directories('src')],
    cpp_args: ['-std=c++20', '-DGL_GLEXT_PROTOTYPES', '-msse4.1', '-Wno-narrowing'],
)
