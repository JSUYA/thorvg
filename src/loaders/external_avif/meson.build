source_file = [
   'tvgAvifLoader.h',
   'tvgAvifLoader.cpp',
]

avif_dep = dependency('libavif', required: true)

if avif_dep.found()
   subloader_dep += [declare_dependency(
      include_directories : include_directories('.'),
      dependencies : avif_dep,
      sources : source_file
      )]
endif
