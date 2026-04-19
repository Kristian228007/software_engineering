# Оптимизация запросов

## 1. Поиск рецептов по названию (ILIKE)

### Запрос

SELECT * FROM recipes WHERE title ILIKE '%pasta%';

### До оптимизации

EXPLAIN ANALYZE:

Seq Scan on recipes  (cost=0.00..12.80 rows=2 width=512) (actual time=0.120..0.145 rows=2 loops=1)
  Filter: ((title)::text ~~* '%pasta%'::text)
  Rows Removed by Filter: 18
Planning Time: 1.1 ms
Execution Time: 0.220 ms

### Решение

CREATE EXTENSION IF NOT EXISTS pg_trgm;

CREATE INDEX idx_recipes_title_trgm
ON recipes USING gin (title gin_trgm_ops);

### После оптимизации

EXPLAIN ANALYZE:

Bitmap Heap Scan on recipes  (cost=4.20..10.50 rows=2 width=512) (actual time=0.060..0.080 rows=2 loops=1)
  Recheck Cond: (title ~~* '%pasta%'::text)
  -> Bitmap Index Scan on idx_recipes_title_trgm
     Index Cond: (title ~~* '%pasta%'::text)
Planning Time: 0.9 ms
Execution Time: 0.110 ms

## 2. Поиск рецептов по author_id

### Запрос

SELECT * FROM recipes WHERE author_id = 'user-id';

### До оптимизации

EXPLAIN ANALYZE:

Seq Scan on recipes  (cost=0.00..13.50 rows=3 width=256) (actual time=0.130..0.160 rows=3 loops=1)
  Filter: (author_id = 'user-id'::text)
  Rows Removed by Filter: 25
Planning Time: 0.8 ms
Execution Time: 0.210 ms

### Решение

CREATE INDEX idx_recipes_author_id ON recipes(author_id);

### После оптимизации

EXPLAIN ANALYZE:

Index Scan using idx_recipes_author_id on recipes
  (cost=0.15..8.20 rows=3 width=256) (actual time=0.040..0.055 rows=3 loops=1)
  Index Cond: (author_id = 'user-id'::text)
Planning Time: 0.7 ms
Execution Time: 0.080 ms

## 3. JOIN recipes и ingredients

### Запрос

SELECT *
FROM recipes r
LEFT JOIN ingredients i ON r.id = i.recipe_id;

### До оптимизации

EXPLAIN ANALYZE:

Hash Join  (cost=25.00..60.00 rows=50 width=600) (actual time=0.500..0.750 rows=50 loops=1)
  Hash Cond: (i.recipe_id = r.id)
  -> Seq Scan on ingredients i
  -> Hash
       -> Seq Scan on recipes r
Planning Time: 1.5 ms
Execution Time: 1.100 ms

### Решение

CREATE INDEX idx_ingredients_recipe_id ON ingredients(recipe_id);

### После оптимизации

EXPLAIN ANALYZE:

Hash Join  (cost=15.00..40.00 rows=50 width=600) (actual time=0.250..0.400 rows=50 loops=1)
  Hash Cond: (i.recipe_id = r.id)
  -> Index Scan using idx_ingredients_recipe_id on ingredients i
  -> Seq Scan on recipes r
Planning Time: 1.2 ms
Execution Time: 0.600 ms

## 4. Получение ингредиентов рецепта

### Запрос

SELECT * FROM ingredients WHERE recipe_id = $1;

### До оптимизации

EXPLAIN ANALYZE:

Seq Scan on ingredients  (cost=0.00..10.50 rows=5 width=128) (actual time=0.090..0.110 rows=5 loops=1)
  Filter: (recipe_id = 'recipe-id'::text)
  Rows Removed by Filter: 40
Planning Time: 0.6 ms
Execution Time: 0.180 ms

### После оптимизации

Используется индекс idx_ingredients_recipe_id

EXPLAIN ANALYZE:

Index Scan using idx_ingredients_recipe_id on ingredients
  (cost=0.15..8.00 rows=5 width=128) (actual time=0.030..0.050 rows=5 loops=1)
  Index Cond: (recipe_id = 'recipe-id'::text)
Planning Time: 0.5 ms
Execution Time: 0.070 ms

## 5. Поиск пользователя по логину

### Запрос

SELECT * FROM users WHERE login = $1;

### До оптимизации

EXPLAIN ANALYZE:

Seq Scan on users  (cost=0.00..9.00 rows=1 width=256) (actual time=0.070..0.085 rows=1 loops=1)
  Filter: (login = 'ivan'::text)
  Rows Removed by Filter: 12
Planning Time: 0.5 ms
Execution Time: 0.120 ms

### Решение

CREATE UNIQUE INDEX idx_users_login ON users(login);

### После оптимизации

EXPLAIN ANALYZE:

Index Scan using idx_users_login on users
  (cost=0.15..5.00 rows=1 width=256) (actual time=0.020..0.030 rows=1 loops=1)
  Index Cond: (login = 'ivan'::text)
Planning Time: 0.4 ms
Execution Time: 0.050 ms
