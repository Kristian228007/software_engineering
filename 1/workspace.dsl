workspace "Recipe Management System" "Система управления рецептами" {
    !identifiers hierarchical
    
    model {
        # Роли пользователей)
        guest = person "Гость" "Неавторизованный пользователь, просматривает рецепты"
        user = person "Зарегистрированный пользователь" "Создает рецепты, добавляет ингредиенты, работает с избранным"
        admin = person "Администратор" "Управляет системой и контентом"
        
        # Внешние системы
        emailSystem = softwareSystem "Email-сервис" "Отправка уведомлений пользователям" "external"
        authSystem = softwareSystem "Система аутентификации" "Внешний OAuth/OIDC провайдер" "external"
        
        recipeSystem = softwareSystem "Система управления рецептами" "Позволяет пользователям создавать, хранить и делиться рецептами" {
            
            webApp = container "Web Application" {
                technology "React, TypeScript"
                description "Пользовательский интерфейс для взаимодействия с системой"
                tags "frontend"
            }
            
            apiGateway = container "API Gateway" {
                technology "Nest.js, TypeScript"
                description "Единая точка входа для всех API-запросов, маршрутизация, аутентификация"
                tags "js, ts, gateway"
            }
            
            userService = container "User Service" {
                technology "Mux, Go"
                description "Управление пользователями: регистрация, поиск, профили"
                tags "go, microservice"
            }
            
            recipeService = container "Recipe Service" {
                technology "Mux, Go"
                description "Управление рецептами и ингредиентами: создание, редактирование, поиск"
                tags "go, microservice"
            }
            
            favoritesService = container "Favorites Service" {
                technology "Mux, Go"
                description "Управление избранными рецептами пользователей"
                tags "go, microservice"
            }
            
            database = container "PostgreSQL Database" {
                technology "PostgreSQL"
                description "Хранение данных о пользователях, рецептах, ингредиентах и избранном"
                tags "database"
            }
            
            cache = container "Redis Cache" {
                technology "Redis"
                description "Кэширование популярных рецептов и сессий"
                tags "cache"
            }
        }
        
        # Связи между контейнерами внутри системы
        recipeSystem.webApp -> recipeSystem.apiGateway "Отправляет HTTPS/REST API запросы"
        
        recipeSystem.apiGateway -> recipeSystem.userService "Маршрутизация запросов /api/users/**" "HTTPS/REST"
        recipeSystem.apiGateway -> recipeSystem.recipeService "Маршрутизация запросов /api/recipes/**" "HTTPS/REST"
        recipeSystem.apiGateway -> recipeSystem.favoritesService "Маршрутизация запросов /api/favorites/**" "HTTPS/REST"
        recipeSystem.apiGateway -> authSystem "Проверка токенов" "HTTPS/OIDC"
        
        recipeSystem.userService -> recipeSystem.database "CRUD операции" "GORM"
        recipeSystem.recipeService -> recipeSystem.database "CRUD операции" "GORM"
        recipeSystem.favoritesService -> recipeSystem.database "CRUD операции" "GORM"
        
        recipeSystem.recipeService -> recipeSystem.cache "Кэширование популярных рецептов" "Redis Protocol"
        
        recipeSystem.userService -> emailSystem "Отправка писем" "SMTP/HTTPS"
        
        # Связи пользователей с веб-приложением
        guest -> recipeSystem.webApp "HTTPS просмотр рецептов"
        user -> recipeSystem.webApp "HTTPS управление рецептами"
        admin -> recipeSystem.webApp "HTTPS администрирование"
        
        # Связи для диаграммы контекста
        guest -> recipeSystem "Просматривает рецепты"
        user -> recipeSystem "Управляет рецептами и избранным"
        admin -> recipeSystem "Администрирует систему"
        recipeSystem -> emailSystem "Отправляет email-уведомления"
        recipeSystem -> authSystem "Аутентифицирует пользователей"
    }
    
    views {
        # System Context view
        systemContext recipeSystem {
            include *
            autoLayout
        }
        
        # Container view
        container recipeSystem {
            include *
            autoLayout
        }
        
        # Dynamic view для сценария "Создание рецепта"
        dynamic recipeSystem "CreateRecipe" "Создание нового рецепта с ингредиентами" {
            autoLayout lr
            
            user -> recipeSystem.webApp "Заполняет форму рецепта"
            recipeSystem.webApp -> recipeSystem.apiGateway "POST /api/recipes"
            recipeSystem.apiGateway -> recipeSystem.recipeService "Создание рецепта"
            
            # Добавление первого ингредиента
            user -> recipeSystem.webApp "Добавляет первый ингредиент"
            recipeSystem.webApp -> recipeSystem.apiGateway "POST /api/recipes/{id}/ingredients (1)"
            recipeSystem.apiGateway -> recipeSystem.recipeService "Добавление первого ингредиента"
            recipeSystem.recipeService -> recipeSystem.database "Сохраняет первый ингредиент"
            
            # Добавление второго ингредиента
            user -> recipeSystem.webApp "Добавляет второй ингредиент"
            recipeSystem.webApp -> recipeSystem.apiGateway "POST /api/recipes/{id}/ingredients (2)"
            recipeSystem.apiGateway -> recipeSystem.recipeService "Добавление второго ингредиента"
            recipeSystem.recipeService -> recipeSystem.database "Сохраняет второй ингредиент"
            
            # Завершение создания рецепта
            recipeSystem.recipeService -> recipeSystem.cache "Инвалидирует кэш"
            recipeSystem.apiGateway -> recipeSystem.webApp "Возвращает результат"
            recipeSystem.webApp -> user "Отображает созданный рецепт"
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
            element "external" {
                background #999999
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
            element "frontend" {
                background #6bb7d5
                color #ffffff
            }
            element "go" {
                background #2e7d32
                color #ffffff
            }
            element "microservice" {
                background #2e7d32
                color #ffffff
            }
            element "gateway" {
                background #7b1fa2
                color #ffffff
            }
        }
    }
}
