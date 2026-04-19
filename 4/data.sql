-- Очистка таблиц (для повторного запуска)
TRUNCATE TABLE ingredients CASCADE;
TRUNCATE TABLE recipes CASCADE;
TRUNCATE TABLE users CASCADE;

-- Вставка пользователей
INSERT INTO users (id, login, password_hash, first_name, last_name) VALUES
('11111111-1111-1111-1111-111111111111', 'alice', 'hash123', 'Alice', 'Smith'),
('22222222-2222-2222-2222-222222222222', 'bob', 'hash456', 'Bob', 'Johnson'),
('33333333-3333-3333-3333-333333333333', 'carol', 'hash789', 'Carol', 'Williams'),
('44444444-4444-4444-4444-444444444444', 'dave', 'hashabc', 'Dave', 'Brown'),
('55555555-5555-5555-5555-555555555555', 'eve', 'hashdef', 'Eve', 'Jones'),
('66666666-6666-6666-6666-666666666666', 'frank', 'hashghi', 'Frank', 'Miller'),
('77777777-7777-7777-7777-777777777777', 'grace', 'hashjkl', 'Grace', 'Davis'),
('88888888-8888-8888-8888-888888888888', 'henry', 'hashmno', 'Henry', 'Garcia'),
('99999999-9999-9999-9999-999999999999', 'iris', 'hashpqr', 'Iris', 'Rodriguez'),
('aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa', 'jack', 'hashstu', 'Jack', 'Martinez');

-- Вставка рецептов
INSERT INTO recipes (id, title, description, author_id) VALUES
('aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee1', 'Pasta Carbonara', 'Classic Italian pasta dish', '11111111-1111-1111-1111-111111111111'),
('aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee2', 'Chicken Curry', 'Spicy Indian curry', '22222222-2222-2222-2222-222222222222'),
('aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee3', 'Caesar Salad', 'Fresh romaine salad', '33333333-3333-3333-3333-333333333333'),
('aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee4', 'Chocolate Cake', 'Rich chocolate dessert', '11111111-1111-1111-1111-111111111111'),
('aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee5', 'Vegetable Soup', 'Healthy vegetable soup', '44444444-4444-4444-4444-444444444444'),
('aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee6', 'Sushi Roll', 'Japanese sushi', '55555555-5555-5555-5555-555555555555'),
('aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee7', 'Tacos', 'Mexican tacos', '66666666-6666-6666-6666-666666666666'),
('aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee8', 'Omelette', 'French omelette', '77777777-7777-7777-7777-777777777777'),
('aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee9', 'Pancakes', 'Fluffy breakfast pancakes', '88888888-8888-8888-8888-888888888888'),
('aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeee10', 'Smoothie Bowl', 'Healthy breakfast bowl', '99999999-9999-9999-9999-999999999999'),
('aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeee11', 'Grilled Cheese', 'Classic grilled sandwich', 'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa');

-- Вставка ингредиентов
INSERT INTO ingredients (id, recipe_id, name, amount, unit) VALUES
('11111111-1111-1111-1111-111111111001', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee1', 'Spaghetti', '200', 'g'),
('11111111-1111-1111-1111-111111111002', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee1', 'Eggs', '2', 'pcs'),
('11111111-1111-1111-1111-111111111003', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee1', 'Pancetta', '100', 'g'),
('11111111-1111-1111-1111-111111111004', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee2', 'Chicken', '300', 'g'),
('11111111-1111-1111-1111-111111111005', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee2', 'Curry paste', '2', 'tbsp'),
('11111111-1111-1111-1111-111111111006', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee3', 'Lettuce', '1', 'head'),
('11111111-1111-1111-1111-111111111007', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee3', 'Croutons', '50', 'g'),
('11111111-1111-1111-1111-111111111008', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee4', 'Flour', '200', 'g'),
('11111111-1111-1111-1111-111111111009', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee4', 'Chocolate', '150', 'g'),
('11111111-1111-1111-1111-111111111010', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee5', 'Tomatoes', '400', 'g'),
('11111111-1111-1111-1111-111111111011', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee6', 'Rice', '250', 'g'),
('11111111-1111-1111-1111-111111111012', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee7', 'Tortilla', '4', 'pcs'),
('11111111-1111-1111-1111-111111111013', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee8', 'Eggs', '3', 'pcs'),
('11111111-1111-1111-1111-111111111014', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee9', 'Flour', '250', 'g'),
('11111111-1111-1111-1111-111111111015', 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeee10', 'Banana', '1', 'pcs');
