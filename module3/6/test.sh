#!/bin/bash
echo "=== Тестирование Задания 6 (UDP Broadcast) ==="
echo "Запустите этот скрипт в двух разных терминалах для проверки связи."

# Запуск клиента
./task6 &
CLIENT_PID=$!

echo "Клиент запущен с PID $CLIENT_PID. Подождите 5 секунд..."
sleep 5

# Завершение
kill -SIGINT $CLIENT_PID
wait $CLIENT_PID 2>/dev/null
echo "=== Тест завершен ==="