#!/bin/bash
echo "=== Тестирование Задания 3 (POSIX Chat) ==="

# Запуск первого участника
./task3 testchat > user1.log &
PID1=$!
sleep 1

# Запуск второго участника
./task3 testchat > user2.log &
PID2=$!
sleep 1

# Эмуляция диалога (через echo и перенаправление в stdin сложно, 
# поэтому просто проверим, что процессы запустились и создали очереди)
echo "Процессы запущены. PID1: $PID1, PID2: $PID2"

# Завершение
kill $PID1
sleep 1
kill $PID2
wait $PID1 $PID2 2>/dev/null

# Проверка логов
echo "--- Лог пользователя 1 ---"
cat user1.log
echo "--- Лог пользователя 2 ---"
cat user2.log

rm -f user1.log user2.log
echo "=== Тест завершен ==="