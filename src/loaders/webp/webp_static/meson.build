source_file = [
   'dec/alpha.cpp',
   'dec/buffer.cpp',
   'dec/frame.cpp',   
   'dec/io.cpp',
   'dec/quant.cpp',
   'dec/tree.cpp',
   'dec/vp8.cpp',
   'dec/vp8l.cpp',
   'dec/webp.cpp',

   'utils/bit_reader.cpp',
   'utils/color_cache.cpp',
   'utils/filters.cpp',
   'utils/huffman.cpp',
   'utils/quant_levels_dec.cpp',
   'utils/random.cpp',
   'utils/rescaler.cpp',
   'utils/thread.cpp',
   'utils/utils.cpp',

   'dsp/alpha_processing.cpp',   
   'dsp/argb.cpp',
   'dsp/cost.cpp',
   'dsp/cpu.cpp',
   'dsp/dec.cpp',
   'dsp/dec_clip_tables.cpp',
   'dsp/filters.cpp',
   'dsp/lossless.cpp',
   'dsp/rescaler.cpp',
   'dsp/upsampling.cpp',
   'dsp/yuv.cpp',
]

webp_static_dep += [declare_dependency(
    include_directories : include_directories('.'),
    sources : source_file
    )]
