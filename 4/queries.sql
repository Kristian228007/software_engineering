-- 1. Создание пользователя
INSERT INTO users (login, password_hash, first_name, last_name)
VALUES ($1, $2, $3, $4)
RETURNING id, login;

-- 2. Получение всех пользователей
SELECT id, login, first_name, last_name FROM users;

-- 3. Получение пользователя по логину
SELECT id, login, password_hash, first_name, last_name
FROM users
WHERE login = $1;

-- 4. Аутентификация
SELECT id, password_hash FROM users WHERE login = $1;

-- 5. Создание рецепта (с проверкой уникальности)
--    Проверка:
SELECT 1 FROM recipes WHERE title = $1;
--    Вставка:
INSERT INTO recipes (title, description, author_id)
VALUES ($1, $2, $3)
RETURNING id, title, description, author_id, created_at;

-- 6. Получение всех рецептов с ингредиентами
SELECT
    r.id, r.title, r.description, r.author_id, r.created_at,
    COALESCE(
        json_agg(json_build_object('id', i.id, 'name', i.name, 'amount', i.amount, 'unit', i.unit))
        FILTER (WHERE i.id IS NOT NULL), '[]'
    ) AS ingredients
FROM recipes r
LEFT JOIN ingredients i ON r.id = i.recipe_id
GROUP BY r.id;

-- 7. Поиск рецептов по названию
SELECT * FROM recipes
WHERE title ILIKE '%' || $1 || '%';

-- 8. Рецепты пользователя
SELECT * FROM recipes WHERE author_id = $1;

-- 9. Добавление ингредиента (с проверкой уникальности)
--    Проверка:
SELECT 1 FROM ingredients WHERE recipe_id = $1 AND name = $2;
--    Вставка:
INSERT INTO ingredients (recipe_id, name, amount, unit)
VALUES ($1, $2, $3, $4)
RETURNING id, name, amount, unit;

-- 10. Получение ингредиентов рецепта
SELECT id, name, amount, unit FROM ingredients WHERE recipe_id = $1;