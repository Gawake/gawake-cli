# SOURCES DIRECTORIES
subdir('utils')
subdir('gawaked')
subdir('gawake-cli')
subdir('gawake-dbus-server')

# DEPENDENCIES
sqlite = dependency('sqlite3')
glib = dependency('glib-2.0', version: '>= 2.72')
gio = dependency('gio-2.0', version: '>= 2.72')
gio_unix = dependency('gio-unix-2.0', version: '>= 2.72')

gawake_dbus_server_dependencies = [
	glib,
	gio,
	gio_unix
]

gawaked_dependencies = [
	glib,
	gio,
	gio_unix,
	sqlite
]

gawake_cli_dependencies = [
	glib,
	gio,
	gio_unix,
	sqlite
]

# EXECUTABLES
executable(
	'gawake-dbus-server.out',
	gawake_dbus_server_sources,
	dependencies: gawake_dbus_server_dependencies,
	c_args: '-DPREPROCESSOR_DEBUG=1'
)

executable(
	'gawaked.out',
	gawaked_sources,
	dependencies: gawaked_dependencies,
	c_args: '-DPREPROCESSOR_DEBUG=1'
)

executable(
	'gawake-cli.out',
	gawake_cli_sources,
	dependencies: gawake_cli_dependencies,
	c_args: '-DPREPROCESSOR_DEBUG=1'
)