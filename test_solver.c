#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Rename main of main.c so we can define our own test main
#define main tui_main
#include "main.c"
#undef main

// Helper function to count occurrences of a subject in a class schedule
int get_actual_hours(int cls, int sub) {
    int count = 0;
    for (int d = 0; d < num_days; d++) {
        for (int s = 0; s < TOTAL_SLOTS; s++) {
            if (timetable[cls][d][s] == sub) {
                count++;
            }
        }
    }
    return count;
}

// Verification Routine
void verify_constraints() {
    printf("  [Verification] Checking constraints for %d classes, %d days, %d subjects...\n", num_classes, num_days, num_subjects);
    
    for (int c = 0; c < num_classes; c++) {
        // 1. Check Breaks
        for (int d = 0; d < num_days; d++) {
            assert(timetable[c][d][TEA_SLOT] == TEA_BREAK);
            assert(timetable[c][d][LUNCH_SLOT] == LUNCH_BREAK);
        }
        
        // 2. Check Free Periods
        int free_count = 0;
        for (int d = 0; d < num_days; d++) {
            for (int s = 0; s < TOTAL_SLOTS; s++) {
                if (timetable[c][d][s] == FREE_PERIOD) {
                    free_count++;
                }
            }
        }
        // At least FREE_PER_WEEK free periods should be present
        printf("    Class %d: Free periods count = %d (Expected: >= %d)\n", c + 1, free_count, FREE_PER_WEEK);
        assert(free_count >= FREE_PER_WEEK);

        // 3. Check Subject Hour Allocation and Lab Continuity
        for (int i = 0; i < num_subjects; i++) {
            int assigned = get_actual_hours(c, i);
            printf("    Class %d, Subject %d (%s): Assigned = %d, Required = %d\n", 
                   c + 1, i + 1, subjects[i].name, assigned, subjects[i].weekly_hours);
            assert(assigned == subjects[i].weekly_hours);

            // Lab session check: must be in pairs of consecutive slots
            if (subjects[i].is_lab) {
                int lab_slots_checked = 0;
                for (int d = 0; d < num_days; d++) {
                    for (int s = 0; s < TOTAL_SLOTS; s++) {
                        if (timetable[c][d][s] == i) {
                            if (s + 1 < TOTAL_SLOTS && timetable[c][d][s + 1] == i) {
                                lab_slots_checked += 2;
                                s++; // Skip next slot
                            } else {
                                assert(0 && "Lab subject must be scheduled in consecutive double-slots!");
                            }
                        }
                    }
                }
                assert(lab_slots_checked == subjects[i].weekly_hours);
            }
        }

        // 4. Consecutive Theory Repetition Check
        for (int d = 0; d < num_days; d++) {
            for (int s = 0; s < TOTAL_SLOTS - 1; s++) {
                int curr = timetable[c][d][s];
                int next = timetable[c][d][s + 1];
                if (curr >= 0 && next >= 0 && curr == next) {
                    // Repetition is only allowed for lab subjects
                    assert(subjects[curr].is_lab && "Consecutive repetition is only allowed for Lab subjects!");
                }
            }
        }
    }
    printf("  [Success] All constraint checks passed!\n\n");
}

int main() {
    printf("============================================================\n");
    printf("           RUNNING TIMETABLE SOLVER UNIT TESTS              \n");
    printf("============================================================\n\n");

    srand(12345); // Seed with constant to make tests deterministic

    // ------------------------------------------------------------------
    // TEST 1: Standard Configuration (Theory only)
    // ------------------------------------------------------------------
    printf("[Test 1] Standard theory-only timetable generation\n");
    num_classes = 3;
    num_days = 5;
    num_subjects = 5;

    char *subj_names[] = {"Mathematics", "Physics", "Chemistry", "English", "Biology"};
    for (int i = 0; i < num_subjects; i++) {
        strcpy(subjects[i].name, subj_names[i]);
        subjects[i].weekly_hours = 4;
        subjects[i].is_lab = 0;
        subjects[i].max_per_day = 1;
    }

    char status_msg[128];
    int gen_ok = run_generator(status_msg);
    assert(gen_ok && "Failed to generate standard timetable!");
    verify_constraints();

    // ------------------------------------------------------------------
    // TEST 2: Theory + Lab (Even hours)
    // ------------------------------------------------------------------
    printf("[Test 2] Mixed Theory and Lab subjects timetable generation\n");
    num_classes = 2;
    num_days = 6;
    num_subjects = 6;

    char *subj_names2[] = {"Maths", "Physics", "Chemistry", "Comp Sci Lab", "Physics Lab", "English"};
    int hours2[] = {4, 4, 3, 4, 2, 3};
    int is_lab2[] = {0, 0, 0, 1, 1, 0};

    for (int i = 0; i < num_subjects; i++) {
        strcpy(subjects[i].name, subj_names2[i]);
        subjects[i].weekly_hours = hours2[i];
        subjects[i].is_lab = is_lab2[i];
        subjects[i].max_per_day = is_lab2[i] ? 2 : 1;
    }

    gen_ok = run_generator(status_msg);
    assert(gen_ok && "Failed to generate mixed theory/lab timetable!");
    verify_constraints();

    // ------------------------------------------------------------------
    // TEST 3: Over-allocated Hours (Should handle failure gracefully)
    // ------------------------------------------------------------------
    printf("[Test 3] Over-allocated hours generation check\n");
    num_classes = 1;
    num_days = 3; // 3 days * 7 periods = 21 slots
    num_subjects = 5;

    // Allocate 25 hours total (exceeds available 21 slots)
    for (int i = 0; i < num_subjects; i++) {
        strcpy(subjects[i].name, "Overload");
        subjects[i].weekly_hours = 5; 
        subjects[i].is_lab = 0;
        subjects[i].max_per_day = 1;
    }

    gen_ok = run_generator(status_msg);
    printf("    Generator returned: %d (Expected: 0 for failure)\n", gen_ok);
    assert(gen_ok == 0 && "Generator should have failed due to slot overload!");
    printf("    Graceful failure message: '%s'\n", status_msg);
    printf("  [Success] Overload check passed!\n\n");

    printf("============================================================\n");
    printf("        ALL TESTS COMPLETED SUCCESSFULLY (100%% PASSED)    \n");
    printf("============================================================\n");

    return 0;
}
