project(
  'random-c',
  'c',
  meson_version : '>= 1.3.0',
  version : '0.1',
  default_options : ['warning_level=3'],
)

cc = meson.get_compiler('c')

dependencies = [
  dependency('sdl3'),
  cc.find_library('m', required : false),
]

sources = [
  'sdl_allme.c',
  'src/collision_grid_2d.c',
  'src/sdl_helpers.c',
  'src/vec2.c',
]

include_dir = include_directories('src')
exe = executable(
  'random-c',
  sources,
  dependencies : dependencies,
  install : true,
  include_directories: include_dir
)

test('basic', exe)
