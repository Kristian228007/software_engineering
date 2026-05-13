#include "RecipeService.hpp"
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>

RecipeService::RecipeService(RecipeRepository &r, UserRepository &u, CacheService &c)
    : recipeRepo(r), userRepo(u), cache(c), producer(nullptr)
{
}

void RecipeService::setProducer(std::shared_ptr<RecipeProducer> p)
{
    producer = p;
}

Recipe RecipeService::createRecipe(const std::string authorId, const CreateRecipeRequest &dto)
{
    auto existing = recipeRepo.findByTitle(dto.title);
    if (!existing.empty())
        throw ConflictException("Recipe with this title already exists");

    Recipe r;
    r.id = Poco::UUIDGenerator::defaultGenerator().createRandom().toString();
    r.title = dto.title;
    r.description = dto.description;
    r.authorId = authorId;

    // Сохраняем в Redis (сквозная запись - cache-aside pattern)
    cache.set("recipe:" + r.id + ":pending", JsonUtils::recipeToJson(r), 300);

    // Отправляем сообщение в Kafka
    if (producer)
    {
        producer->produceRecipeCreated(r.id, r.title, r.description, r.authorId);
    }

    // Инвалидируем кэш списка рецептов
    cache.del("recipes:all");

    return r;
}

std::vector<Recipe> RecipeService::listRecipes()
{
    auto cached = cache.get("recipes:all");

    if (cached)
    {
        return JsonUtils::jsonToRecipes(*cached);
    }

    auto ptrs = recipeRepo.getAll();

    std::vector<Recipe> result;
    for (auto &p : ptrs)
        result.push_back(*p);

    cache.set("recipes:all", JsonUtils::recipesToJson(result), 60);

    return result;
}

std::vector<Recipe> RecipeService::searchRecipes(const std::string &title)
{
    auto ptrs = recipeRepo.findByTitle(title);
    std::vector<Recipe> result;
    result.reserve(ptrs.size());

    for (auto &p : ptrs)
        result.push_back(*p);

    return result;
}

std::vector<Recipe> RecipeService::getRecipesByUserId(const std::string &userId)
{
    // Проверяем существование пользователя
    auto user = userRepo.findByLogin(userId);
    if (!user)
    {
        // Пробуем найти по ID, если не нашли по логину
        auto ptrs = recipeRepo.findByAuthorId(userId);
        std::vector<Recipe> result;
        result.reserve(ptrs.size());
        for (auto &p : ptrs)
            result.push_back(*p);
        return result;
    }

    auto ptrs = recipeRepo.findByAuthorId(user->id);
    std::vector<Recipe> result;
    result.reserve(ptrs.size());

    for (auto &p : ptrs)
        result.push_back(*p);

    return result;
}

Ingredient RecipeService::addIngredient(std::string recipe_id, const AddIngredientRequest &dto)
{
    // Проверяем существование рецепта
    auto recipe = recipeRepo.findById(recipe_id);
    if (!recipe)
        throw NotFoundException("Recipe not found");

    // Проверяем, не существует ли уже такой ингредиент
    if (recipeRepo.ingredientExists(recipe_id, dto.name))
        throw ConflictException("Ingredient already exists in this recipe");

    Ingredient i;
    i.id = Poco::UUIDGenerator::defaultGenerator().createRandom().toString();
    i.name = dto.name;
    i.amount = dto.amount;
    i.unit = dto.unit;

    // Отправляем сообщение в Kafka
    if (producer)
    {
        producer->produceIngredientAdded(recipe_id, i.id, i.name, i.amount, i.unit);
    }

    // Инвалидируем кэши
    cache.del("recipes:all");
    cache.del("recipe:" + recipe_id + ":ingredients");

    return i;
}

std::vector<Ingredient> RecipeService::getIngredients(std::string recipe_id)
{
    // Проверяем существование рецепта
    auto recipe = recipeRepo.findById(recipe_id);
    if (!recipe)
    {
        throw NotFoundException("Recipe not found");
    }

    std::string cacheKey = "recipe:" + recipe_id + ":ingredients";

    // Пытаемся получить из кэша
    auto cached = cache.get(cacheKey);
    if (cached)
    {
        return JsonUtils::jsonToIngredients(*cached);
    }

    // Если нет в кэше, получаем из БД
    auto ingredients = recipeRepo.findIngredientsByRecipeId(recipe_id);

    // Сохраняем в кэш
    cache.set(cacheKey, JsonUtils::ingredientsToJson(ingredients), 60);

    return ingredients;
}

Recipe RecipeService::getRecipeById(const std::string &recipeId)
{
    std::string cacheKey = "recipe:" + recipeId;

    // Пытаемся получить из кэша
    auto cached = cache.get(cacheKey);
    if (cached)
    {
        return JsonUtils::jsonToRecipe(*cached);
    }

    // Если нет в кэше, получаем из БД
    auto recipe = recipeRepo.findById(recipeId);
    if (!recipe)
    {
        throw NotFoundException("Recipe not found");
    }

    // Сохраняем в кэш
    cache.set(cacheKey, JsonUtils::recipeToJson(*recipe), 60);

    return *recipe;
}

