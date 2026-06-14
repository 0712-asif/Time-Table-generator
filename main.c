/*
 * ============================================================
 *   WEEKLY TIMETABLE GENERATOR FOR N CLASSES
 *   Using Backtracking Algorithm (DAA Mini Project)
 *   with truecolor Console-based Text User Interface (TUI)
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

/* ──────────────── Configuration Limits ──────────────── */

#define MAX_CLASSES    10
#define MAX_DAYS        6
#define TOTAL_SLOTS     9     /* Fixed: 7 periods + 2 breaks     */
#define MAX_SUBJECTS   20
#define SUBJ_NAME_LEN  30    /* Max subject name length          */
#define FREE_PER_WEEK   3    /* Guaranteed free periods/class    */

/* ──────────────── MySQL Configuration ──────────────── */

#define DB_HOST   "localhost"
#define DB_NAME   "timetable_db"

static char db_user[64] = "root";
static char db_password[64] = "";

/* ──────────────── Special Slot Markers ──────────────── */

#define EMPTY         -1     /* Unassigned slot                  */
#define FREE_PERIOD   -2     /* Intentionally free               */
#define TEA_BREAK     -3     /* Tea break  (10:20 - 10:40)       */
#define LUNCH_BREAK   -4     /* Lunch break (12:30 - 01:30)      */

/* ──────────────── Fixed Slot Timings ──────────────── */

static const char *SLOT_START[] = {
    "08:30", "09:25", "10:20", "10:40", "11:35",
    "12:30", "01:30", "02:25", "03:20"
};
static const char *SLOT_END[] = {
    "09:25", "10:20", "10:40", "11:35", "12:30",
    "01:30", "02:25", "03:20", "04:15"
};

/* Break slot indices (0-indexed) */
#define TEA_SLOT    2
#define LUNCH_SLOT  5

/* ──────────────── Day Names ──────────────── */

static const char *DAY_NAMES[] = {
    "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

/* ──────────────── Data Structures ──────────────── */

typedef struct {
    char name[SUBJ_NAME_LEN];
    int  weekly_hours;      /* Total periods per week            */
    int  is_lab;            /* 1 = Lab (double slot), 0 = Theory */
    int  max_per_day;       /* Max periods of this subject/day   */
} Subject;

/* ──────────────── Global State ──────────────── */

static int     timetable[MAX_CLASSES][MAX_DAYS][TOTAL_SLOTS];
static Subject subjects[MAX_SUBJECTS];
static int     remaining[MAX_SUBJECTS];

static int     num_classes;
static int     num_days;
static int     num_subjects;

/* ──────────────── TUI Engine State ──────────────── */

typedef struct {
    char ch[4]; // UTF-8 char
    unsigned char fg_r, fg_g, fg_b;
    unsigned char bg_r, bg_g, bg_b;
} TuiCell;

#define TUI_WIDTH 120
#define TUI_HEIGHT 35

static TuiCell tui_buffer[TUI_HEIGHT][TUI_WIDTH];
static char tui_write_buffer[256 * 1024];

enum Screen {
    SCREEN_CONFIG,
    SCREEN_SUBJECTS,
    SCREEN_SOLVING,
    SCREEN_VIEWER
};
static enum Screen current_screen = SCREEN_CONFIG;

// Color Palette (Catppuccin Mocha themed)
static struct { unsigned char r, g, b; } SUBJECT_COLORS[] = {
    {224, 108, 117}, {152, 195, 121}, {229, 192, 123},
    {97, 175, 239},  {198, 120, 221}, {86, 182, 194},
    {209, 154, 102}, {171, 178, 191}, {244, 143, 177},
    {128, 203, 196}, {212, 225, 87},  {255, 171, 64},
    {141, 110, 99},  {144, 164, 174}, {121, 85, 72},
    {233, 30, 99},   {156, 39, 176},  {103, 58, 183},
    {63, 81, 181},   {0, 150, 136}
};

static char viewer_status[128] = "";
static unsigned char status_color_r = 166, status_color_g = 227, status_color_b = 161;

static HANDLE hConsoleOut;
static DWORD dwOldMode;
static UINT oldCodePage;
static CONSOLE_CURSOR_INFO oldCursorInfo;

/* ──────────────── TUI Engine Functions ──────────────── */

static void tui_clear() {
    for (int y = 0; y < TUI_HEIGHT; y++) {
        for (int x = 0; x < TUI_WIDTH; x++) {
            strcpy(tui_buffer[y][x].ch, " ");
            tui_buffer[y][x].fg_r = 205;
            tui_buffer[y][x].fg_g = 214;
            tui_buffer[y][x].fg_b = 244;
            tui_buffer[y][x].bg_r = 30;
            tui_buffer[y][x].bg_g = 30;
            tui_buffer[y][x].bg_b = 46;
        }
    }
}

static void tui_draw_cell_utf8(int x, int y, const char *utf8_char, unsigned char fr, unsigned char fg, unsigned char fb, unsigned char br, unsigned char bg, unsigned char bb) {
    if (x >= 0 && x < TUI_WIDTH && y >= 0 && y < TUI_HEIGHT) {
        int len = strlen(utf8_char);
        if (len < 4) {
            strcpy(tui_buffer[y][x].ch, utf8_char);
        } else {
            strncpy(tui_buffer[y][x].ch, utf8_char, 3);
            tui_buffer[y][x].ch[3] = '\0';
        }
        tui_buffer[y][x].fg_r = fr;
        tui_buffer[y][x].fg_g = fg;
        tui_buffer[y][x].fg_b = fb;
        tui_buffer[y][x].bg_r = br;
        tui_buffer[y][x].bg_g = bg;
        tui_buffer[y][x].bg_b = bb;
    }
}

static void tui_draw_char(int x, int y, char ch, unsigned char fr, unsigned char fg, unsigned char fb, unsigned char br, unsigned char bg, unsigned char bb) {
    char buf[2] = {ch, '\0'};
    tui_draw_cell_utf8(x, y, buf, fr, fg, fb, br, bg, bb);
}

static void tui_draw_str(int x, int y, const char *str, unsigned char fr, unsigned char fg, unsigned char fb, unsigned char br, unsigned char bg, unsigned char bb) {
    int i = 0;
    int curr_x = x;
    while (str[i] != '\0' && curr_x < TUI_WIDTH) {
        // UTF-8 lead byte detection
        unsigned char lead = (unsigned char)str[i];
        int char_len = 1;
        if (lead >= 0xF0) char_len = 4;
        else if (lead >= 0xE0) char_len = 3;
        else if (lead >= 0xC0) char_len = 2;

        char buf[5] = {0};
        for (int j = 0; j < char_len && str[i] != '\0'; j++) {
            buf[j] = str[i++];
        }
        tui_draw_cell_utf8(curr_x, y, buf, fr, fg, fb, br, bg, bb);
        curr_x++;
    }
}

static void tui_present() {
    char *p = tui_write_buffer;
    
    // Disable line wrap
    p += sprintf(p, "\x1b[7l");

    unsigned char cur_fg_r = 0, cur_fg_g = 0, cur_fg_b = 0;
    unsigned char cur_bg_r = 0, cur_bg_g = 0, cur_bg_b = 0;
    int first = 1;

    for (int y = 0; y < TUI_HEIGHT; y++) {
        // Move cursor to start of row y+1
        p += sprintf(p, "\x1b[%d;1H", y + 1);

        for (int x = 0; x < TUI_WIDTH; x++) {
            TuiCell cell = tui_buffer[y][x];

            if (first ||
                cell.fg_r != cur_fg_r || cell.fg_g != cur_fg_g || cell.fg_b != cur_fg_b ||
                cell.bg_r != cur_bg_r || cell.bg_g != cur_bg_g || cell.bg_b != cur_bg_b) {
                
                p += sprintf(p, "\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm",
                             cell.fg_r, cell.fg_g, cell.fg_b,
                             cell.bg_r, cell.bg_g, cell.bg_b);

                cur_fg_r = cell.fg_r;
                cur_fg_g = cell.fg_g;
                cur_fg_b = cell.fg_b;
                cur_bg_r = cell.bg_r;
                cur_bg_g = cell.bg_g;
                cur_bg_b = cell.bg_b;
                first = 0;
            }

            if (cell.ch[0] == '\0') {
                *p++ = ' ';
            } else {
                int idx = 0;
                while (cell.ch[idx] != '\0') {
                    *p++ = cell.ch[idx++];
                }
            }
        }
    }
    *p = '\0'; // Null-terminate buffer

    fputs(tui_write_buffer, stdout);
    fflush(stdout);
}

static void tui_init() {
    hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    
    oldCodePage = GetConsoleOutputCP();
    SetConsoleOutputCP(65001); // Set standard UTF-8 console output

    GetConsoleMode(hConsoleOut, &dwOldMode);
    DWORD dwNewMode = dwOldMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsoleOut, dwNewMode);

    GetConsoleCursorInfo(hConsoleOut, &oldCursorInfo);
    CONSOLE_CURSOR_INFO newCursorInfo = oldCursorInfo;
    newCursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsoleOut, &newCursorInfo);

    // Set console buffer size and window size safely
    SMALL_RECT tmp = { 0, 0, 1, 1 };
    SetConsoleWindowInfo(hConsoleOut, TRUE, &tmp);

    COORD bufferSize = { TUI_WIDTH, TUI_HEIGHT };
    SetConsoleScreenBufferSize(hConsoleOut, bufferSize);
    
    SMALL_RECT windowSize = { 0, 0, TUI_WIDTH - 1, TUI_HEIGHT - 1 };
    if (!SetConsoleWindowInfo(hConsoleOut, TRUE, &windowSize)) {
        COORD max_size = GetLargestConsoleWindowSize(hConsoleOut);
        SMALL_RECT fallbackSize = { 0, 0, 
            (short)(TUI_WIDTH < max_size.X ? TUI_WIDTH - 1 : max_size.X - 1), 
            (short)(TUI_HEIGHT < max_size.Y ? TUI_HEIGHT - 1 : max_size.Y - 1) 
        };
        SetConsoleWindowInfo(hConsoleOut, TRUE, &fallbackSize);
    }

    // Set size via ANSI sequence
    printf("\x1b[8;%d;%dt", TUI_HEIGHT, TUI_WIDTH);
    printf("\x1b[7l"); // Turn off auto-wrap
    printf("\x1b[2J\x1b[H\x1b[?25l"); // Hide cursor, clear screen
    
    tui_clear();
}

