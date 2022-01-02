CREATE TABLE gauge_data (
    timestamp INTEGER NOT NULL,
    gauge_id TEXT NOT NULL,
    flow NUMERIC DEFAULT NULL,
    PRIMARY KEY (timestamp, gauge_id)
);