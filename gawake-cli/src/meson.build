gawaked_sources = files(
	'gawaked.c',
	'gawake-dbus.c',
	'privileges.c',
	'database-connection.c',
)

dbus_client_sources = files(
	'dbus-client.c',
	'gawake-dbus.c'
)

dbus_client_sync_sources = files(
	'dbus-client-sync.c',
	'gawake-dbus.c'
)

sqlite = dependency('sqlite3')
glib = dependency('glib-2.0')
gio = dependency('gio-2.0')
gio_unix = dependency('gio-unix-2.0')

gawake_cli_dependencies = [

]

gawaked_dependencies = [
	glib,
	gio,
	gio_unix,
	sqlite
]

dbus_client_dependencies = [
	glib,
	gio,
	gio_unix
]

dbus_client_sync_dependencies = dbus_client_dependencies

executable(
	'gawaked',
	gawaked_sources,
	dependencies: gawaked_dependencies,
	c_args: '-DPREPROCESSOR_DEBUG'
)

executable(
	'dbus-client',
	dbus_client_sources,
	dependencies: dbus_client_dependencies
)

executable(
	'dbus-client-sync',
	dbus_client_sync_sources,
	dependencies: dbus_client_sync_dependencies
)

# executable(
# 	'gawake-cli',
# 	gawake_cli_sources,
# 	dependencies: gawake_cli_dependencies
# )