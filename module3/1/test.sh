#!/bin/bash
echo "=== Тестирование Задания 1 (Копирование файлов) ==="

# Подготовка
echo "Test Data 1" > file1.txt
echo "Test Data 2" > file2.txt

# Тест 1: Обычное копирование
echo "--- Тест 1: Неименованный канал ---"
./task1 file1.txt file2.txt
if [ -f "file1.txt.copy" ] && [ -f "file2.txt.copy" ]; then
    echo "Успех: Файлы .copy созданы."
else
    echo "Ошибка: Файлы не созданы."
fi

# Тест 2: Именованный канал
echo "--- Тест 2: Именованный канал (-p) ---"
./task1 -p my_fifo file1.txt
if [ -f "file1.txt.copy" ]; then
    echo "Успех: Копирование через FIFO прошло."
fi

# Очистка
rm -f file1.txt file2.txt *.copy my_fifo
echo "=== Тест завершен ==="