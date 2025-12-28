#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_REC 100
#define MAX_STR 128
#define FILE_NAME "monoblocks.txt"

typedef struct {
    char manufacturer[MAX_STR];
    char os[MAX_STR];
    char cpu[MAX_STR];
    char gpu[MAX_STR];

    int ramGB;
    unsigned int ssdGB;
    double screenDiagonal;
    float price;
    char energyClass;
} Monoblock;

Monoblock db[MAX_REC];
int dbSize = 0;

static void clearInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static void readLine(const char* msg, char* s) {
    printf("%s", msg);
    if (!fgets(s, MAX_STR, stdin)) {
        s[0] = '\0';
        return;
    }
    s[strcspn(s, "\r\n")] = '\0';
}

static void stripUtf8Bom(char* s) {
    if (!s) return;
    unsigned char* p = (unsigned char*)s;
    if (p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) {
        memmove(s, s + 3, strlen(s + 3) + 1);
    }
}

static int cmpIgnoreCase(const char* a, const char* b) {
    unsigned char ca, cb;
    while (*a && *b) {
        ca = (unsigned char)tolower((unsigned char)*a);
        cb = (unsigned char)tolower((unsigned char)*b);
        if (ca != cb) return (int)ca - (int)cb;
        a++; b++;
    }
    return (int)tolower((unsigned char)*a) - (int)tolower((unsigned char)*b);
}

static void setStr(char* dst, const char* src) {
    if (!src) src = "";
    strncpy(dst, src, MAX_STR - 1);
    dst[MAX_STR - 1] = '\0';
}

static int parseIntOr0(const char* s, int* out) {
    if (!s || !*s) { *out = 0; return 1; }
    char* end = NULL;
    long v = strtol(s, &end, 10);
    if (end == s) { *out = 0; return 0; }
    *out = (int)v;
    return 1;
}

static int parseUIntOr0(const char* s, unsigned int* out) {
    if (!s || !*s) { *out = 0u; return 1; }
    char* end = NULL;
    unsigned long v = strtoul(s, &end, 10);
    if (end == s) { *out = 0u; return 0; }
    *out = (unsigned int)v;
    return 1;
}

static int parseDoubleOr0(const char* s, double* out) {
    if (!s || !*s) { *out = 0.0; return 1; }
    char* end = NULL;
    double v = strtod(s, &end);
    if (end == s) { *out = 0.0; return 0; }
    *out = v;
    return 1;
}

static int parseFloatOr0(const char* s, float* out) {
    if (!s || !*s) { *out = 0.0f; return 1; }
    char* end = NULL;
    float v = strtof(s, &end);
    if (end == s) { *out = 0.0f; return 0; }
    *out = v;
    return 1;
}

static int splitPipePreserveEmpty(char* line, char* out[], int expectedCount) {
    int n = 0;
    char* p = line;

    out[n++] = p;
    while (*p && n < expectedCount) {
        if (*p == '|') {
            *p = '\0';
            out[n++] = p + 1;
        }
        p++;
    }

    while (n < expectedCount) {
        out[n++] = (char*)"";
    }

    return n;
}

