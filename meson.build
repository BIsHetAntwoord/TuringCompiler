project(
    'TuringCompiler',
    ['cpp'],
    version: '0.0.0',
    default_options: [
        'cpp_std=c++2a',
        'buildtype=debugoptimized',
    ]
)

cpp = meson.get_compiler('cpp')

# ANTLR setup
flex_exec = find_program('flex')
bison_exec = find_program('bison')

flex_gen = generator(flex_exec,
    output: [
        '@BASENAME@.lex.cc',
        '@BASENAME@.lex.h'
    ],
    arguments: ['--header-file=@BUILD_DIR@/@BASENAME@.lex.h', '-o', '@BUILD_DIR@/@BASENAME@.lex.cc', '@INPUT@']
    )
bison_gen = generator(bison_exec,
    output: [
        '@BASENAME@.parse.cc',
        '@BASENAME@.parse.h'
    ],
    arguments: ['--header=@BUILD_DIR@/@BASENAME@.parse.h', '-o', '@BUILD_DIR@/@BASENAME@.parse.cc', '-rall', '@INPUT@']
    )

bison_sources = bison_gen.process('grammar/turingc.y')
flex_sources = flex_gen.process('grammar/turingc.l')

# Final executable
sources = [
    'src/backend/instr.cpp',
    'src/backend/turingcompiler.cpp',
    'src/backend/turingstate.cpp',
    'src/output/binarywriter.cpp',
    'src/utils.cpp'
]

sources_asm = [
    'src/assembler/parser.cpp',
    'src/assembler/main.cpp'
]

sources_c = [
    'src/frontend/asmgen.cpp',
    'src/frontend/ast.cpp',
    'src/frontend/main.cpp',
    'src/frontend/semcheck.cpp',
    'src/frontend/symtab.cpp'
]

executable(
    'turingasm',
    [sources, sources_asm],
    install: true,
    build_by_default: true,
    include_directories: [include_directories('include')]
)

executable(
    'turingc',
    [sources, sources_c, bison_sources, flex_sources],
    install: true,
    build_by_default: true,
    include_directories: [include_directories('include')]
)