static void tui_shutdown() {
    printf("\x1b[0m\x1b[7h\x1b[2J\x1b[H\x1b[?25h"); // Show cursor, clear screen, reset attributes, restore wrap
    
    SetConsoleMode(hConsoleOut, dwOldMode);
    SetConsoleOutputCP(oldCodePage);
    SetConsoleCursorInfo(hConsoleOut, &oldCursorInfo);
}

/* ──────────────── Utility Solver Functions ──────────────── */

static int is_break_slot(int slot)
{
    if (slot == TEA_SLOT)   return TEA_BREAK;
    if (slot == LUNCH_SLOT) return LUNCH_BREAK;
    return 0;
}

static void shuffle(int arr[], int n)
{
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i]  = arr[j];
        arr[j]  = tmp;
    }
}

static int count_subject_in_day(int cls, int day, int sub)
{
    int count = 0;
    for (int s = 0; s < TOTAL_SLOTS; s++) {
        if (timetable[cls][day][s] == sub)
            count++;
    }
    return count;
}

static void place_free_periods(int cls)
{
    int positions[MAX_DAYS * TOTAL_SLOTS];
    int count = 0;

    for (int d = 0; d < num_days; d++) {
        for (int s = 0; s < TOTAL_SLOTS; s++) {
            if (!is_break_slot(s))
                positions[count++] = d * TOTAL_SLOTS + s;
        }
    }

    shuffle(positions, count);

    for (int i = 0; i < FREE_PER_WEEK && i < count; i++) {
        int d = positions[i] / TOTAL_SLOTS;
        int s = positions[i] % TOTAL_SLOTS;
        timetable[cls][d][s] = FREE_PERIOD;
    }
}

/* ──────────────── Backtracking Solver ──────────────── */

static int solve(int cls, int day, int slot)
{
    if (day >= num_days)
        return 1;

    if (slot >= TOTAL_SLOTS)
        return solve(cls, day + 1, 0);

    if (timetable[cls][day][slot] != EMPTY)
        return solve(cls, day, slot + 1);

    int order[MAX_SUBJECTS];
    for (int i = 0; i < num_subjects; i++)
        order[i] = i;
    shuffle(order, num_subjects);

    for (int i = 0; i < num_subjects; i++) {
        int sub = order[i];

        if (remaining[sub] <= 0)
            continue;

        if (!subjects[sub].is_lab && slot > 0) {
            int prev = slot - 1;
            if (is_break_slot(prev) && prev > 0)
                prev--;
            if (prev >= 0 && timetable[cls][day][prev] == sub)
                continue;
        }

        if (count_subject_in_day(cls, day, sub) >= subjects[sub].max_per_day)
            continue;

        if (subjects[sub].is_lab) {
            int next = slot + 1;
            if (next >= TOTAL_SLOTS)                continue;
            if (is_break_slot(next))                continue;
            if (timetable[cls][day][next] != EMPTY) continue;
            if (remaining[sub] < 2)                 continue;

            timetable[cls][day][slot] = sub;
            timetable[cls][day][next] = sub;
            remaining[sub] -= 2;

            if (solve(cls, day, slot + 2))
                return 1;

            timetable[cls][day][slot] = EMPTY;
            timetable[cls][day][next] = EMPTY;
            remaining[sub] += 2;
        } else {
            timetable[cls][day][slot] = sub;
            remaining[sub]--;

            if (solve(cls, day, slot + 1))
                return 1;

            timetable[cls][day][slot] = EMPTY;
            remaining[sub]++;
        }
    }

    /* No subject fits -> mark as FREE */
    timetable[cls][day][slot] = FREE_PERIOD;
    if (solve(cls, day, slot + 1))
        return 1;
    timetable[cls][day][slot] = EMPTY;

    return 0;
}

