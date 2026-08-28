#!/bin/bash
echo "=== Тестирование Задания 5 (ShMem POSIX) ==="

# Инициализация
./task5 init

# Запуск производителя
./task5 prod &
PROD_PID=$!

# Запуск потребителя
echo "--- Вывод потребителя ---"
./task5 cons

wait $PROD_PID

# Очистка
./task5 clean
echo "=== Тест завершен ==="