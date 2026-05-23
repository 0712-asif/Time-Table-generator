# Weekly Timetable Generator

A **C-based timetable generator** that creates conflict-free weekly schedules for N classes using a **Backtracking Algorithm** (Constraint Satisfaction), wrapped in a beautiful, high-performance, mouse-free **Truecolor Text User Interface (TUI)**.

## Features

- **Interactive TUI Dashboard**: Styled with a 24-bit Truecolor *Catppuccin Mocha* palette, complete with a navigable settings form and grid-based subject editor.
- **Flicker-Free, Zero-Lag Rendering**: Uses single-write in-memory buffering (formatting frames in RAM and printing in a single `fputs` call) to eliminate input latency.
- **Generates timetables for multiple classes simultaneously** while preventing resource conflicts.
- **Supports Theory (single slot) and Lab (double slot) subjects**.
- **Fixed college timings (8:30 AM to 4:15 PM)** including **Tea Break** (10:20 - 10:40) and **Lunch Break** (12:30 - 1:30).
- **3 FREE periods** randomly placed per class per week.
- **No repetition on the same day**: Theory subjects appear once per day, and Lab subjects appear as a single consecutive double-slot.
- **Database Schema & Text Export**: Exports generated schedules to `timetable.txt` or writes directly to MySQL database using native command-line connectors.

---

## Interactive TUI Controls

### 1. Configuration Screen (Dashboard)
- `[UP]` / `[DOWN]`: Move focus between settings fields (Classes, Days, Subjects, MySQL User/Password).
- `[LEFT]` / `[RIGHT]` or **Numeric Keys**: Increment/decrement or type number values directly.
- **Text Typing**: Enter database username and password (password is safely masked with `*`).
- `[Enter]`: Navigate to next field, or click `[ PROCEED ]` to go to the subject editor.

### 2. Subject Details Editor Grid
- `[Arrow Keys]`: Navigate cells up, down, left, and right.
- `[Tab]` / `[Enter]`: Commit the cell value and jump focus to the next cell.
- `[Space]`: Toggle subject type between **Theory** and **Lab** (Labs automatically verify and set even weekly hours).
- `[Delete]`: Clear text or numeric values in the focused cell instantly.
- `[Backspace]`: Delete character-by-character.
- `[Enter]` on `[ GENERATE TIMETABLE ]`: Validate and launch the solver.

### 3. Timetable Viewer Screen
- `[LEFT]` / `[RIGHT]`: Cycle between different class schedules (e.g., Class 1, Class 2).
- `[S]`: Save the weekly schedule to `timetable.txt`.
- `[M]`: Export and import schedules directly into MySQL server.
- `[Esc]`: Return to the subject grid editor.

---

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

---

## How to Compile & Run

Ensure you have a C compiler like `gcc` (MinGW) on your path.

### Compile & Run the Timetable Generator:
```bash
gcc main.c -o timetable.exe -std=c99
.\timetable.exe
```

### Compile & Run the Automated Constraint Verification Suite:
```bash
gcc test_solver.c -o test_solver.exe
.\test_solver.exe
```

---

## Algorithm

**Backtracking with Constraint Satisfaction:**
1. Pre-places breaks and 3 random FREE periods.
2. Tries subject placements in a shuffled order (for scheduling variety).
3. Applies constraints: **No subject repetition on the same day**, consecutive lab session pairs, no theory repetition in consecutive slots.
4. Backtracks on dead-ends, retrying with alternative free period placements.

**Time Complexity:** $O(S^{D \times P})$ worst-case, prunings by constraints keep execution sub-millisecond in normal configurations.

---

## Database Integration
The system auto-generates `timetable_data.sql` and imports it into MySQL `timetable_db` tables:
- `config`: Generation settings configuration.
- `subjects`: Input list of subject names and hour requirements.
- `timetable`: Generated timetable matrix with exact timing intervals and categories.

---

## DAA Mini Project
Built as a Design and Analysis of Algorithms mini-project demonstrating practical constraint-satisfaction backtracking.
