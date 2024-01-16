CREATE TABLE rules_turnon (
	id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	rule_name   TEXT NOT NULL,
	time        TEXT NOT NULL,
	sun         INTEGER NOT NULL,
	mon         INTEGER NOT NULL,
	tue         INTEGER NOT NULL,
	wed         INTEGER NOT NULL,
	thu         INTEGER NOT NULL,
	fri         INTEGER NOT NULL,
	sat         INTEGER NOT NULL,
	active      INTEGER NOT NULL
);

CREATE TABLE rules_turnoff (
	id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	rule_name   TEXT NOT NULL,
	time        TEXT NOT NULL,
	sun         INTEGER NOT NULL,
	mon         INTEGER NOT NULL,
	tue         INTEGER NOT NULL,
	wed         INTEGER NOT NULL,
	thu         INTEGER NOT NULL,
	fri         INTEGER NOT NULL,
	sat         INTEGER NOT NULL,
	active      INTEGER NOT NULL,
	mode        TEXT NOT NULL
);

CREATE TABLE config (
	id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	options     TEXT,
	status      INTEGER NOT NULL,
	version     TEXT,
	commands    INTEGER NOT NULL,
	localtime   INTEGER NOT NULL,
	def_mode    TEXT NOT NULL,
	boot_time   INTEGER NOT NULL
);

INSERT INTO config (options, status, version, commands, localtime, def_mode, boot_time)
VALUES ('-a', 1, '1.0.0', 0, 1, 'off', 120);