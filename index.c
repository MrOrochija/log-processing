#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Использование: %s <лог_файл> <выход_файл>\n", argv[0]);
        return 1;
    }

    const char *pattern = "[ERROR]"; // Что ищем
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) { perror("Ошибка открытия файла"); return 1; }

    // Получаем размер файла
    struct stat st;
    fstat(fd, &st);
    size_t size = st.st_size;

    // "Отображаем" файл в память
    char *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) { perror("mmap"); return 1; }

    FILE *out = fopen(argv[2], "w");
    long count = 0;
    char *ptr = data;
    char *end = data + size;

    // Основной цикл поиска
    while (ptr < end) {
        char *line_end = memchr(ptr, '\n', end - ptr);
        if (!line_end) line_end = end;

        size_t line_len = line_end - ptr;
        
        // Быстрый поиск подстроки в текущей строке
        if (memmem(ptr, line_len, pattern, strlen(pattern))) {
            fwrite(ptr, 1, line_len + (line_end < end), out);
            count++;
        }

        ptr = line_end + 1;
    }

    printf("Найдено строк ERROR: %ld\n", count);

    // Чистим за собой
    fclose(out);
    munmap(data, size);
    close(fd);

    return 0;
}