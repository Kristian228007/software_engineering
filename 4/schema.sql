-- Таблица пользователей
CREATE TABLE IF NOT EXISTS users (
    id            UUID PRIMARY KEY,
    login         VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    first_name    VARCHAR(100),
    last_name     VARCHAR(100),
    created_at    TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Таблица рецептов
CREATE TABLE IF NOT EXISTS recipes (
    id          UUID PRIMARY KEY,
    title       VARCHAR(255) NOT NULL UNIQUE,
    description TEXT,
    author_id   UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at  TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Таблица ингредиентов
CREATE TABLE IF NOT EXISTS ingredients (
    id        UUID PRIMARY KEY,
    recipe_id UUID NOT NULL REFERENCES recipes(id) ON DELETE CASCADE,
    name      VARCHAR(255) NOT NULL,
    amount    VARCHAR(100),
    unit      VARCHAR(50),
    UNIQUE(recipe_id, name)
);

-- Индексы для оптимизации
CREATE INDEX IF NOT EXISTS idx_recipes_author_id ON recipes(author_id);
CREATE INDEX IF NOT EXISTS idx_ingredients_recipe_id ON ingredients(recipe_id);
CREATE INDEX IF NOT EXISTS idx_recipes_title ON recipes(title);
CREATE EXTENSION IF NOT EXISTS pg_trgm;
CREATE INDEX IF NOT EXISTS idx_recipes_title_trgm ON recipes USING gin (title gin_trgm_ops);