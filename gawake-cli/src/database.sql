CREATE TABLE rules_turnon (
	id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	rule_name   TEXT NOT NULL,
	rule_time   TEXT NOT NULL,
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
	rule_time   TEXT NOT NULL,
	sun         INTEGER NOT NULL,
	mon         INTEGER NOT NULL,
	tue         INTEGER NOT NULL,
	wed         INTEGER NOT NULL,
	thu         INTEGER NOT NULL,
	fri         INTEGER NOT NULL,
	sat         INTEGER NOT NULL,
	active      INTEGER NOT NULL,
	mode        INTEGER NOT NULL
);

CREATE TABLE config (
	id                      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	status                  INTEGER NOT NULL,
	cli_version             TEXT,
	-- Boolean: use localtime or not (utc)
	localtime               INTEGER NOT NULL,
	default_mode            INTEGER NOT NULL,
	boot_time               INTEGER NOT NULL,
	notification_time       INTEGER NOT NULL,
	-- Boolean: shutdown on failure
	shutdown_fail           INTEGER NOT NULL
);

INSERT INTO config (status, cli_version, localtime, default_mode, boot_time, notification_time)
VALUES (1, '1.0.0', 1, 2, 120, 5, 1);