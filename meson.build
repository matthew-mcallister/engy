project('engy', 'cpp')

sdl2 = dependency('sdl2')
gl = dependency('gl')

executable(
    'engy',
    'src/asset.cpp',
    'src/camera.cpp',
    'src/chunk.cpp',
    'src/main.cpp',
    'src/render.cpp',
    'src/math/aabb.cpp',
    'src/math/matrix.cpp',
    'src/math/scene.cpp',
    dependencies: [sdl2, gl],
    include_directories: [include_directories('src')],
    cpp_args: ['-std=c++20', '-DGL_GLEXT_PROTOTYPES', '-msse4.1', '-Wno-narrowing'],
)
