project('roots', 'cpp',
  default_options : ['cpp_std=c++17', 'cpp_args=-pedantic -Wall'])

sources = ['expr.cpp', 'matrix.cpp', 'solve.cpp']
# executable('roots', sources + ['main.cpp'])

qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets'])
qt5_moc = qt5.preprocess(moc_headers: 'mainwindow.h', ui_files: 'mainwindow.ui')
executable('roots', sources + ['main.cpp', 'mainwindow.cpp'] + qt5_moc, dependencies: qt5_dep)

gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_main_dep')
# gtest_dep = dependency('gtest', main: true, required: false)

expr_test = executable('expr_test', sources + ['expr_test.cpp'], dependencies: gtest_dep)
test('expr test', expr_test)
matrix_test = executable('matrix_test', sources + ['matrix_test.cpp'], dependencies: gtest_dep)
test('matrix test', matrix_test)
solve_test = executable('solve_test', sources + ['solve_test.cpp'], dependencies: gtest_dep)
test('solve test', solve_test)
