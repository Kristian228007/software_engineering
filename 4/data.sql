-- Очистка таблиц (для повторного запуска)
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
