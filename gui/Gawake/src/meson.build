gawake_sources = [
  'main.vala',
  'application.vala',
  'window.vala',
  # 'header-bar-switcher.vala',
]

gawake_deps = [
  dependency('gtk4'),
]

gawake_sources += gnome.compile_resources('gawake-resources',
  'gawake.gresource.xml',
  c_name: 'gawake'
)

executable('gawake', gawake_sources,
  dependencies: gawake_deps,
       install: true,
)
