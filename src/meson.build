# SOURCES DIRECTORIES
subdir('utils')
subdir('database-connection')
subdir('gawaked')
subdir('gawake-cli')
subdir('gawake-dbus-server')

# DEPENDENCIES
# TODO review versions for distributing for Debian >= 12; add missing versions
sqlite = dependency('sqlite3')
glib = dependency('glib-2.0', version: '>= 2.72')
gio = dependency('gio-2.0', version: '>= 2.72')
gio_unix = dependency('gio-unix-2.0', version: '>= 2.72')

gawake_dbus_server_dependencies = [
	glib,
	gio,
	gio_unix
]

# TODO remove GIO (?)
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
# executable(
# 	'gawake-dbus-server.out',
# 	gawake_dbus_server_sources,
# 	dependencies: gawake_dbus_server_dependencies
# )

# executable(
# 	'gawaked.out',
# 	gawaked_sources,
# 	dependencies: gawaked_dependencies
# )

executable(
	'gawake-cli.out',
	gawake_cli_sources,
	dependencies: gawake_cli_dependencies
)