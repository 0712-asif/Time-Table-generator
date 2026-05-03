/*
 * ============================================================
 *   WEEKLY TIMETABLE GENERATOR FOR N CLASSES
 *   Using Backtracking Algorithm (DAA Mini Project)
 * ============================================================
 *
 *  Features:
 *    - Generates conflict-free timetables for N classes
 *    - Supports Theory (single slot) and Lab (double slot)
 *    - Fixed time slots: 8:30 AM to 4:15 PM with Tea & Lunch breaks
 *    - 3 FREE periods randomly placed per class per week
 *    - Prevents same-subject repetition in consecutive slots
 *    - Each class has its own independent subject pool
 *    - Exports timetable to file
 *
 *  Algorithm: Constraint-Satisfaction via Backtracking
 *  Time Complexity: O(S^(D*P)) worst case, pruned by constraints
 *
 *  Daily Schedule (matches college format):
 *    Slot 0: 08:30 - 09:25  (Period 1)
 *    Slot 1: 09:25 - 10:20  (Period 2)
 *    Slot 2: 10:20 - 10:40  (TEA BREAK)
 *    Slot 3: 10:40 - 11:35  (Period 3)
 *    Slot 4: 11:35 - 12:30  (Period 4)
 *    Slot 5: 12:30 - 01:30  (LUNCH BREAK)
 *    Slot 6: 01:30 - 02:25  (Period 5)
 *    Slot 7: 02:25 - 03:20  (Period 6)
 *    Slot 8: 03:20 - 04:15  (Period 7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ──────────────── Configuration Limits ──────────────── */

#define MAX_CLASSES    10
#define MAX_DAYS        6
#define TOTAL_SLOTS     9     /* Fixed: 7 periods + 2 breaks     */
#define MAX_SUBJECTS   20
#define NAME_LEN       30
#define FREE_PER_WEEK   3     /* Guaranteed free periods/class    */

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
    char name[NAME_LEN];
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

/* ──────────────── Utility Functions ──────────────── */

/*
 * is_break_slot - Check if a slot is a break.
 */
static int is_break_slot(int slot)
{
    if (slot == TEA_SLOT)   return TEA_BREAK;
    if (slot == LUNCH_SLOT) return LUNCH_BREAK;
    return 0;
}

/*
 * shuffle - Fisher-Yates shuffle for randomized ordering.
 */
static void shuffle(int arr[], int n)
{
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i]  = arr[j];
        arr[j]  = tmp;
    }
}

/*
 * count_subject_in_day - Count occurrences of a subject in a day.
 */
static int count_subject_in_day(int cls, int day, int sub)
{
    int count = 0;
    for (int s = 0; s < TOTAL_SLOTS; s++) {
        if (timetable[cls][day][s] == sub)
            count++;
    }
    return count;
}

/*
 * place_free_periods - Randomly pre-place FREE_PER_WEEK free periods
 * into usable (non-break) slots for a class.
 */
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

/*
 * solve - Fill remaining EMPTY slots with subjects using backtracking.
 *
 * Breaks and FREE periods are already pre-placed.
 * Constraints:
 *   a. Subject has remaining hours
 *   b. No consecutive repetition for theory
 *   c. Daily limit per subject
 *   d. Labs need two consecutive empty slots
 *   e. If no subject fits, mark as FREE (overflow free)
 */
static int solve(int cls, int day, int slot)
{
    if (day >= num_days)
        return 1;

    if (slot >= TOTAL_SLOTS)
        return solve(cls, day + 1, 0);

    /* Skip non-empty slots (breaks, free, already filled) */
    if (timetable[cls][day][slot] != EMPTY)
        return solve(cls, day, slot + 1);

    /* Try each subject in random order */
    int order[MAX_SUBJECTS];
    for (int i = 0; i < num_subjects; i++)
        order[i] = i;
    shuffle(order, num_subjects);

    for (int i = 0; i < num_subjects; i++) {
        int sub = order[i];

        if (remaining[sub] <= 0)
            continue;

        /* No consecutive repetition for theory */
        if (!subjects[sub].is_lab && slot > 0) {
            int prev = slot - 1;
            if (is_break_slot(prev) && prev > 0)
                prev--;
            if (prev >= 0 && timetable[cls][day][prev] == sub)
                continue;
        }

        /* Daily limit */
        if (count_subject_in_day(cls, day, sub) >= subjects[sub].max_per_day)
            continue;

        /* LAB: double slot */
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
            /* THEORY: single slot */
            timetable[cls][day][slot] = sub;
            remaining[sub]--;

            if (solve(cls, day, slot + 1))
                return 1;

            timetable[cls][day][slot] = EMPTY;
            remaining[sub]++;
        }
    }

    /* No subject fits -> mark as extra FREE and continue */
    timetable[cls][day][slot] = FREE_PERIOD;
    if (solve(cls, day, slot + 1))
        return 1;
    timetable[cls][day][slot] = EMPTY;

    return 0;
}

/* ──────────────── Display Functions ──────────────── */

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

    /* Row 1: Time slots */
    fprintf(out, "  | %-10s |", "Time");
    for (int s = 0; s < TOTAL_SLOTS; s++) {
        char timebuf[14];
        sprintf(timebuf, "%s-%s", SLOT_START[s], SLOT_END[s]);
        fprintf(out, " %-12s |", timebuf);
    }
    fprintf(out, "\n");

    print_line(out, TOTAL_SLOTS);

    /* Row 2: Period labels */
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

    /* Data rows */
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

/* ──────────────── Input Functions ──────────────── */