/* ──────────────── Display / File Export Functions ──────────────── */

static const char *get_slot_label(int val)
{
    switch (val) {
        case TEA_BREAK:   return "TEA BREAK";
        case LUNCH_BREAK: return "LUNCH";
        case FREE_PERIOD: return "FREE";
        case EMPTY:       return "---";
        default:
            if (val >= 0 && val < num_subjects)
                return subjects[val].name;
            return "???";
    }
}

static void print_line(FILE *out, int cols)
{
    fprintf(out, "  +------------+");
    for (int i = 0; i < cols; i++)
        fprintf(out, "--------------+");
    fprintf(out, "\n");
}

static void print_timetable(FILE *out, int cls)
{
    fprintf(out, "\n");
    fprintf(out, "  ========================================================\n");
    fprintf(out, "      TIMETABLE FOR CLASS %d\n", cls + 1);
    fprintf(out, "  ========================================================\n\n");

    print_line(out, TOTAL_SLOTS);

    fprintf(out, "  | %-10s |", "Time");
    for (int s = 0; s < TOTAL_SLOTS; s++) {
        char timebuf[14];
        sprintf(timebuf, "%s-%s", SLOT_START[s], SLOT_END[s]);
        fprintf(out, " %-12s |", timebuf);
    }
    fprintf(out, "\n");

    print_line(out, TOTAL_SLOTS);

    fprintf(out, "  | %-10s |", "");
    int pnum = 1;
    for (int s = 0; s < TOTAL_SLOTS; s++) {
        if (is_break_slot(s)) {
            fprintf(out, " %-12s |", s == TEA_SLOT ? "TEA BREAK" : "LUNCH");
        } else {
            char pbuf[14];
            sprintf(pbuf, "Period %d", pnum++);
            fprintf(out, " %-12s |", pbuf);
        }
    }
    fprintf(out, "\n");

    print_line(out, TOTAL_SLOTS);

    for (int d = 0; d < num_days; d++) {
        fprintf(out, "  | %-10s |", DAY_NAMES[d]);
        for (int s = 0; s < TOTAL_SLOTS; s++) {
            const char *label = get_slot_label(timetable[cls][d][s]);
            fprintf(out, " %-12s |", label);
        }
        fprintf(out, "\n");
        print_line(out, TOTAL_SLOTS);
    }
}

static void print_summary(FILE *out, int cls)
{
    fprintf(out, "\n  Subject Allocation Summary (Class %d):\n", cls + 1);
    fprintf(out, "  +-----------------------+----------+----------+--------+\n");
    fprintf(out, "  | %-21s | %-8s | %-8s | %-6s |\n",
            "Subject", "Required", "Assigned", "Type");
    fprintf(out, "  +-----------------------+----------+----------+--------+\n");

    int total_assigned = 0;
    for (int i = 0; i < num_subjects; i++) {
        int assigned = 0;
        for (int d = 0; d < num_days; d++)
            for (int s = 0; s < TOTAL_SLOTS; s++)
                if (timetable[cls][d][s] == i)
                    assigned++;
        total_assigned += assigned;

        fprintf(out, "  | %-21s | %-8d | %-8d | %-6s |\n",
                subjects[i].name,
                subjects[i].weekly_hours,
                assigned,
                subjects[i].is_lab ? "Lab" : "Theory");
    }
    fprintf(out, "  +-----------------------+----------+----------+--------+\n");

    int free_count = 0;
    for (int d = 0; d < num_days; d++)
        for (int s = 0; s < TOTAL_SLOTS; s++)
            if (timetable[cls][d][s] == FREE_PERIOD)
                free_count++;

    int usable = 7 * num_days;
    fprintf(out, "  Total: %d assigned + %d free = %d / %d usable slots\n",
            total_assigned, free_count, total_assigned + free_count, usable);
}

/* ──────────────── MySQL Storage ──────────────── */

static void sql_escape(char *dest, const char *src, int max)
{
    int j = 0;
    for (int i = 0; src[i] && j < max - 2; i++) {
        if (src[i] == '\'') {
            dest[j++] = '\'';
            dest[j++] = '\'';
        } else {
            dest[j++] = src[i];
        }
    }
    dest[j] = '\0';
}

static int save_to_mysql_tui(void)
{
    /* Generate SQL file */
    FILE *sql = fopen("timetable_data.sql", "w");
    if (!sql) {
        return -1;
    }

    fprintf(sql, "-- Timetable Generator - Auto-generated SQL\n");
    fprintf(sql, "-- Database: %s\n\n", DB_NAME);

    fprintf(sql, "CREATE DATABASE IF NOT EXISTS %s;\n", DB_NAME);
    fprintf(sql, "USE %s;\n\n", DB_NAME);

    fprintf(sql, "DROP TABLE IF EXISTS timetable;\n");
    fprintf(sql, "DROP TABLE IF EXISTS subjects;\n");
    fprintf(sql, "DROP TABLE IF EXISTS config;\n\n");

    fprintf(sql,
        "CREATE TABLE config (\n"
        "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
        "  num_classes INT NOT NULL,\n"
        "  num_days INT NOT NULL,\n"
        "  num_subjects INT NOT NULL,\n"
        "  periods_per_day INT NOT NULL,\n"
        "  generated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP\n"
        ");\n\n");

    fprintf(sql,
        "INSERT INTO config (num_classes, num_days, num_subjects, periods_per_day)\n"
        "VALUES (%d, %d, %d, 7);\n\n",
        num_classes, num_days, num_subjects);

    fprintf(sql,
        "CREATE TABLE subjects (\n"
        "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
        "  subject_index INT NOT NULL,\n"
        "  name VARCHAR(50) NOT NULL,\n"
        "  weekly_hours INT NOT NULL,\n"
        "  type VARCHAR(10) NOT NULL,\n"
        "  max_per_day INT NOT NULL\n"
        ");\n\n");

    for (int i = 0; i < num_subjects; i++) {
        char escaped[61];
        sql_escape(escaped, subjects[i].name, 61);
        fprintf(sql,
            "INSERT INTO subjects (subject_index, name, weekly_hours, type, max_per_day)\n"
            "VALUES (%d, '%s', %d, '%s', %d);\n",
            i, escaped, subjects[i].weekly_hours,
            subjects[i].is_lab ? "Lab" : "Theory",
            subjects[i].max_per_day);
    }

    fprintf(sql, "\n"
        "CREATE TABLE timetable (\n"
        "  id INT AUTO_INCREMENT PRIMARY KEY,\n"
        "  class_num INT NOT NULL,\n"
        "  day_name VARCHAR(15) NOT NULL,\n"
        "  slot_num INT NOT NULL,\n"
        "  time_start VARCHAR(10) NOT NULL,\n"
        "  time_end VARCHAR(10) NOT NULL,\n"
        "  subject VARCHAR(50) NOT NULL,\n"
        "  slot_type VARCHAR(15) NOT NULL\n"
        ");\n\n");

    for (int c = 0; c < num_classes; c++) {
        fprintf(sql, "-- Class %d\n", c + 1);
        for (int d = 0; d < num_days; d++) {
            for (int s = 0; s < TOTAL_SLOTS; s++) {
                const char *label = get_slot_label(timetable[c][d][s]);
                const char *slot_type;

                int val = timetable[c][d][s];
                if (val == TEA_BREAK)        slot_type = "Tea Break";
                else if (val == LUNCH_BREAK) slot_type = "Lunch Break";
                else if (val == FREE_PERIOD) slot_type = "Free";
                else if (val >= 0)           slot_type = subjects[val].is_lab ? "Lab" : "Theory";
                else                         slot_type = "Empty";

                char escaped_label[61];
                sql_escape(escaped_label, label, 61);

                fprintf(sql,
                    "INSERT INTO timetable "
                    "(class_num, day_name, slot_num, time_start, time_end, subject, slot_type)\n"
                    "VALUES (%d, '%s', %d, '%s', '%s', '%s', '%s');\n",
                    c + 1, DAY_NAMES[d], s + 1,
                    SLOT_START[s], SLOT_END[s],
                    escaped_label, slot_type);
            }
        }
        fprintf(sql, "\n");
    }

    fclose(sql);

    char cmd[512];
    sprintf(cmd,
        "\"C:\\Program Files\\MySQL\\MySQL Server 9.3\\bin\\mysql.exe\" "
        "-u%s -p%s < timetable_data.sql > mysql_out.txt 2> mysql_err.txt",
        db_user, db_password);

    int result = system(cmd);
    return result;
}

