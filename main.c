#include <stdio.h>
#include <string.h>
#include <locale.h>

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

void clearInput() {
    while (getchar() != '\n');
}

void readLine(const char* msg, char* s) {
    printf("%s", msg);
    fgets(s, MAX_STR, stdin);
    s[strcspn(s, "\n")] = 0;
}

void inputMonoblock(Monoblock* m) {
    readLine("Производитель: ", m->manufacturer);
    readLine("Операционная система: ", m->os);
    readLine("Процессор: ", m->cpu);
    readLine("Видеокарта: ", m->gpu);

    while (1) {
        printf("ОЗУ (ГБ): ");
        if (scanf("%d", &m->ramGB) != 1) {
            printf("Ошибка: введите целое число.\n");
            clearInput();
            continue;
        }
        clearInput();
        if (m->ramGB <= 0) {
            printf("Ошибка: ОЗУ должно быть > 0.\n");
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
            printf("Ошибка: допустимы только A, B или C.\n");
            continue;
        }
        break;
    }
}

void printTableHeader() {
    printf("\n");
    printf("========================================================================================================================================================================================\n");
    printf("| %-4s | %-30s | %-25s | %-30s | %-28s | %-6s | %-6s | %-8s | %-10s | %-6s |\n",
        "№",
        "Производитель",
        "ОС",
        "Процессор",
        "Видеокарта",
        "RAM",
        "SSD",
        "Диаг.",
        "Цена",
        "Кл.");
    printf("========================================================================================================================================================================================\n");
}

void printOne(const Monoblock* m, int i) {
    printf("| %-4d | %-30s | %-25s | %-30s | %-28s | %-6d | %-6u | %-8.1f | %-10.2f | %-6c |\n",
        i + 1,
        m->manufacturer,
        m->os,
        m->cpu,
        m->gpu,
        m->ramGB,
        m->ssdGB,
        m->screenDiagonal,
        m->price,
        m->energyClass);
}

void createRecord() {
    if (dbSize >= MAX_REC) {
        printf("Достигнут лимит записей.\n");
        return;
    }
    printf("\n--- Новая запись ---\n");
    inputMonoblock(&db[dbSize]);
    dbSize++;
}

void searchRecords() {
    char os[MAX_STR];
    int ram;

    readLine("ОС для поиска (пусто — не учитывать): ", os);
    printf("ОЗУ (0 — не учитывать): ");
    scanf("%d", &ram);
    clearInput();

    int found = 0;
    printTableHeader();

    for (int i = 0; i < dbSize; i++) {
        int ok = 1;

        if (strlen(os) > 0 && strcmp(db[i].os, os) != 0)
            ok = 0;
        if (ram > 0 && db[i].ramGB != ram)
            ok = 0;

        if (ok) {
            printOne(&db[i], i);
            found = 1;
        }
    }

    if (!found)
        printf("| %-105s |\n", "Записей не найдено.");
}

void sortAndPrint() {
    int field;
    printf("\nСортировка по полю:\n");
    printf("1 - Производитель\n");
    printf("2 - ОС\n");
    printf("3 - ОЗУ\n");
    printf("4 - Диагональ\n");
    printf("5 - Цена\n");
    printf("Выбор: ");
    scanf("%d", &field);
    clearInput();

    for (int i = 0; i < dbSize - 1; i++) {
        for (int j = 0; j < dbSize - 1 - i; j++) {
            int swap = 0;

            if (field == 1)
                swap = strcmp(db[j].manufacturer, db[j + 1].manufacturer) > 0;
            else if (field == 2)
                swap = strcmp(db[j].os, db[j + 1].os) > 0;
            else if (field == 3)
                swap = db[j].ramGB > db[j + 1].ramGB;
            else if (field == 4)
                swap = db[j].screenDiagonal > db[j + 1].screenDiagonal;
            else if (field == 5)
                swap = db[j].price > db[j + 1].price;

            if (swap) {
                Monoblock tmp = db[j];
                db[j] = db[j + 1];
                db[j + 1] = tmp;
            }
        }
    }

    printTableHeader();
    for (int i = 0; i < dbSize; i++)
        printOne(&db[i], i);
}

void saveToFile() {
    FILE* f = fopen(FILE_NAME, "w");
    if (!f) {
        printf("Ошибка записи файла.\n");
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
            db[i].energyClass);
    }

    fclose(f);
    printf("Данные сохранены в файл.\n");
}

void loadFromFile() {
    FILE* f = fopen(FILE_NAME, "r");
    if (!f) {
        printf("Файл не найден.\n");
        return;
    }

    dbSize = 0;
    while (dbSize < MAX_REC) {
        Monoblock* m = &db[dbSize];
        int r = fscanf(f,
            "%127[^|]|%127[^|]|%127[^|]|%127[^|]|%d|%u|%lf|%f|%c\n",
            m->manufacturer,
            m->os,
            m->cpu,
            m->gpu,
            &m->ramGB,
            &m->ssdGB,
            &m->screenDiagonal,
            &m->price,
            &m->energyClass);

        if (r != 9) break;
        dbSize++;
    }

    fclose(f);
    printf("Загружено записей: %d\n", dbSize);
}

void editRecord() {
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

void menu() {
    printf("1 - Добавить запись\n");
    printf("2 - Поиск\n");
    printf("3 - Сортировка и печать\n");
    printf("4 - Сохранить в файл\n");
    printf("5 - Загрузить из файла\n");
    printf("6 - Редактировать запись\n");
    printf("7 - Печать всех записей\n");
    printf("0 - Выход\n");
    printf("Выбор: ");
}

int main() {
    setlocale(LC_CTYPE, "RUS");

    int cmd;
    while (1) {
        menu();
        scanf("%d", &cmd);
        clearInput();

        if (cmd == 1) createRecord();
        else if (cmd == 2) searchRecords();
        else if (cmd == 3) sortAndPrint();
        else if (cmd == 4) saveToFile();
        else if (cmd == 5) loadFromFile();
        else if (cmd == 6) editRecord();
        else if (cmd == 7) {
            printTableHeader();
            for (int i = 0; i < dbSize; i++)
                printOne(&db[i], i);
        }
        else if (cmd == 0)
            break;
    }

    return 0;
}