static void read_config(void)
{
    printf("\n");
    printf("  =============================================\n");
    printf("   WEEKLY TIMETABLE GENERATOR (DAA Project)\n");
    printf("   Algorithm: Backtracking + Constraints\n");
    printf("  =============================================\n");
    printf("\n");
    printf("  Daily Schedule:\n");
    printf("    08:30 - 09:25  Period 1\n");
    printf("    09:25 - 10:20  Period 2\n");
    printf("    10:20 - 10:40  TEA BREAK\n");
    printf("    10:40 - 11:35  Period 3\n");
    printf("    11:35 - 12:30  Period 4\n");
    printf("    12:30 - 01:30  LUNCH BREAK\n");
    printf("    01:30 - 02:25  Period 5\n");
    printf("    02:25 - 03:20  Period 6\n");
    printf("    03:20 - 04:15  Period 7\n");
    printf("\n");

    printf("  -- Configuration --\n\n");

    printf("  Number of classes  (1-%d) : ", MAX_CLASSES);
    scanf("%d", &num_classes);
    if (num_classes < 1 || num_classes > MAX_CLASSES) {
        printf("  [!] Invalid. Setting to 1.\n");
        num_classes = 1;
    }

    printf("  Number of days     (1-%d) : ", MAX_DAYS);
    scanf("%d", &num_days);
    if (num_days < 1 || num_days > MAX_DAYS) {
        printf("  [!] Invalid. Setting to 5.\n");
        num_days = 5;
    }

    printf("  Number of subjects (1-%d) : ", MAX_SUBJECTS);
    scanf("%d", &num_subjects);
    if (num_subjects < 1 || num_subjects > MAX_SUBJECTS) {
        printf("  [!] Invalid. Setting to 5.\n");
        num_subjects = 5;
    }

    int available = 7 * num_days;
    printf("\n  [*] Available: %d slots (7 periods x %d days)\n", available, num_days);
    printf("  [*] 3 FREE periods will be randomly placed per class.\n\n");
}

static void read_subjects(void)
{
    printf("  -- Subject Details --\n");
    printf("  (Same subjects for ALL %d classes)\n", num_classes);

    for (int i = 0; i < num_subjects; i++) {
        printf("\n  [Subject %d]\n", i + 1);

        printf("    Name              : ");
        scanf(" %[^\n]", subjects[i].name);

        printf("    Periods per week  : ");
        if (scanf("%d", &subjects[i].weekly_hours) != 1) {
            printf("    [!] Invalid input. Retry.\n");
            while (getchar() != '\n');
            i--;
            continue;
        }

        printf("    Type (0=Theory, 1=Lab) : ");
        if (scanf("%d", &subjects[i].is_lab) != 1 ||
            (subjects[i].is_lab != 0 && subjects[i].is_lab != 1)) {
            printf("    [!] Invalid. Enter 0 or 1. Retry.\n");
            while (getchar() != '\n');
            i--;
            continue;
        }

        if (subjects[i].is_lab && subjects[i].weekly_hours % 2 != 0) {
            subjects[i].weekly_hours++;
            printf("    [*] Lab hours rounded to %d (needs even count).\n",
                   subjects[i].weekly_hours);
        }

        subjects[i].max_per_day = 2;
    }

    int total = 0;
    for (int i = 0; i < num_subjects; i++)
        total += subjects[i].weekly_hours;

    int available = 7 * num_days;
    printf("\n  [OK] Total subject hours: %d / %d available\n", total, available);
    if (total + FREE_PER_WEEK > available)
        printf("  [WARNING] Hours + free (%d) > slots (%d). Some may overflow.\n",
               total + FREE_PER_WEEK, available);
}

/* ──────────────── Main ──────────────── */

int main(void)
{
    srand((unsigned int)time(NULL));

    read_config();
    read_subjects();

    printf("\n  Generating timetables...\n");

    for (int c = 0; c < num_classes; c++) {
        int success = 0;

        /* Try up to 50 random free placements */
        for (int attempt = 0; attempt < 50 && !success; attempt++) {

            /* Clear timetable */
            for (int d = 0; d < num_days; d++)
                for (int s = 0; s < TOTAL_SLOTS; s++)
                    timetable[c][d][s] = EMPTY;

            /* Place breaks */
            for (int d = 0; d < num_days; d++) {
                timetable[c][d][TEA_SLOT]   = TEA_BREAK;
                timetable[c][d][LUNCH_SLOT] = LUNCH_BREAK;
            }

            /* Place 3 random free periods */
            place_free_periods(c);

            /* Reset subject hours */
            for (int i = 0; i < num_subjects; i++)
                remaining[i] = subjects[i].weekly_hours;

            /* Check if all remaining hours are 0 after solve */
            if (solve(c, 0, 0)) {
                int all_placed = 1;
                for (int i = 0; i < num_subjects; i++) {
                    if (remaining[i] > 0) { all_placed = 0; break; }
                }
                if (all_placed) success = 1;
            }
        }

        if (success) {
            printf("  [+] Class %d: Schedule generated successfully.\n", c + 1);
        } else {
            printf("  [!] Class %d: Partial schedule (some subjects missing).\n", c + 1);
        }
    }

    /* Display */
    printf("\n");
    printf("  =============================================\n");
    printf("        GENERATED WEEKLY TIMETABLES\n");
    printf("  =============================================\n");

    for (int c = 0; c < num_classes; c++) {
        print_timetable(stdout, c);
        print_summary(stdout, c);
    }

    /* Export */
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
        printf("\n  [*] Timetable saved to 'timetable.txt'\n");
    } else {
        printf("\n  [!] Error: Could not write to timetable.txt\n");
    }

    printf("\n  Press Enter to exit...");
    while (getchar() != '\n');
    getchar();

    return 0;
}