/* ──────────────── TUI Screen Drawing Code ──────────────── */

static void draw_config_screen(int focus) {
    tui_clear();
    
    // Draw top panel
    tui_draw_str(10, 2, "╔══════════════════════════════════════════════════════════════════════════════════════════╗", 180, 190, 254, 30, 30, 46);
    tui_draw_str(10, 3, "║                               WEEKLY TIMETABLE GENERATOR                                 ║", 180, 190, 254, 30, 30, 46);
    tui_draw_str(10, 4, "╚══════════════════════════════════════════════════════════════════════════════════════════╝", 180, 190, 254, 30, 30, 46);
    
    tui_draw_str(12, 6, "Configure your timetable parameters. Use [UP]/[DOWN] to move, [ENTER] to proceed.", 205, 214, 244, 30, 30, 46);

    // Highlight colors
    unsigned char focus_fg_r = 137, focus_fg_g = 180, focus_fg_b = 250; // Selected Blue
    unsigned char normal_fg_r = 205, normal_fg_g = 214, normal_fg_b = 244; // Text Off-white
    
    char buf[128];

    // Field 1: Classes
    sprintf(buf, "  Number of Classes (1-%d)  :  [ %d ]", MAX_CLASSES, num_classes);
    tui_draw_str(15, 10, buf, (focus == 0) ? focus_fg_r : normal_fg_r, (focus == 0) ? focus_fg_g : normal_fg_g, (focus == 0) ? focus_fg_b : normal_fg_b, 30, 30, 46);
    if (focus == 0) tui_draw_str(75, 10, "<- Use LEFT/RIGHT arrows or type digits", 180, 190, 254, 30, 30, 46);

    // Field 2: Days
    sprintf(buf, "  Number of Days (1-%d)     :  [ %d ]", MAX_DAYS, num_days);
    tui_draw_str(15, 12, buf, (focus == 1) ? focus_fg_r : normal_fg_r, (focus == 1) ? focus_fg_g : normal_fg_g, (focus == 1) ? focus_fg_b : normal_fg_b, 30, 30, 46);
    if (focus == 1) tui_draw_str(75, 12, "<- Use LEFT/RIGHT arrows or type digits", 180, 190, 254, 30, 30, 46);

    // Field 3: Subjects
    sprintf(buf, "  Number of Subjects (1-%d) :  [ %d ]", MAX_SUBJECTS, num_subjects);
    tui_draw_str(15, 14, buf, (focus == 2) ? focus_fg_r : normal_fg_r, (focus == 2) ? focus_fg_g : normal_fg_g, (focus == 2) ? focus_fg_b : normal_fg_b, 30, 30, 46);
    if (focus == 2) tui_draw_str(75, 14, "<- Use LEFT/RIGHT arrows or type digits", 180, 190, 254, 30, 30, 46);

    // Field 4: Database Settings header
    tui_draw_str(15, 17, "--- MySQL Database Settings ---", 180, 190, 254, 30, 30, 46);

    // Field 5: MySQL Host
    tui_draw_str(15, 19, "  MySQL Host                :  localhost", 150, 150, 150, 30, 30, 46);

    // Field 6: MySQL User
    sprintf(buf, "  MySQL User                :  [ %s ]", db_user);
    tui_draw_str(15, 21, buf, (focus == 3) ? focus_fg_r : normal_fg_r, (focus == 3) ? focus_fg_g : normal_fg_g, (focus == 3) ? focus_fg_b : normal_fg_b, 30, 30, 46);
    if (focus == 3) tui_draw_str(75, 21, "<- Type username & [Backspace] to delete", 180, 190, 254, 30, 30, 46);

    // Field 7: MySQL Password
    char pwd_masked[64];
    int pwd_len = strlen(db_password);
    for (int i = 0; i < pwd_len; i++) pwd_masked[i] = '*';
    pwd_masked[pwd_len] = '\0';
    sprintf(buf, "  MySQL Password            :  [ %s ]", pwd_masked);
    tui_draw_str(15, 23, buf, (focus == 4) ? focus_fg_r : normal_fg_r, (focus == 4) ? focus_fg_g : normal_fg_g, (focus == 4) ? focus_fg_b : normal_fg_b, 30, 30, 46);
    if (focus == 4) tui_draw_str(75, 23, "<- Type password & [Backspace] to delete", 180, 190, 254, 30, 30, 46);

    // Field 8: Next button
    if (focus == 5) {
        tui_draw_str(35, 27, "  [ PROCEED TO SUBJECT CONFIGURATION ]  ", 30, 30, 46, 166, 227, 161);
    } else {
        tui_draw_str(35, 27, "    PROCEED TO SUBJECT CONFIGURATION    ", 166, 227, 161, 30, 30, 46);
    }
    
    // Bottom instructions
    tui_draw_str(10, 31, "────────────────────────────────────────────────────────────────────────────────────────────", 100, 100, 110, 30, 30, 46);
    tui_draw_str(12, 32, "Press [Esc] to Exit the Application.", 243, 139, 168, 30, 30, 46);
}

