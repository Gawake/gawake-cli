gawake_sources = [
  'main.vala',
  'application.vala',
  'window.vala',
  'utils.vala',
  'database-connection.vala',
  'rule-setup-dialog.vala',
  'rule-row.vala',
]

# FIXME
# pd = get_option('PREPROCESSOR_DEBUG')
# add_global_arguments('-DPREPROCESSOR_DEBUG=@pd@', language : 'vala')

gawake_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
  dependency('sqlite3'),
]

gawake_sources += gnome.compile_resources('gawake-resources',
  'gawake.gresource.xml',
  c_name: 'gawake'
)

executable('gawake', gawake_sources,
  dependencies: gawake_deps,
       install: true,
)