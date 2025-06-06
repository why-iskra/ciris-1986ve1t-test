project(
    'milandr',
    'c',
    default_options: {
        'c_std': 'c17',
        'warning_level': '2',
        'werror': false,
        'buildtype': 'debug',
        'debug': true,
        'optimization' : 'plain',

        'c_args': [
            '-ffreestanding',
            '-nostartfiles',
            '-mcpu=cortex-m1',
            '-mthumb',
        ],

        'c_link_args': [
            '-ffreestanding',
            '-nostartfiles',
            '-mcpu=cortex-m1',
            '-mthumb',

            '--specs=nosys.specs',
            '-T', meson.project_source_root() / 'extra/1986ve1t.ld',
            '-Wl,--print-memory-usage',
        ],
    },
)

c_warning_flags = [
    '-Wpedantic',
    '-Wdouble-promotion',
    '-Wshadow',
    '-Wformat',
    '-Wstrict-prototypes',
    '-Wunused-parameter',
    '-Wconversion',
    '-Wredundant-decls',
    '-Wmissing-prototypes',
    '-Wmissing-declarations',
    '-Wmissing-field-initializers',
    '-Wimplicit-function-declaration',
    '-Wvla',
    '-Wcast-qual',
    '-Woverflow',
]

subdir('firmware')