static void handle_config_input(int key, int *focus) {
    if (key == 13) { // Enter
        if (*focus == 5) {
            if (num_classes < 1) num_classes = 1;
            if (num_days < 1) num_days = 1;
            if (num_subjects < 1) num_subjects = 1;
            current_screen = SCREEN_SUBJECTS;
            // Fill default subjects details
            for (int i = 0; i < num_subjects; i++) {
                if (subjects[i].name[0] == '\0') {
                    sprintf(subjects[i].name, "Subj %d", i + 1);
                    subjects[i].weekly_hours = 4;
                    subjects[i].is_lab = 0;
                    subjects[i].max_per_day = 1;
                }
            }
        } else {
            *focus = 5;
        }
        return;
    }
    
    if (key == 224 || key == 0) { // Special keys
        int extra = _getch();
        if (extra == 72) { // Up
            (*focus)--;
            if (*focus < 0) *focus = 5;
        } else if (extra == 80) { // Down
            (*focus)++;
            if (*focus > 5) *focus = 0;
        } else if (extra == 75) { // Left
            if (*focus == 0 && num_classes > 1) num_classes--;
            else if (*focus == 1 && num_days > 1) num_days--;
            else if (*focus == 2 && num_subjects > 1) num_subjects--;
        } else if (extra == 77) { // Right
            if (*focus == 0 && num_classes < MAX_CLASSES) num_classes++;
            else if (*focus == 1 && num_days < MAX_DAYS) num_days++;
            else if (*focus == 2 && num_subjects < MAX_SUBJECTS) num_subjects++;
        }
        return;
    }

    if (*focus == 3) { // MySQL User
        int len = strlen(db_user);
        if (key == 8) { // Backspace
            if (len > 0) db_user[len - 1] = '\0';
        } else if (key >= 32 && key <= 126 && len < 63) {
            db_user[len] = (char)key;
            db_user[len + 1] = '\0';
        }
    } else if (*focus == 4) { // MySQL Password
        int len = strlen(db_password);
        if (key == 8) { // Backspace
            if (len > 0) db_password[len - 1] = '\0';
        } else if (key >= 32 && key <= 126 && len < 63) {
            db_password[len] = (char)key;
            db_password[len + 1] = '\0';
        }
    } else {
        if (key >= '0' && key <= '9') {
            int digit = key - '0';
            if (*focus == 0) {
                num_classes = num_classes * 10 + digit;
                if (num_classes > MAX_CLASSES) num_classes = digit;
                if (num_classes == 0) num_classes = 1;
            } else if (*focus == 1) {
                num_days = num_days * 10 + digit;
                if (num_days > MAX_DAYS) num_days = digit;
                if (num_days == 0) num_days = 1;
            } else if (*focus == 2) {
                num_subjects = num_subjects * 10 + digit;
                if (num_subjects > MAX_SUBJECTS) num_subjects = digit;
                if (num_subjects == 0) num_subjects = 1;
            }
        }
    }
}