void RecipeService::updateRecipe(const std::string &recipeId, const CreateRecipeRequest &dto)
{
    auto recipe = recipeRepo.findById(recipeId);
    if (!recipe)
    {
        throw NotFoundException("Recipe not found");
    }

    // Проверяем уникальность названия
    auto existing = recipeRepo.findByTitle(dto.title);
    for (const auto &r : existing)
    {
        if (r->id != recipeId)
        {
            throw ConflictException("Recipe with this title already exists");
        }
    }

    // Отправляем событие обновления в Kafka
    if (producer)
    {
        Poco::JSON::Object message;
        message.set("event_type", "RECIPE_UPDATED");
        message.set("recipe_id", recipeId);
        message.set("title", dto.title);
        message.set("description", dto.description);
        message.set("timestamp", Poco::Timestamp().epochTime());

        std::ostringstream oss;
        Poco::JSON::Stringifier::stringify(message, oss);
        producer->produceRecipeUpdated(recipeId, dto.title, dto.description);
    }

    // Инвалидируем кэши
    cache.del("recipes:all");
    cache.del("recipe:" + recipeId);
}

void RecipeService::deleteRecipe(const std::string &recipeId)
{
    auto recipe = recipeRepo.findById(recipeId);
    if (!recipe)
    {
        throw NotFoundException("Recipe not found");
    }

    // Отправляем событие удаления в Kafka
    if (producer)
    {
        producer->produceRecipeDeleted(recipeId);
    }

    // Инвалидируем кэши
    cache.del("recipes:all");
    cache.del("recipe:" + recipeId);
    cache.del("recipe:" + recipeId + ":ingredients");
}

void RecipeService::deleteIngredient(const std::string &recipeId, const std::string &ingredientId)
{
    auto recipe = recipeRepo.findById(recipeId);
    if (!recipe)
    {
        throw NotFoundException("Recipe not found");
    }

    auto ingredients = recipeRepo.findIngredientsByRecipeId(recipeId);
    bool found = false;
    for (const auto &ing : ingredients)
    {
        if (ing.id == ingredientId)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        throw NotFoundException("Ingredient not found");
    }

    // Отправляем событие удаления в Kafka
    if (producer)
    {
        producer->produceIngredientDeleted(recipeId, ingredientId);
    }

    // Инвалидируем кэши
    cache.del("recipe:" + recipeId + ":ingredients");
    cache.del("recipes:all");
}

Ingredient RecipeService::updateIngredient(const std::string &recipeId,
                                           const std::string &ingredientId,
                                           const AddIngredientRequest &dto)
{
    auto recipe = recipeRepo.findById(recipeId);
    if (!recipe)
    {
        throw NotFoundException("Recipe not found");
    }

    auto ingredients = recipeRepo.findIngredientsByRecipeId(recipeId);
    bool found = false;
    for (const auto &ing : ingredients)
    {
        if (ing.id == ingredientId)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        throw NotFoundException("Ingredient not found");
    }

    // Проверяем уникальность имени ингредиента в рамках рецепта
    if (recipeRepo.ingredientExists(recipeId, dto.name) && dto.name != "")
    {
        // Check if the existing ingredient with same name is not the one we're updating
        auto ingredients = recipeRepo.findIngredientsByRecipeId(recipeId);
        for (const auto &ing : ingredients)
        {
            if (ing.name == dto.name && ing.id != ingredientId)
            {
                throw ConflictException("Ingredient with this name already exists in recipe");
            }
        }
    }

    Ingredient updatedIngredient;
    updatedIngredient.id = ingredientId;
    updatedIngredient.name = dto.name;
    updatedIngredient.amount = dto.amount;
    updatedIngredient.unit = dto.unit;

    // Отправляем событие обновления в Kafka
    if (producer)
    {
        producer->produceIngredientUpdated(recipeId, ingredientId, dto.name, dto.amount, dto.unit);
    }

    // Инвалидируем кэши
    cache.del("recipe:" + recipeId + ":ingredients");
    cache.del("recipes:all");

    return updatedIngredient;
}

std::vector<Recipe> RecipeService::searchRecipesByIngredients(const std::vector<std::string> &ingredientNames)
{
    // Get all recipes and filter by ingredients (simple implementation)
    auto allRecipes = recipeRepo.getAll();
    std::vector<Recipe> result;

    for (const auto &ptr : allRecipes)
    {
        for (const auto &ingName : ingredientNames)
        {
            for (const auto &ing : ptr->ingredients)
            {
                if (ing.name == ingName)
                {
                    result.push_back(*ptr);
                    break;
                }
            }
        }
    }

    return result;
}