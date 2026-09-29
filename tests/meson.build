foreach name : ['math', 'entry', 'pretty']
  test(name, executable(name + '-test', name + '.cpp',
    include_directories: src_inc,
    link_with: engine,
    dependencies: qt_core,
  ))
endforeach
