project('engy', 'cpp')

sdl2 = dependency('sdl2')
gl = dependency('gl')

executable(
    'engy',
    'src/main.cpp',
    'src/asset.cpp',
    'src/render.cpp',
    dependencies: [sdl2, gl],
    cpp_args: ['-std=c++20', '-DGL_GLEXT_PROTOTYPES'],
)
