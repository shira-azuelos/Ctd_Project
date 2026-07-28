CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    elo INT DEFAULT 1200,
    wins INT DEFAULT 0,
    losses INT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS games (
    id SERIAL PRIMARY KEY,
    room_id VARCHAR(50) NOT NULL,
    white_user VARCHAR(50) NOT NULL,
    black_user VARCHAR(50) NOT NULL,
    winner VARCHAR(50) DEFAULT '',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO users (username, password, elo, wins, losses) 
VALUES 
    ('shira', '1234', 1250, 5, 1),
    ('uria', '1234', 1200, 3, 2),
    ('yael', '1234', 1180, 1, 3),
    ('guest', 'guest', 1000, 0, 0)
ON CONFLICT (username) DO NOTHING;