static void draw_subjects_screen(int focus_row, int focus_col) {
    tui_clear();
    
    // Header
    tui_draw_str(10, 2, "╔══════════════════════════════════════════════════════════════════════════════════════════╗", 180, 190, 254, 30, 30, 46);
    tui_draw_str(10, 3, "║                                  SUBJECT DETAILS EDITOR                                  ║", 180, 190, 254, 30, 30, 46);
    tui_draw_str(10, 4, "╚══════════════════════════════════════════════════════════════════════════════════════════╝", 180, 190, 254, 30, 30, 46);

    tui_draw_str(12, 6, "Fill in subject details. Use ARROW keys to navigate cells. Press [Esc] to return.", 205, 214, 244, 30, 30, 46);

    int table_x = 15;
    int table_y = 9;
    
    tui_draw_str(table_x, table_y,     "┌──────┬───────────────────────────┬────────────────┬──────────────┐", 180, 190, 254, 30, 30, 46);
    tui_draw_str(table_x, table_y + 1, "│  ID  │       Subject Name        │  Weekly Hours  │     Type     │", 180, 190, 254, 30, 30, 46);
    tui_draw_str(table_x, table_y + 2, "├──────┼───────────────────────────┼────────────────┼──────────────┤", 180, 190, 254, 30, 30, 46);

    unsigned char focus_bg_r = 45, focus_bg_g = 45, focus_bg_b = 70;
    unsigned char normal_bg_r = 30, normal_bg_g = 30, normal_bg_b = 46;
    unsigned char text_fg_r = 205, text_fg_g = 214, text_fg_b = 244;
    unsigned char active_fg_r = 137, active_fg_g = 180, active_fg_b = 250;

    int total_hours = 0;

    for (int r = 0; r < num_subjects; r++) {
        total_hours += subjects[r].weekly_hours;
        int curr_y = table_y + 3 + r;
        
        char id_str[8];
        sprintf(id_str, "  %02d  ", r + 1);
        tui_draw_str(table_x, curr_y, "│", 180, 190, 254, normal_bg_r, normal_bg_g, normal_bg_b);
        tui_draw_str(table_x + 1, curr_y, id_str, text_fg_r, text_fg_g, text_fg_b, normal_bg_r, normal_bg_g, normal_bg_b);
        tui_draw_str(table_x + 7, curr_y, "│", 180, 190, 254, normal_bg_r, normal_bg_g, normal_bg_b);

        // Subject Name (with <Enter Name> placeholder)
        char name_cell[30];
        int is_empty = (subjects[r].name[0] == '\0');
        if (is_empty) {
            sprintf(name_cell, " %-25s ", "<Enter Name>");
        } else {
            sprintf(name_cell, " %-25s ", subjects[r].name);
        }
        int is_name_focused = (focus_row == r && focus_col == 0);
        
        unsigned char cell_fg_r = is_name_focused ? active_fg_r : (is_empty ? 120 : text_fg_r);
        unsigned char cell_fg_g = is_name_focused ? active_fg_g : (is_empty ? 120 : text_fg_g);
        unsigned char cell_fg_b = is_name_focused ? active_fg_b : (is_empty ? 130 : text_fg_b);

        tui_draw_str(table_x + 8, curr_y, name_cell, 
                     cell_fg_r, cell_fg_g, cell_fg_b,
                     is_name_focused ? focus_bg_r : normal_bg_r,
                     is_name_focused ? focus_bg_g : normal_bg_g,
                     is_name_focused ? focus_bg_b : normal_bg_b);
        tui_draw_str(table_x + 35, curr_y, "│", 180, 190, 254, normal_bg_r, normal_bg_g, normal_bg_b);

        // Weekly Hours
        char hours_cell[20];
        sprintf(hours_cell, "       %2d       ", subjects[r].weekly_hours);
        int is_hours_focused = (focus_row == r && focus_col == 1);
        tui_draw_str(table_x + 36, curr_y, hours_cell, 
                     is_hours_focused ? active_fg_r : text_fg_r,
                     is_hours_focused ? active_fg_g : text_fg_g,
                     is_hours_focused ? active_fg_b : text_fg_b,
                     is_hours_focused ? focus_bg_r : normal_bg_r,
                     is_hours_focused ? focus_bg_g : normal_bg_g,
                     is_hours_focused ? focus_bg_b : normal_bg_b);
        tui_draw_str(table_x + 52, curr_y, "│", 180, 190, 254, normal_bg_r, normal_bg_g, normal_bg_b);

        // Type
        char type_cell[16];
        sprintf(type_cell, "   %-10s ", subjects[r].is_lab ? "Lab" : "Theory");
        int is_type_focused = (focus_row == r && focus_col == 2);
        tui_draw_str(table_x + 53, curr_y, type_cell, 
                     is_type_focused ? active_fg_r : text_fg_r,
                     is_type_focused ? active_fg_g : text_fg_g,
                     is_type_focused ? active_fg_b : text_fg_b,
                     is_type_focused ? focus_bg_r : normal_bg_r,
                     is_type_focused ? focus_bg_g : normal_bg_g,
                     is_type_focused ? focus_bg_b : normal_bg_b);
        tui_draw_str(table_x + 67, curr_y, "│", 180, 190, 254, normal_bg_r, normal_bg_g, normal_bg_b);
    }
    
    int last_y = table_y + 3 + num_subjects;
    tui_draw_str(table_x, last_y, "└──────┴───────────────────────────┴────────────────┴──────────────┘", 180, 190, 254, normal_bg_r, normal_bg_g, normal_bg_b);

    // Generate Button
    int button_y = last_y + 2;
    if (focus_row == num_subjects) {
        tui_draw_str(table_x + 18, button_y, "  [ GENERATE TIMETABLE ]  ", 30, 30, 46, 166, 227, 161);
    } else {
        tui_draw_str(table_x + 18, button_y, "    GENERATE TIMETABLE    ", 166, 227, 161, 30, 30, 46);
    }

    // Stats and error checking
    int available_slots = num_days * 7;
    int total_with_free = total_hours + FREE_PER_WEEK;

    int stats_y = last_y + 4;
    char stats_buf[128];
    sprintf(stats_buf, "  Total Subject Hours: %d  |  Free Periods: %d  |  Total slots needed: %d / %d", 
            total_hours, FREE_PER_WEEK, total_with_free, available_slots);
    
    int has_empty_names = 0;
    for (int i = 0; i < num_subjects; i++) {
        if (subjects[i].name[0] == '\0') {
            has_empty_names = 1;
            break;
        }
    }

    if (has_empty_names) {
        tui_draw_str(table_x, stats_y, "  [!] ERROR: Some subject names are empty! Please enter all names before generating.", 243, 139, 168, 30, 30, 46);
    } else if (total_with_free > available_slots) {
        tui_draw_str(table_x, stats_y, stats_buf, 243, 139, 168, 30, 30, 46);
        tui_draw_str(table_x, stats_y + 1, "  [!] WARNING: Exceeds available slots! Backtracking algorithm might fail or loop.", 243, 139, 168, 30, 30, 46);
    } else {
        tui_draw_str(table_x, stats_y, stats_buf, 166, 227, 161, 30, 30, 46);
        tui_draw_str(table_x, stats_y + 1, "  [OK] Hours allocated are within schedule limits.", 166, 227, 161, 30, 30, 46);
    }

    tui_draw_str(table_x, stats_y + 3, "────────────────────────────────────────────────────────────────────────────────────────────", 100, 100, 110, 30, 30, 46);
    tui_draw_str(table_x, stats_y + 4, "Controls: [ARROW KEYS] Navigate | [TAB/ENTER] Next Field | [DEL] Clear Cell | [Space] Toggle Type", 180, 190, 254, 30, 30, 46);
}

static void handle_subjects_input(int key, int *focus_row, int *focus_col) {
    if (key == 224 || key == 0) {
        int extra = _getch();
        if (extra == 72) { // Up arrow
            if (*focus_row > 0) (*focus_row)--;
            else *focus_row = num_subjects; // Wrap to Generate Button
        } else if (extra == 80) { // Down arrow
            if (*focus_row < num_subjects) (*focus_row)++;
            else *focus_row = 0; // Wrap to first subject
        } else if (extra == 75) { // Left arrow
            if (*focus_row < num_subjects && *focus_col > 0) (*focus_col)--;
        } else if (extra == 77) { // Right arrow
            if (*focus_row < num_subjects && *focus_col < 2) (*focus_col)++;
        } else if (extra == 83) { // DELETE key
            if (*focus_row < num_subjects) {
                if (*focus_col == 0) {
                    subjects[*focus_row].name[0] = '\0';
                } else if (*focus_col == 1) {
                    subjects[*focus_row].weekly_hours = 0;
                }
            }
        }
        return;
    }

    if (key == 9) { // Tab key
        if (*focus_row == num_subjects) {
            *focus_row = 0;
            *focus_col = 0;
        } else {
            (*focus_col)++;
            if (*focus_col > 2) {
                *focus_col = 0;
                (*focus_row)++;
            }
        }
        return;
    }

    if (key == 13) { // Enter key
        if (*focus_row == num_subjects) {
            int has_empty_names = 0;
            for (int i = 0; i < num_subjects; i++) {
                if (subjects[i].name[0] == '\0') {
                    has_empty_names = 1;
                    break;
                }
            }
            if (!has_empty_names) {
                current_screen = SCREEN_SOLVING;
            }
        } else {
            // Move to next cell on Enter
            (*focus_col)++;
            if (*focus_col > 2) {
                *focus_col = 0;
                (*focus_row)++;
            }
        }
        return;
    }

    // Process alphanumeric keyboard typing for cells
    if (*focus_row < num_subjects) {
        if (*focus_col == 0) {
            int len = strlen(subjects[*focus_row].name);
            if (key == 8) { // Backspace
                if (len > 0) subjects[*focus_row].name[len - 1] = '\0';
            } else if (key >= 32 && key <= 126 && len < SUBJ_NAME_LEN - 2) {
                subjects[*focus_row].name[len] = (char)key;
                subjects[*focus_row].name[len + 1] = '\0';
            }
        } else if (*focus_col == 1) {
            if (key == 8) { // Backspace
                subjects[*focus_row].weekly_hours /= 10;
            } else if (key >= '0' && key <= '9') {
                int digit = key - '0';
                int val = subjects[*focus_row].weekly_hours * 10 + digit;
                if (val <= 40) {
                    subjects[*focus_row].weekly_hours = val;
                }
            }
        } else if (*focus_col == 2) {
            if (key == ' ' || key == 13) {
                subjects[*focus_row].is_lab = !subjects[*focus_row].is_lab;
                if (subjects[*focus_row].is_lab && subjects[*focus_row].weekly_hours % 2 != 0) {
                    subjects[*focus_row].weekly_hours++;
                }
                subjects[*focus_row].max_per_day = subjects[*focus_row].is_lab ? 2 : 1;
            }
        }
    }
}

