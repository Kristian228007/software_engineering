# Recipe Management REST API

REST API сервис для управления рецептами.

Основа проекта взята из лабы 3. Добавлены redis для частых запросов и rate-limiting.

## Стек

- C++
- POCO
- JWT
- Docker
- PostgreSQL
- Redis (кеширование + rate limiting)

## Запуск

docker compose up --build

## Swagger:

http://localhost:8080/swagger.yaml

## API docs:

http://localhost:8080/docs
