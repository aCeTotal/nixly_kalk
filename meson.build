project('nixlykalk', 'cpp',
  version: '1.0',
  meson_version: '>=1.1',
  default_options: ['cpp_std=c++20', 'warning_level=3', 'buildtype=debugoptimized'],
)

qt_core = dependency('qt6', modules: ['Core'])
qt_widgets = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])

install_data('desktop/nixlykalk.desktop',
  install_dir: get_option('datadir') / 'applications',
)
install_data('desktop/nixlykalk.svg',
  install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps',
)

subdir('src')
subdir('tests')
