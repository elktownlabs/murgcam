CREATE TABLE IF NOT EXISTS cam_config (
	cam_id INTEGER DEFAULT 0 PRIMARY KEY,
	active_config TEXT DEFAULT NULL,
	active_timestamp INTEGER DEFAULT NULL,
	modified_config TEXT DEFAULT NULL,
	modified_timestamp INTEGER DEFAULT NULL,
	server_config TEXT DEFAULT NULL,
	server_timestamp INTEGER DEFAULT NULL
);


CREATE TABLE IF NOT EXISTS cam_frequency (
	cam_id INTEGER DEFAULT 0 PRIMARY KEY NOT NULL,
	current_mode INTEGER DEFAULT 0,
	current_mode_start INTEGER DEFAULT NULL,
	current_mode_end INTEGER DEFAULT NULL,
	current_mode_initiating_user TEXT DEFAULT NULL,
	current_mode_initiating_time INTEGER DEFAULT NULL,
	requested_mode INTEGER DEFAULT NULL,
	requested_mode_end INTEGER DEFAULT NULL,
	requested_mode_initiating_time INTEGER DEFAULT NULL,
	requested_mode_initiating_user TEXT DEFAULT NULL
);


CREATE TABLE IF NOT EXISTS users (
	id TEXT PRIMARY KEY,
	password TEXT NOT NULL,
	full_name TEXT DEFAULT "unnamed user",
	rights TEXT DEFAULT NULL,
	avatar TEXT DEFAULT NULL,
	initials TEXT DEFAULT NULL,
	login_duration INTEGER DEFAULT 86400
);

CREATE TABLE IF NOT EXISTS active_logins (
	token TEXT PRIMARY_KEY,
	user_id TEXT NOT NULL,
	ip TEXT NOT NULL,
	login_time INTEGER DEFAULT NULL,
	expiration INT NOT NULL,
	FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE cascade ON UPDATE cascade
);

