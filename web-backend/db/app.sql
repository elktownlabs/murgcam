CREATE TABLE IF NOT EXISTS users (
	username TEXT NOT NULL,
	password TEXT NOT NULL,
	fullname TEXT DEFAULT NULL,
	active INTEGER NOT NULL DEFAULT 1
);


CREATE TABLE IF NOT EXISTS config (
        cam_id INTEGER DEFAULT 0 PRIMARY KEY,
        active_config TEXT DEFAULT NULL,
        active_timestamp INTEGER DEFAULT NULL,
        modified_config TEXT DEFAULT NULL,
        modified_timestamp INTEGER DEFAULT NULL
);

