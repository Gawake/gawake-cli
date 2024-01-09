gawake_dbus_server_sources = files(
	'gawake-dbus-server.c',
	'dbus-server.c',
	'database-connection.c',
	'gawake-types.c',
)

gawake_cli_sources = files(
	'gawake-cli.c',
	'gawake-types.c',
	'dbus-server.c',
	'dbus-client.c'
)


sqlite = dependency('sqlite3')
glib = dependency('glib-2.0')
gio = dependency('gio-2.0')
gio_unix = dependency('gio-unix-2.0')

gawake_cli_dependencies = [
	glib,
	gio,
	gio_unix
]

gawake_dbus_server_dependencies = [
	glib,
	gio,
	gio_unix,
	sqlite
]

executable(
	'gawake-dbus-server',
	gawake_dbus_server_sources,
	dependencies: gawake_dbus_server_dependencies,
	c_args: '-DPREPROCESSOR_DEBUG=1'
)

executable(
	'gawake-cli',
	gawake_cli_sources,
	dependencies: gawake_cli_dependencies
)