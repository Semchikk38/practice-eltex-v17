#!/bin/bash
echo "=== Тестирование Задания 8 (RAW Sockets) ==="
echo "Внимание: Требуется sudo!"

# Запуск сниффера на 5 секунд с фильтром DNS
sudo timeout 5 ./task8 2 > sniff_output.txt

echo "--- Результаты захвата ---"
cat sniff_output.txt

rm -f sniff_output.txt
echo "=== Тест завершен ==="