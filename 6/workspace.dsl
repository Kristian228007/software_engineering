workspace "Recipe Management System - Event-Driven Architecture" "CQRS Pattern with Kafka" {
    !identifiers hierarchical

    model {
        # Пользователи системы
        guest = person "Гость" "Неавторизованный пользователь, просматривает рецепты"
        user = person "Зарегистрированный пользователь" "Создает рецепты, добавляет ингредиенты"
        admin = person "Администратор" "Управляет системой"

        # Event-Driven Architecture с CQRS
        recipeSystem = softwareSystem "Recipe Management System" "Event-Driven архитектура с CQRS паттерном" {

            # API Service - Command Side (Write)
            apiService = container "API Service" {
                technology "C++, Poco Framework"
                description "REST API сервер - Command side CQRS. Принимает команды и отправляет события в Kafka"
                tags "api, command, producer"
            }

            # Consumer Service - Query Side (Read)
            consumerService = container "Consumer Service" {
                technology "C++, Poco Framework"
                description "Kafka consumer - Query side CQRS. Читает события из Kafka и сохраняет в БД"
                tags "consumer, query, event-processor"
            }

            # Kafka - Message Broker
            kafka = container "Apache Kafka" {
                technology "Kafka"
                description "Broker сообщений для Event-Driven архитектуры. Хранит topic 'recipe_events'"
                tags "message-broker, kafka"
            }

            # PostgreSQL - Write Database
            postgresDB = container "PostgreSQL Database" {
                technology "PostgreSQL"
                description "Write model CQRS. Хранит рецепты и ингредиенты"
                tags "database, postgresql, write-model"
            }

            # Redis - Read Cache
            redisCache = container "Redis Cache" {
                technology "Redis"
                description "Read model CQRS. Кэш для быстрого чтения популярных рецептов (write-through pattern)"
                tags "cache, redis, read-model"
            }
        }

        # Потоки данных

        # Command Flow: User -> API -> Kafka
        user -> recipeSystem.apiService "Отправляет команды (CREATE, UPDATE, DELETE)"
        recipeSystem.apiService -> recipeSystem.kafka "Публикует события в topic 'recipe_events'" "Kafka Protocol"
        recipeSystem.kafka -> recipeSystem.consumerService "Потребляет события" "Kafka Protocol"

        # Query Flow: User -> API -> Cache/DB
        user -> recipeSystem.apiService "Запрашивает данные (READ)"
        recipeSystem.apiService -> recipeSystem.redisCache "Проверяет кэш" "Redis Protocol"
        recipeSystem.apiService -> recipeSystem.postgresDB "Читает из БД при cache miss" "SQL"

        # Consumer пишет в БД
        recipeSystem.consumerService -> recipeSystem.postgresDB "Сохраняет события в БД" "SQL"

        # Write-through: API обновляет кэш
        recipeSystem.apiService -> recipeSystem.redisCache "Обновляет кэш после записи" "Redis Protocol"

        # Admin управляет системой
        admin -> recipeSystem "Администрирует"
    }

    views {
        # System Context
        systemContext recipeSystem {
            include *
            autoLayout
        }

        # Container View - Event-Driven Architecture
        container recipeSystem {
            include *
            autoLayout
        }

        # Dynamic View - CQRS Command Flow
        dynamic recipeSystem "CreateRecipeCommand" "Создание рецепта (CQRS Command)" {
            autoLayout lr

            user -> recipeSystem.apiService "POST /api/recipes (команда создания)"
            recipeSystem.apiService -> recipeSystem.kafka "produceRecipeCreated() -> topic: recipe_events"
            recipeSystem.apiService -> recipeSystem.redisCache "Обновление кэша (write-through)"
            recipeSystem.apiService -> user "Возвращает result"
        }

        # Dynamic View - Query Flow (Cache Miss scenario)
        dynamic recipeSystem "GetRecipeQuery" "Получение рецепта (CQRS Query)" {
            autoLayout lr

            user -> recipeSystem.apiService "GET /api/recipes/{id}"
            recipeSystem.apiService -> recipeSystem.redisCache "Проверка кэша"
            recipeSystem.redisCache -> recipeSystem.apiService "Cache Miss"
            recipeSystem.apiService -> recipeSystem.postgresDB "Запрос в БД"
            recipeSystem.postgresDB -> recipeSystem.apiService "Возвращает данные"
            recipeSystem.apiService -> recipeSystem.redisCache "Сохраняет в кэш"
            recipeSystem.apiService -> user "Возвращает рецепт"
        }

        # Dynamic View - Event Processing
        dynamic recipeSystem "EventProcessing" "Обработка событий из Kafka" {
            autoLayout lr

            recipeSystem.kafka -> recipeSystem.consumerService "consumer->poll() -> RECIPE_CREATED"
            recipeSystem.consumerService -> recipeSystem.postgresDB "INSERT INTO recipes"
            recipeSystem.consumerService -> recipeSystem.kafka "consumer.commit()"
        }

        styles {
            element "person" {
                shape person
                background #08427b
                color #ffffff
            }
            element "softwareSystem" {
                background #1168bd
                color #ffffff
            }
            element "container" {
                background #438dd5
                color #ffffff
            }
            element "database" {
                shape cylinder
                background #438dd5
                color #ffffff
            }
            element "cache" {
                shape cylinder
                background #ff9900
                color #ffffff
            }
            element "message-broker" {
                shape hexagon
                background #9bb63d
                color #ffffff
            }
            element "api" {
                background #2e7d32
                color #ffffff
            }
            element "command" {
                background #2e7d32
                color #ffffff
            }
            element "producer" {
                background #2e7d32
                color #ffffff
            }
            element "consumer" {
                background #1565c0
                color #ffffff
            }
            element "query" {
                background #1565c0
                color #ffffff
            }
            element "event-processor" {
                background #1565c0
                color #ffffff
            }
        }
    }
}
