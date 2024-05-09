# Used locally as a (string) list of files
_utils_debugger = [
	'debugger.c',
	'get-time.c'
]

utils_debugger = files(
	_utils_debugger
)

utils_database_connection = files(
	'database-connection.c',
	'validate-rtcwake-args.c',

	# To send signals through D-Bus
	'../gawake-dbus-server/dbus-server.c',
	'dbus-client.c',

	_utils_debugger
)

utils_gawake_types = files(
	'gawake-types.c'
)

utils_get_time = files(
	'get-time.c'
)

utils_validate_rtcwake_args = files(
	'validate-rtcwake-args.c',
	_utils_debugger
)