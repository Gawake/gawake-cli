gawake_cli_sources = files(

)

gawaked_sources = files(
	'gawaked.c',
	'gawake-dbus.c',
	'privileges.c',
	'database-connection.c',
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

executable(
	'gawaked',
	gawaked_sources,
	dependencies: gawaked_dependencies,
	c_args: '-DPREPROCESSOR_DEBUG'
)

# executable(
# 	'gawake-cli',
# 	gawake_cli_sources,
# 	dependencies: gawake_cli_dependencies
# )