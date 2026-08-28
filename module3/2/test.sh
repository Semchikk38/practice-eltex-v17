#!/bin/bash
echo "=== Тестирование Задания 2 (MsgQ System V) ==="

# Запуск брокера в фоне
./task2 -b &
BROKER_PID=$!
sleep 1

# Запуск подписчика в фоне
./task2 -s news sport > sub_output.txt &
SUB_PID=$!
sleep 1

# Отправка сообщений
echo "--- Отправка сообщений ---"
./task2 -p news "News Item 1"
./task2 -p sport "Goal!"
./task2 -p weather "Rain" # Не должно прийти подписчику

sleep 2

# Проверка результата
echo "--- Результат у подписчика ---"
cat sub_output.txt

# Завершение брокера (он сам убьет подписчика)
kill -SIGINT $BROKER_PID
wait $BROKER_PID 2>/dev/null

rm -f sub_output.txt
echo "=== Тест завершен ==="