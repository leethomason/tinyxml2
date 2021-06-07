# Copyright © 2020 Dylan Baker

# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any
# damages arising from the use of this software.

# Permission is granted to anyone to use this software for any
# purpose, including commercial applications, and to alter it and
# redistribute it freely, subject to the following restrictions:

# 1. The origin of this software must not be misrepresented; you must
# not claim that you wrote the original software. If you use this
# software in a product, an acknowledgment in the product documentation
# would be appreciated but is not required.

# 2. Altered source versions must be plainly marked as such, and
# must not be misrepresented as being the original software.

# 3. This notice may not be removed or altered from any source
# distribution.

project(
    'tinyxml2',
    ['cpp'],
    version : '9.0.0',
    meson_version : '>= 0.49.0',
)

cpp = meson.get_compiler('cpp')

tinyxml_extra_args = []
if cpp.get_argument_syntax() == 'msvc'
    tinyxml_extra_args += '-D_CRT_SECURE_NO_WARNINGS'
endif

if get_option('default_library') == 'shared'
    tinyxml_extra_args += '-DTINYXML2_EXPORT'
endif

if get_option('debug')
    tinyxml_extra_args += '-DTINYXML2_DEBUG'
endif

lib_tinyxml2 = library(
    'tinyxml2',
    ['tinyxml2.cpp'],
    cpp_args : tinyxml_extra_args,
    gnu_symbol_visibility : 'hidden',
    version : meson.project_version(),
    install : true,
)

dep_tinyxml2 = declare_dependency(
    link_with : lib_tinyxml2,
    include_directories : include_directories('.'),
)

# This is the new way to set dependencies, but let's not break users of older
# versions of meson
if meson.version().version_compare('>= 0.54.0')
  meson.override_dependency('tinyxml2', dep_tinyxml2)
endif

if get_option('tests')
    # Try to find a copy command. If this is windows we probably don't have cp,
    # but if this is msys then we do, so make cp not required in that case, and
    # try Xcopy if cp isn't found
    prog_cp = find_program('cp', required : build_machine.system() != 'windows')
    command = ['-r']
    if not prog_cp.found()
        prog_cp = find_program('Xcopy')
        command = ['/E', '/I']
    endif

    # Copy the test resources into the build dir
    run_command(
        prog_cp,
        [
            command,
            meson.current_source_dir() / 'resources',
            meson.current_build_dir(),
        ],
    )

    test(
        'xmltest',
        executable(
            'xmltest',
            ['xmltest.cpp'],
            link_with : [lib_tinyxml2],
        ),
        workdir : meson.current_build_dir(),
    )
endif

install_headers('tinyxml2.h')

# This is better than using the .in because meson tracks dependencies
# internally, and will generate a more accurate pkg-config file
pkg = import('pkgconfig')
pkg.generate(
    lib_tinyxml2,
    description : 'simple, small, C++ XML parser',
)
