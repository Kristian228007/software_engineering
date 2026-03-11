workspace "Recipe Management System" "Система управления рецептами" {
    !identifiers hierarchical
    
    model {
        # Роли пользователей (три типа доступа)
        guest = person "Гость" "Неавторизованный пользователь, просматривает рецепты"
        user = person "Зарегистрированный пользователь" "Создает рецепты, добавляет ингредиенты, работает с избранным"
        admin = person "Администратор" "Управляет системой и контентом, модернирует рецепты"
        
        # Внешние системы (с которыми мы интегрируемся)
        emailSystem = softwareSystem "Email-сервис" "Отправка уведомлений пользователям (подтверждение регистрации, восстановление пароля)" "external"
        authSystem = softwareSystem "Система аутентификации" "Внешний OAuth/OIDC провайдер" "external"
        
        # Наша основная система
        recipeSystem = softwareSystem "Система управления рецептами" "Позволяет пользователям создавать, хранить и делиться рецептами" {
            
            webApp = container "Web Application" {
                technology "React, TypeScript"
                description "Пользовательский интерфейс для взаимодействия с системой (фронтенд)"
                tags "frontend"
            }
            
            apiGateway = container "API Gateway" {
                technology "Nest.js, TypeScript"
                description "Единая точка входа для всех API-запросов, занимается маршрутизацией и проверкой аутентификации"
                tags "js, ts, gateway"
            }
            
            userService = container "User Service" {
                technology "Mux, Go"
                description "Всё что связано с пользователями: регистрация, поиск, редактирование профилей"
                tags "go, microservice"
            }
            
            recipeService = container "Recipe Service" {
                technology "Mux, Go"
                description "Управление рецептами и ингредиентами: создание, редактирование, поиск по названию"
                tags "go, microservice"
            }
            
            favoritesService = container "Favorites Service" {
                technology "Mux, Go"
                description "Избранное: добавление/удаление рецептов, получение списка избранного для пользователя"
                tags "go, microservice"
            }
            
            database = container "PostgreSQL Database" {
                technology "PostgreSQL"
                description "Основное хранилище данных: пользователи, рецепты, ингредиенты, избранное"
                tags "database"
            }
            
            cache = container "Redis Cache" {
                technology "Redis"
                description "Кэш для популярных рецептов и пользовательских сессий (чтобы быстрее работало)"
                tags "cache"
            }
        }
        
        # Связи между контейнерами внутри нашей системы
        recipeSystem.webApp -> recipeSystem.apiGateway "Посылает HTTPS/REST запросы (все апи вызовы)"
        
        recipeSystem.apiGateway -> recipeSystem.userService "Направляет запросы по пути /api/users/**" "HTTPS/REST"
        recipeSystem.apiGateway -> recipeSystem.recipeService "Направляет запросы по пути /api/recipes/**" "HTTPS/REST"
        recipeSystem.apiGateway -> recipeSystem.favoritesService "Направляет запросы по пути /api/favorites/**" "HTTPS/REST"
        recipeSystem.apiGateway -> authSystem "Проверяет JWT токены" "HTTPS/OIDC"
        
        recipeSystem.userService -> recipeSystem.database "Читает и пишет данные через GORM" "SQL"
        recipeSystem.recipeService -> recipeSystem.database "Читает и пишет данные через GORM" "SQL"
        recipeSystem.favoritesService -> recipeSystem.database "Читает и пишет данные через GORM" "SQL"
        
        recipeSystem.recipeService -> recipeSystem.cache "Кэширует популярные рецепты" "Redis Protocol"
        
        recipeSystem.userService -> emailSystem "Отправляет письма при регистрации" "SMTP/HTTPS"
        
        # Связи пользователей с веб-приложением
        guest -> recipeSystem.webApp "Заходит на сайт, смотрит рецепты" "HTTPS"
        user -> recipeSystem.webApp "Создает рецепты, добавляет в избранное" "HTTPS"
        admin -> recipeSystem.webApp "Заходит в админку" "HTTPS"
        
        # Связи для диаграммы контекста (высокоуровневые)
        guest -> recipeSystem "Просматривает рецепты без регистрации"
        user -> recipeSystem "Управляет своими рецептами и избранным"
        admin -> recipeSystem "Администрирует систему"
        recipeSystem -> emailSystem "Отправляет email-уведомления"
        recipeSystem -> authSystem "Аутентифицирует пользователей через соцсети"
    }
    
    views {
        # System Context view - общий взгляд на систему
        systemContext recipeSystem {
            include *
            autoLayout
        }
        
        # Container view - внутренняя структура
        container recipeSystem {
            include *
            autoLayout
        }
        
        # Dynamic view для сценария "Создание рецепта" (самый частый сценарий)
        dynamic recipeSystem "CreateRecipe" "Создание нового рецепта с ингредиентами" {
            autoLayout lr
            
            user -> recipeSystem.webApp "Заполняет форму рецепта (название, описание)"
            recipeSystem.webApp -> recipeSystem.apiGateway "POST /api/recipes (отправка данных)"
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
            
            # Завершение создания рецепта (обновление кэша и ответ)
            recipeSystem.recipeService -> recipeSystem.cache "Инвалидирует кэш (удаляет старые данные)"
            recipeSystem.apiGateway -> recipeSystem.webApp "Возвращает результат (успех/ошибка)"
            recipeSystem.webApp -> user "Отображает созданный рецепт с ингредиентами"
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