static void draw_solving_screen(int frame) {
    tui_clear();
    tui_draw_str(38, 15, "┌──────────────────────────────────────────┐", 180, 190, 254, 30, 30, 46);
    tui_draw_str(38, 16, "│      GENERATING CONFLICT-FREE TIMETABLE  │", 205, 214, 244, 30, 30, 46);
    
    char anim[4][10] = { "  ⠋  ", "  ⠙  ", "  ⠹  ", "  ⠸  " };
    char msg[64];
    sprintf(msg, "│             Running Solver%s          │", anim[frame % 4]);
    tui_draw_str(38, 17, msg, 137, 180, 250, 30, 30, 46);
    
    tui_draw_str(38, 18, "└──────────────────────────────────────────┘", 180, 190, 254, 30, 30, 46);
    tui_present();
}

static void draw_timetable_viewer(int cls) {
    tui_clear();

    char title[128];
    sprintf(title, "  <<<   CLASS %02d OF %02d   >>>  ", cls + 1, num_classes);
    tui_draw_str(42, 2, title, 180, 190, 254, 30, 30, 46);
    tui_draw_str(32, 3, "Use [LEFT] / [RIGHT] Arrow Keys to Switch Classes", 150, 150, 160, 30, 30, 46);

    int start_x = 5;
    int start_y = 5;
    
    tui_draw_str(start_x, start_y, "┌──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┐", 180, 190, 254, 30, 30, 46);
    
    // Time Headers
    tui_draw_str(start_x, start_y + 1, "│ Day/Time │", 180, 190, 254, 30, 30, 46);
    for (int s = 0; s < TOTAL_SLOTS; s++) {
        char timebuf[16];
        sprintf(timebuf, "%-10s│", SLOT_START[s]);
        tui_draw_str(start_x + 12 + s * 11, start_y + 1, timebuf, 205, 214, 244, 30, 30, 46);
    }
    
    tui_draw_str(start_x, start_y + 2, "├──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┤", 180, 190, 254, 30, 30, 46);
    
    // Periods Headers
    tui_draw_str(start_x, start_y + 3, "│ Period   │", 180, 190, 254, 30, 30, 46);
    int pnum = 1;
    for (int s = 0; s < TOTAL_SLOTS; s++) {
        char pbuf[16];
        if (is_break_slot(s)) {
            sprintf(pbuf, "%-10s│", s == TEA_SLOT ? "TEA" : "LUNCH");
        } else {
            sprintf(pbuf, "P%d        │", pnum++);
        }
        tui_draw_str(start_x + 12 + s * 11, start_y + 3, pbuf, 205, 214, 244, 30, 30, 46);
    }
    
    tui_draw_str(start_x, start_y + 4, "├──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┤", 180, 190, 254, 30, 30, 46);

    // Rows
    for (int d = 0; d < num_days; d++) {
        int curr_y = start_y + 5 + d * 2;
        
        char day_buf[20];
        sprintf(day_buf, "│%-10s│", DAY_NAMES[d]);
        tui_draw_str(start_x, curr_y, day_buf, 180, 190, 254, 30, 30, 46);

        for (int s = 0; s < TOTAL_SLOTS; s++) {
            int val = timetable[cls][d][s];
            char cell_text[12];
            unsigned char fg_r = 20, fg_g = 20, fg_b = 20;
            unsigned char bg_r = 30, bg_g = 30, bg_b = 46;

            if (val == TEA_BREAK) {
                strcpy(cell_text, "   TEA    ");
                fg_r = 240; fg_g = 240; fg_b = 240;
                bg_r = 80; bg_g = 80; bg_b = 90;
            } else if (val == LUNCH_BREAK) {
                strcpy(cell_text, "  LUNCH   ");
                fg_r = 240; fg_g = 240; fg_b = 240;
                bg_r = 80; bg_g = 80; bg_b = 90;
            } else if (val == FREE_PERIOD) {
                strcpy(cell_text, "   FREE   ");
                fg_r = 240; fg_g = 240; fg_b = 240;
                bg_r = 45; bg_g = 85; bg_b = 125;
            } else if (val == EMPTY) {
                strcpy(cell_text, "   ---    ");
                fg_r = 120; fg_g = 120; fg_b = 130;
                bg_r = 30; bg_g = 30; bg_b = 46;
            } else {
                sprintf(cell_text, " %-8.8s ", subjects[val].name);
                bg_r = SUBJECT_COLORS[val % 20].r;
                bg_g = SUBJECT_COLORS[val % 20].g;
                bg_b = SUBJECT_COLORS[val % 20].b;
                fg_r = 20; fg_g = 20; fg_b = 20;
            }
            
            tui_draw_str(start_x + 12 + s * 11, curr_y, cell_text, fg_r, fg_g, fg_b, bg_r, bg_g, bg_b);
            tui_draw_str(start_x + 22 + s * 11, curr_y, "│", 180, 190, 254, 30, 30, 46);
        }

        if (d < num_days - 1) {
            tui_draw_str(start_x, curr_y + 1, "├──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┤", 180, 190, 254, 30, 30, 46);
        } else {
            tui_draw_str(start_x, curr_y + 1, "└──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┘", 180, 190, 254, 30, 30, 46);
        }
    }

    // Allocation Summary
    int summary_y = start_y + 5 + num_days * 2 + 1;
    tui_draw_str(start_x, summary_y, "Subject Summary for Class:", 180, 190, 254, 30, 30, 46);
    
    for (int i = 0; i < num_subjects; i++) {
        int assigned = 0;
        for (int d = 0; d < num_days; d++)
            for (int s = 0; s < TOTAL_SLOTS; s++)
                if (timetable[cls][d][s] == i)
                    assigned++;
        
        char sum_buf[64];
        sprintf(sum_buf, "%s: %d/%d hrs (%s)", subjects[i].name, assigned, subjects[i].weekly_hours, subjects[i].is_lab ? "Lab" : "Theory");
        
        int row_offset = i / 3;
        int col_offset = i % 3;
        tui_draw_str(start_x + col_offset * 37, summary_y + 1 + row_offset, sum_buf, 205, 214, 244, 30, 30, 46);
    }

    // Actions block
    int actions_y = TUI_HEIGHT - 4;
    tui_draw_str(start_x, actions_y, "────────────────────────────────────────────────────────────────────────────────────────────", 100, 100, 110, 30, 30, 46);
    tui_draw_str(start_x, actions_y + 1, "Actions: [S] Save to timetable.txt  |  [M] Export to MySQL  |  [Esc] Back to Edit", 166, 227, 161, 30, 30, 46);

    if (viewer_status[0] != '\0') {
        tui_draw_str(start_x, actions_y + 2, viewer_status, status_color_r, status_color_g, status_color_b, 30, 30, 46);
    }
}

