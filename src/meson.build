lib_inc = [include_directories('.'), amqp_inc]

lib_src = [
  'array.cpp',
  'channelimpl.cpp',
  'connectionimpl.cpp',
  'deferredcancel.cpp',
  'deferredconfirm.cpp',
  'deferredconsumer.cpp',
  'deferredextreceiver.cpp',
  'deferredget.cpp',
  'deferredpublisher.cpp',
  'deferredreceiver.cpp',
  'field.cpp',
  'flags.cpp',
  'receivedframe.cpp',
  'table.cpp',
  'watchable.cpp',
]
lib_deps = []

if get_option('linux-tcp')
  lib_src += [
    'linux_tcp/openssl.cpp',
    'linux_tcp/tcpconnection.cpp',
  ]
  lib_deps += dependency('openssl')
  lib_deps += dependency('threads')
  lib_deps += meson.get_compiler('cpp').find_library('dl')
endif

# Sorry, until meson 0.46 release adds --default-library both you only
# get shared or static.  You can use static, but amqpcpp.pc won't
# include the required pthread flags.
amqp_lib = library('amqpcpp', lib_src,
                   include_directories: lib_inc,
                   dependencies: lib_deps,
                   version: meson.project_version(),
                   soversion: '.'.join([version[0], version[1]]),
                   install: true,
                  )
