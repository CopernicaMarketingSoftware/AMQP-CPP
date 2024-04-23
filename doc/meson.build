conf.set('TOP_SRCDIR', meson.source_root())
conf.set('TOP_BUILDDIR', meson.build_root())

datadir = join_paths(get_option('datadir'), 'doc')
conf.set('DOCDIR', 'doc')

if doxygen.found()
  doxycfg = configure_file(input: 'doxygen.cfg.in',
			   output: 'doxygen.cfg',
			   configuration: conf,
			   install: false)
  
  html_target = custom_target('docs',
			      input: doxycfg,
			      output: 'html',
			      command: [doxygen, doxycfg],
			      install: true,
			      install_dir: datadir)
endif
