# Non-standard build rules using meson
#
# Until meson 0.46 is released with support for default_library=both
# you can only build either static or shared.  Default is shared.
#
# Support for building examples hasn't been added.

project('amqpcpp', 'cpp',
        version: '3.1.0',
        default_options: [
          'cpp_std=c++11',
          'werror=true',
        ],
        meson_version: '>= 0.44',
       )

version = meson.project_version().split('.')

conf = configuration_data()
conf.set_quoted('PACKAGE_VERSION', meson.project_version())
conf.set('DEST_DIR', get_option('prefix'))
conf.set('VERSION_MAJOR', version[0])
conf.set('VERSION_MINOR', version[1])
conf.set('VERSION_PATCH', version[2])

doxygen = find_program('doxygen', required: false)
if find_program('dot', required: false).found()
  conf.set('HAVE_DOT', 'YES')
else
  conf.set('HAVE_DOT', 'NO')
endif
subdir('doc')

subdir('include')
subdir('src')

amqpcpp_dep = declare_dependency(link_with: amqp_lib,
                                 include_directories: amqp_inc,
                                 dependencies: lib_deps)

pkg = import('pkgconfig')
pkg.generate(libraries: lib_deps + [amqp_lib],
             version: meson.project_version(),
             name: 'libamqpcpp',
             filebase: 'amqpcpp',
             description: 'AMQP-CPP is a C++ library for communicating with a RabbitMQ message broker')

subdir('tests')