static void inputMonoblock(Monoblock* m) {
    readLine("Производитель: ", m->manufacturer);
    readLine("Операционная система: ", m->os);
    readLine("Процессор: ", m->cpu);
    readLine("Видеокарта: ", m->gpu);

    while (1) {
        printf("RAM (ГБ): ");
        if (scanf("%d", &m->ramGB) != 1) {
            printf("Ошибка: введите целое число.\n");
            clearInput();
            continue;
        }
        clearInput();
        if (m->ramGB <= 0) {
            printf("Ошибка: RAM должна быть > 0.\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("SSD (ГБ): ");
        if (scanf("%u", &m->ssdGB) != 1) {
            printf("Ошибка: введите целое число.\n");
            clearInput();
            continue;
        }
        clearInput();
        if (m->ssdGB == 0) {
            printf("Ошибка: SSD должен быть > 0.\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("Диагональ экрана: ");
        if (scanf("%lf", &m->screenDiagonal) != 1) {
            printf("Ошибка: введите число.\n");
            clearInput();
            continue;
        }
        clearInput();
        if (m->screenDiagonal <= 0) {
            printf("Ошибка: диагональ должна быть > 0.\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("Цена: ");
        if (scanf("%f", &m->price) != 1) {
            printf("Ошибка: введите число.\n");
            clearInput();
            continue;
        }
        clearInput();
        if (m->price < 0) {
            printf("Ошибка: цена не может быть отрицательной.\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("Класс энергопотребления (A/B/C): ");
        if (scanf(" %c", &m->energyClass) != 1) {
            printf("Ошибка ввода.\n");
            clearInput();
            continue;
        }
        clearInput();
        if (m->energyClass != 'A' && m->energyClass != 'B' && m->energyClass != 'C') {
            printf("Ошибка: допустимо A, B или C.\n");
            continue;
        }
        break;
    }
}

static void printTableHeader(void) {
    printf("\n");
    printf("========================================================================================================================================================================================\n");
    printf("| %-4s | %-30s | %-25s | %-30s | %-28s | %-6s | %-6s | %-8s | %-10s | %-6s |\n",
        "№", "Производитель", "ОС", "Процессор", "Видеокарта", "RAM", "SSD", "Диаг.", "Цена", "Кл.");
    printf("========================================================================================================================================================================================\n");
}

static void printOne(const Monoblock* m, int i) {
    printf("| %-4d | %-30s | %-25s | %-30s | %-28s | %-6d | %-6u | %-8.1f | %-10.2f | %-6c |\n",
        i + 1, m->manufacturer, m->os, m->cpu, m->gpu, m->ramGB, m->ssdGB,
        m->screenDiagonal, m->price, m->energyClass ? m->energyClass : '?');
}

static void createRecord(void) {
    if (dbSize >= MAX_REC) {
        printf("Достигнут лимит записей.\n");
        return;
    }
    printf("\n--- Новая запись ---\n");
    inputMonoblock(&db[dbSize]);
    dbSize++;
}

static void createMultipleRecords(void) {
    int n;
    printf("Сколько записей добавить? ");
    if (scanf("%d", &n) != 1) {
        printf("Ошибка: введите целое число.\n");
        clearInput();
        return;
    }
    clearInput();

    if (n <= 0) {
        printf("Нужно число > 0.\n");
        return;
    }

    for (int i = 0; i < n; i++) {
        if (dbSize >= MAX_REC) {
            printf("Достигнут лимит записей (%d). Остальные не добавлены.\n", MAX_REC);
            break;
        }
        printf("\n[%d/%d]\n", i + 1, n);
        inputMonoblock(&db[dbSize]);
        dbSize++;
    }
}

static void searchRecords(void) {
    char os[MAX_STR];
    int ram;

    readLine("ОС (пусто — не учитывать): ", os);
    printf("RAM (0 — не учитывать): ");
    scanf("%d", &ram);
    clearInput();

    int found = 0;
    printTableHeader();

    for (int i = 0; i < dbSize; i++) {
        int ok = 1;

        if (strlen(os) > 0 && strcmp(db[i].os, os) != 0) ok = 0;
        if (ram > 0 && db[i].ramGB != ram) ok = 0;

        if (ok) {
            printOne(&db[i], i);
            found = 1;
        }
    }

    if (!found) {
        printf("| %-105s |\n", "Записи не найдены.");
    }
}

static void sortAndPrint(void) {
    int field;
    printf("\nСортировка по полю:\n");
    printf("1 - Производитель (без учета регистра)\n");
    printf("2 - ОС\n");
    printf("3 - RAM\n");
    printf("4 - Диагональ\n");
    printf("5 - Цена\n");
    printf("Выбор: ");
    scanf("%d", &field);
    clearInput();

    for (int i = 0; i < dbSize - 1; i++) {
        for (int j = 0; j < dbSize - 1 - i; j++) {
            int swap = 0;

            if (field == 1) swap = (cmpIgnoreCase(db[j].manufacturer, db[j + 1].manufacturer) > 0);
            else if (field == 2) swap = (cmpIgnoreCase(db[j].os, db[j + 1].os) > 0);
            else if (field == 3) swap = (db[j].ramGB > db[j + 1].ramGB);
            else if (field == 4) swap = (db[j].screenDiagonal > db[j + 1].screenDiagonal);
            else if (field == 5) swap = (db[j].price > db[j + 1].price);

            if (swap) {
                Monoblock tmp = db[j];
                db[j] = db[j + 1];
                db[j + 1] = tmp;
            }
        }
    }

    printTableHeader();
    for (int i = 0; i < dbSize; i++) printOne(&db[i], i);
}

static void saveToFile(void) {
    FILE* f = fopen(FILE_NAME, "w");
    if (!f) {
        printf("Ошибка открытия файла.\n");
        return;
    }

    for (int i = 0; i < dbSize; i++) {
        fprintf(f, "%s|%s|%s|%s|%d|%u|%.1f|%.2f|%c\n",
            db[i].manufacturer,
            db[i].os,
            db[i].cpu,
            db[i].gpu,
            db[i].ramGB,
            db[i].ssdGB,
            db[i].screenDiagonal,
            db[i].price,
            db[i].energyClass ? db[i].energyClass : '?');
    }

    fclose(f);
    printf("Данные сохранены в файл.\n");
}

static void loadFromFile(void) {
    FILE* f = fopen(FILE_NAME, "r");
    if (!f) {
        printf("Файл не найден.\n");
        return;
    }

    dbSize = 0;

    char line[1024];
    while (dbSize < MAX_REC && fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';

        if (line[0] == '\0') continue;

        char* fields[9];
        splitPipePreserveEmpty(line, fields, 9);

        stripUtf8Bom(fields[0]);

        Monoblock* m = &db[dbSize];

        setStr(m->manufacturer, fields[0]);
        setStr(m->os, fields[1]);
        setStr(m->cpu, fields[2]);
        setStr(m->gpu, fields[3]);

        parseIntOr0(fields[4], &m->ramGB);
        parseUIntOr0(fields[5], &m->ssdGB);
        parseDoubleOr0(fields[6], &m->screenDiagonal);
        parseFloatOr0(fields[7], &m->price);

        if (fields[8] && fields[8][0]) m->energyClass = fields[8][0];
        else m->energyClass = '?';

        dbSize++;
    }

    fclose(f);
    printf("Загружено записей: %d\n", dbSize);
}

static void editRecord(void) {
    int idx;
    printf("Номер записи для редактирования: ");
    scanf("%d", &idx);
    clearInput();

    if (idx < 1 || idx > dbSize) {
        printf("Неверный номер.\n");
        return;
    }

    inputMonoblock(&db[idx - 1]);
}

static void printAll(void) {
    printTableHeader();
    for (int i = 0; i < dbSize; i++) printOne(&db[i], i);
}

static void menu(void) {
    printf("\n");
    printf("1 - Добавить запись\n");
    printf("2 - Поиск\n");
    printf("3 - Сортировка и вывод\n");
    printf("4 - Сохранить в файл\n");
    printf("5 - Загрузить из файла\n");
    printf("6 - Редактировать запись\n");
    printf("7 - Показать все записи\n");
    printf("8 - Добавить несколько записей\n");
    printf("0 - Выход\n");
    printf("Выбор: ");
}

int main(void) {
    setlocale(LC_ALL, "");

    int cmd;
    while (1) {
        menu();
        if (scanf("%d", &cmd) != 1) {
            printf("Ошибка: введите число.\n");
            clearInput();
            continue;
        }
        clearInput();

        if (cmd == 1) createRecord();
        else if (cmd == 2) searchRecords();
        else if (cmd == 3) sortAndPrint();
        else if (cmd == 4) saveToFile();
        else if (cmd == 5) loadFromFile();
        else if (cmd == 6) editRecord();
        else if (cmd == 7) printAll();
        else if (cmd == 8) createMultipleRecords();
        else if (cmd == 0) break;
    }

    return 0;
}
