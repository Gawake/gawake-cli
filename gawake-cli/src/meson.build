# SOURCES
gawake_dbus_server_sources = files(
	'gawake-dbus-server.c',
	'dbus-server.c',
	'database-connection.c',
	'gawake-types.c',
)

gawaked_sources = files(
	'gawaked.c',
	'gawake-types.c',
	# 'scheduler.c',
	'get-time.c'
)

gawake_cli_sources = files(
	'gawake-cli.c',
	'gawake-types.c',
	'dbus-server.c',
	'dbus-client.c'
)

# DEPENDENCIES
sqlite = dependency('sqlite3')
glib = dependency('glib-2.0')
gio = dependency('gio-2.0')
gio_unix = dependency('gio-unix-2.0')

gawake_dbus_server_dependencies = [
	glib,
	gio,
	gio_unix,
	sqlite
]

gawaked_dependencies = [
	sqlite
]

gawake_cli_dependencies = [
	glib,
	gio,
	gio_unix
]



# EXECUTABLES
executable(
	'gawake-dbus-server',
	gawake_dbus_server_sources,
	dependencies: gawake_dbus_server_dependencies,
	c_args: '-DPREPROCESSOR_DEBUG=1'
)

executable(
	'gawaked',
	gawaked_sources,
	dependencies: gawaked_dependencies
)

executable(
	'gawake-cli',
	gawake_cli_sources,
	dependencies: gawake_cli_dependencies
)