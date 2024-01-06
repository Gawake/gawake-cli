gawaked_sources = files(
	'gawaked.c',
	'dbus-server.c',
	'privileges.c',
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

gawaked_dependencies = [
	glib,
	gio,
	gio_unix,
	sqlite
]

executable(
	'gawaked',
	gawaked_sources,
	dependencies: gawaked_dependencies,
	c_args: '-DPREPROCESSOR_DEBUG=1'
)

executable(
	'gawake-cli',
	gawake_cli_sources,
	dependencies: gawake_cli_dependencies
)