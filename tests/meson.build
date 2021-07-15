if get_option('googletest')
  gtest_sp = subproject('gtest')
  gtest_dep = gtest_sp.get_variable('gtest_main_dep')
  gmock_dep = gtest_sp.get_variable('gmock_main_dep')
else
  gtest_dep = disabler()
  gmock_dep = disabler()
endif

gt_bases = [
  'gt_address',
  'gt_numericfield',
]

foreach base: gt_bases
  test(base, executable(base, base + '.cpp',
                        dependencies: [
                          amqpcpp_dep,
                          gtest_dep,
                        ]))
endforeach
