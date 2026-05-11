# Weekly Timetable Generator

A **C-based timetable generator** that creates conflict-free weekly schedules for N classes using a **Backtracking Algorithm** (Constraint Satisfaction).

## Features

- Generates timetables for **multiple classes** simultaneously
- Supports **Theory** (single slot) and **Lab** (double slot) subjects
- Fixed college timings: **8:30 AM to 4:15 PM**
- Includes **Tea Break** (10:20 - 10:40) and **Lunch Break** (12:30 - 1:30)
- **3 FREE periods** randomly placed per class per week
- **No repetition on same day**: Theory subjects appear once; Labs appear as one double-session
- **Database Integration**: Automatically exports to `timetable.txt` and stores in **MySQL**
- Prevents same-subject repetition in consecutive slots

## Daily Schedule

| Time | Slot |
|------|------|
| 08:30 - 09:25 | Period 1 |
| 09:25 - 10:20 | Period 2 |
| 10:20 - 10:40 | TEA BREAK |
| 10:40 - 11:35 | Period 3 |
| 11:35 - 12:30 | Period 4 |
| 12:30 - 01:30 | LUNCH BREAK |
| 01:30 - 02:25 | Period 5 |
| 02:25 - 03:20 | Period 6 |
| 03:20 - 04:15 | Period 7 |

## Algorithm

**Backtracking with Constraint Satisfaction:**
1. Pre-place breaks and 3 random FREE periods
2. Try each subject in shuffled order (for variety)
3. Apply constraints: **No subject repetition on the same day**, lab continuity
4. Backtrack on dead-ends; retry with different free placements

**Time Complexity:** O(S^(D×P)) worst case, heavily pruned by constraints

## Database Schema
The system creates a database `timetable_db` with three tables:
- `config`: Stores generation settings
- `subjects`: Stores subject names and hour requirements
- `timetable`: Stores the final generated schedule with timings

## How to Compile & Run

```bash
gcc main.c -o timetable.exe -std=c99
./timetable.exe
```

## DAA Mini Project
Built as a Design and Analysis of Algorithms mini project demonstrating practical application of backtracking.
