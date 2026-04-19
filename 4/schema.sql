-- Таблица пользователей
CREATE TABLE IF NOT EXISTS users (
    id            UUID PRIMARY KEY,
    login         VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    first_name    VARCHAR(100),
    last_name     VARCHAR(100),
    created_at    TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);
