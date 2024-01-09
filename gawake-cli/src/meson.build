init_gawake_sources = files(
	'init-gawake.c',
	'scheduler.c',
	'gawake-service.c',
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

init_gawake_dependencies = [
	glib,
	gio,
	gio_unix,
	sqlite
]

executable(
	'init-gawake',
	init_gawake_sources,
	dependencies: init_gawake_dependencies,
	c_args: '-DPREPROCESSOR_DEBUG=1'
)

executable(
	'gawake-cli',
	gawake_cli_sources,
	dependencies: gawake_cli_dependencies
)