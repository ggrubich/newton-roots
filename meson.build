project('roots', 'cpp',
  default_options : ['cpp_std=c++17', 'cpp_args=-pedantic -Wall'])

sources = ['expr.cpp']
executable('roots', sources + ['main.cpp'])

gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_main_dep')
# gtest_dep = dependency('gtest', main: true, required: false)

expr_test = executable('expr_test', sources + ['expr_test.cpp'], dependencies: gtest_dep)
test('expr test', expr_test)
