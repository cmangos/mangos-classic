libmpq_sources = [
    'config.h',
    'libmpq/common.c',
    'libmpq/common.h',
    'libmpq/crypt_buf.h',
    'libmpq/explode.c',
    'libmpq/explode.h',
    'libmpq/extract.c',
    'libmpq/huffman.c',
    'libmpq/huffman.h',
    'libmpq/mpq-internal.h',
    'libmpq/mpq.c',
    'libmpq/mpq.h',
    'libmpq/pack_begin.h',
    'libmpq/pack_end.h',
    'libmpq/platform.h',
    'libmpq/wave.c',
    'libmpq/wave.h'
]

libmpq_inc_dir = ['.']

if meson.get_compiler('cpp').get_id() == 'msvc'
  libmpq_inc_dir = [libmpq_inc_dir, 'win']
endif

libmpq_inc = include_directories(libmpq_inc_dir)

lib_libmpq = static_library('libmpq',
                        libmpq_sources,
                        include_directories: libmpq_inc,
                        dependencies: [zlib_dep, bzip2_dep],
                        install : false
)