amqp_inc = include_directories('.')

install_headers('amqpcpp.h')
install_subdir('amqpcpp',
               install_dir: 'include',
               exclude_directories: 'linux_tcp')
                
if get_option('linux-tcp')
  install_subdir('amqpcpp/linux_tcp',
                 install_dir: 'include/amqpcpp')
endif
