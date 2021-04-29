project('roots', 'cpp',
  default_options : ['cpp_std=c++17', 'cpp_args=-pedantic -Wall'])

sources = ['main.cpp']
executable('roots', sources)

gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_main_dep')
# gtest_dep = dependency('gtest', main: true, required: false)

mock_test = executable('mock_test', 'mock_test.cpp', dependencies: gtest_dep)
test('mock test', mock_test)
