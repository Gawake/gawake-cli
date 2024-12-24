CREATE TABLE IF NOT EXISTS rules_turnon (
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

CREATE TABLE IF NOT EXISTS rules_turnoff (
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

CREATE TABLE IF NOT EXISTS config (
	id                      INTEGER NOT NULL PRIMARY KEY,
	cli_version             TEXT,
	-- Boolean: use localtime or not (utc)
	localtime               INTEGER NOT NULL,
	default_mode            INTEGER NOT NULL,
	notification_time       INTEGER NOT NULL,
	-- Boolean: shutdown on failure
	shutdown_fail           INTEGER NOT NULL
);

INSERT OR IGNORE INTO config (id, cli_version, localtime, default_mode, notification_time, shutdown_fail)
VALUES (1, '3.1.0', 1, 2, 5, 0);

CREATE TABLE IF NOT EXISTS custom_schedule (
	id             INTEGER NOT NULL PRIMARY KEY,
	hour           INTEGER NOT NULL,
	minutes        INTEGER NOT NULL,
	day            INTEGER NOT NULL,
	month          INTEGER NOT NULL,
	year           INTEGER NOT NULL,
	mode           INTEGER NOT NULL
);

INSERT OR IGNORE INTO custom_schedule (id, hour, minutes, day, month, year, mode)
VALUES (1, 0, 0, 0, 0, 0, 0);