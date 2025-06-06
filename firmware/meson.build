include = include_directories('include')

sources = run_command(
    find_program('python', required: true),
    meson.project_source_root() / 'extra' / 'scripts' / 'walker.py',
    '-p', 'src/**/*.[cs]',
    check: true,
).stdout().strip().split('\n')

executable(
    'firmware.elf',
    sources,
    include_directories: include,
    c_args: [c_warning_flags, '-fno-common'],
    link_args: ['-Wl,-Map=firmware.map'],
    install: false,
)