static int run_generator(char *status_msg) {
    int overall_success = 1;
    status_msg[0] = '\0';
    
    for (int c = 0; c < num_classes; c++) {
        int success = 0;

        for (int attempt = 0; attempt < 50 && !success; attempt++) {

            for (int d = 0; d < num_days; d++)
                for (int s = 0; s < TOTAL_SLOTS; s++)
                    timetable[c][d][s] = EMPTY;

            for (int d = 0; d < num_days; d++) {
                timetable[c][d][TEA_SLOT]   = TEA_BREAK;
                timetable[c][d][LUNCH_SLOT] = LUNCH_BREAK;
            }

            place_free_periods(c);

            for (int i = 0; i < num_subjects; i++)
                remaining[i] = subjects[i].weekly_hours;

            if (solve(c, 0, 0)) {
                int all_placed = 1;
                for (int i = 0; i < num_subjects; i++) {
                    if (remaining[i] > 0) { all_placed = 0; break; }
                }
                if (all_placed) success = 1;
            }
        }
        
        if (!success) {
            overall_success = 0;
            sprintf(status_msg, "Class %d generated a partial schedule.", c + 1);
        }
    }
    return overall_success;
}

/* ──────────────── Main Program Entry ──────────────── */

int main()
{
    srand((unsigned int)time(NULL));

    // Default configuration values
    num_classes = 2;
    num_days = 5;
    num_subjects = 6;
    strcpy(db_user, "root");
    db_password[0] = '\0';
    
    // Default subject values
    char *default_names[] = {"Mathematics", "Physics", "Chemistry", "English", "Computer Sci", "Biology"};
    for (int i = 0; i < MAX_SUBJECTS; i++) {
        if (i < 6) {
            strcpy(subjects[i].name, default_names[i]);
            subjects[i].weekly_hours = 4;
            subjects[i].is_lab = 0;
            subjects[i].max_per_day = 1;
        } else {
            subjects[i].name[0] = '\0';
            subjects[i].weekly_hours = 0;
            subjects[i].is_lab = 0;
            subjects[i].max_per_day = 0;
        }
    }

    tui_init();

    int config_focus = 0;
    int subject_focus_row = 0;
    int subject_focus_col = 0;
    int viewer_class = 0;
    
    int running = 1;
    int tui_needs_redraw = 1;
    while (running) {
        if (tui_needs_redraw) {
            if (current_screen == SCREEN_CONFIG) {
                draw_config_screen(config_focus);
                tui_present();
            } else if (current_screen == SCREEN_SUBJECTS) {
                draw_subjects_screen(subject_focus_row, subject_focus_col);
                tui_present();
            } else if (current_screen == SCREEN_SOLVING) {
                draw_solving_screen(0);
                
                char gen_msg[128];
                int ok = run_generator(gen_msg);
                
                if (ok) {
                    strcpy(viewer_status, "[+] Conflict-free weekly schedule generated successfully!");
                    status_color_r = 166; status_color_g = 227; status_color_b = 161;
                } else {
                    sprintf(viewer_status, "[!] Partial schedule: %s", gen_msg);
                    status_color_r = 249; status_color_g = 226; status_color_b = 175;
                }
                
                viewer_class = 0;
                current_screen = SCREEN_VIEWER;
                tui_needs_redraw = 1;
                continue;
            } else if (current_screen == SCREEN_VIEWER) {
                draw_timetable_viewer(viewer_class);
                tui_present();
            }
            tui_needs_redraw = 0;
        }

        if (_kbhit()) {
            int key = _getch();
            tui_needs_redraw = 1;
            
            if (key == 27) { // ESC key
                if (current_screen == SCREEN_CONFIG) {
                    running = 0;
                } else if (current_screen == SCREEN_SUBJECTS) {
                    current_screen = SCREEN_CONFIG;
                } else if (current_screen == SCREEN_VIEWER) {
                    current_screen = SCREEN_SUBJECTS;
                }
            } else {
                if (current_screen == SCREEN_CONFIG) {
                    handle_config_input(key, &config_focus);
                } else if (current_screen == SCREEN_SUBJECTS) {
                    handle_subjects_input(key, &subject_focus_row, &subject_focus_col);
                } else if (current_screen == SCREEN_VIEWER) {
                    if (key == 224 || key == 0) {
                        int extra = _getch();
                        if (extra == 75) { // Left arrow
                            viewer_class--;
                            if (viewer_class < 0) viewer_class = num_classes - 1;
                            viewer_status[0] = '\0';
                        } else if (extra == 77) { // Right arrow
                            viewer_class++;
                            if (viewer_class >= num_classes) viewer_class = 0;
                            viewer_status[0] = '\0';
                        }
                    } else if (key == 's' || key == 'S') {
                        FILE *fp = fopen("timetable.txt", "w");
                        if (fp) {
                            time_t now = time(NULL);
                            fprintf(fp, "WEEKLY TIMETABLE - Generated: %s", ctime(&now));
                            fprintf(fp, "Config: %d classes, %d days, 7 periods/day, %d subjects\n",
                                    num_classes, num_days, num_subjects);
                            fprintf(fp, "=============================================\n");
                            for (int c = 0; c < num_classes; c++) {
                                print_timetable(fp, c);
                                print_summary(fp, c);
                            }
                            fclose(fp);
                            strcpy(viewer_status, "[+] Timetable saved successfully to 'timetable.txt'");
                            status_color_r = 166; status_color_g = 227; status_color_b = 161;
                        } else {
                            strcpy(viewer_status, "[!] Error: Could not write to 'timetable.txt'");
                            status_color_r = 243; status_color_g = 139; status_color_b = 168;
                        }
                    } else if (key == 'm' || key == 'M') {
                        strcpy(viewer_status, "[*] Saving to MySQL database...");
                        status_color_r = 137; status_color_g = 180; status_color_b = 250;
                        draw_timetable_viewer(viewer_class);
                        tui_present();
                        
                        int db_res = save_to_mysql_tui();
                        if (db_res == 0) {
                            strcpy(viewer_status, "[+] All tables (config, subjects, timetable) exported to MySQL successfully.");
                            status_color_r = 166; status_color_g = 227; status_color_b = 161;
                        } else {
                            sprintf(viewer_status, "[!] MySQL Export Failed (code %d). Check mysql_err.txt.", db_res);
                            status_color_r = 243; status_color_g = 139; status_color_b = 168;
                        }
                    }
                }
            }
        } else {
            Sleep(10);
        }
    }

    tui_shutdown();
    return 0;
}
