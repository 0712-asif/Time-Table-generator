-- Timetable Generator - Auto-generated SQL
-- Database: timetable_db

CREATE DATABASE IF NOT EXISTS timetable_db;
USE timetable_db;

DROP TABLE IF EXISTS timetable;
DROP TABLE IF EXISTS subjects;
DROP TABLE IF EXISTS config;

CREATE TABLE config (
  id INT AUTO_INCREMENT PRIMARY KEY,
  num_classes INT NOT NULL,
  num_days INT NOT NULL,
  num_subjects INT NOT NULL,
  periods_per_day INT NOT NULL,
  generated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO config (num_classes, num_days, num_subjects, periods_per_day)
VALUES (2, 5, 6, 7);

CREATE TABLE subjects (
  id INT AUTO_INCREMENT PRIMARY KEY,
  subject_index INT NOT NULL,
  name VARCHAR(50) NOT NULL,
  weekly_hours INT NOT NULL,
  type VARCHAR(10) NOT NULL,
  max_per_day INT NOT NULL
);

INSERT INTO subjects (subject_index, name, weekly_hours, type, max_per_day)
VALUES (0, 'Mathematics', 4, 'Theory', 2);
INSERT INTO subjects (subject_index, name, weekly_hours, type, max_per_day)
VALUES (1, 'Physics', 4, 'Theory', 2);
INSERT INTO subjects (subject_index, name, weekly_hours, type, max_per_day)
VALUES (2, 'Chemistry', 4, 'Theory', 2);
INSERT INTO subjects (subject_index, name, weekly_hours, type, max_per_day)
VALUES (3, 'English', 3, 'Theory', 2);
INSERT INTO subjects (subject_index, name, weekly_hours, type, max_per_day)
VALUES (4, 'Computer Science', 4, 'Theory', 2);
INSERT INTO subjects (subject_index, name, weekly_hours, type, max_per_day)
VALUES (5, 'Biology', 3, 'Theory', 2);

CREATE TABLE timetable (
  id INT AUTO_INCREMENT PRIMARY KEY,
  class_num INT NOT NULL,
  day_name VARCHAR(15) NOT NULL,
  slot_num INT NOT NULL,
  time_start VARCHAR(10) NOT NULL,
  time_end VARCHAR(10) NOT NULL,
  subject VARCHAR(50) NOT NULL,
  slot_type VARCHAR(15) NOT NULL
);

-- Class 1
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Monday', 1, '08:30', '09:25', 'Mathematics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Monday', 2, '09:25', '10:20', 'Physics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Monday', 3, '10:20', '10:40', 'TEA BREAK', 'Tea Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Monday', 4, '10:40', '11:35', 'English', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Monday', 5, '11:35', '12:30', 'Chemistry', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Monday', 6, '12:30', '01:30', 'LUNCH', 'Lunch Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Monday', 7, '01:30', '02:25', 'Biology', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Monday', 8, '02:25', '03:20', 'Physics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Monday', 9, '03:20', '04:15', 'English', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Tuesday', 1, '08:30', '09:25', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Tuesday', 2, '09:25', '10:20', 'Chemistry', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Tuesday', 3, '10:20', '10:40', 'TEA BREAK', 'Tea Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Tuesday', 4, '10:40', '11:35', 'Physics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Tuesday', 5, '11:35', '12:30', 'Computer Science', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Tuesday', 6, '12:30', '01:30', 'LUNCH', 'Lunch Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Tuesday', 7, '01:30', '02:25', 'Chemistry', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Tuesday', 8, '02:25', '03:20', 'Physics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Tuesday', 9, '03:20', '04:15', 'English', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Wednesday', 1, '08:30', '09:25', 'Chemistry', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Wednesday', 2, '09:25', '10:20', 'Computer Science', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Wednesday', 3, '10:20', '10:40', 'TEA BREAK', 'Tea Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Wednesday', 4, '10:40', '11:35', 'Biology', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Wednesday', 5, '11:35', '12:30', 'Computer Science', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Wednesday', 6, '12:30', '01:30', 'LUNCH', 'Lunch Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Wednesday', 7, '01:30', '02:25', 'Biology', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Wednesday', 8, '02:25', '03:20', 'Mathematics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Wednesday', 9, '03:20', '04:15', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Thursday', 1, '08:30', '09:25', 'Computer Science', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Thursday', 2, '09:25', '10:20', 'Mathematics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Thursday', 3, '10:20', '10:40', 'TEA BREAK', 'Tea Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Thursday', 4, '10:40', '11:35', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Thursday', 5, '11:35', '12:30', 'Mathematics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Thursday', 6, '12:30', '01:30', 'LUNCH', 'Lunch Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Thursday', 7, '01:30', '02:25', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Thursday', 8, '02:25', '03:20', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Thursday', 9, '03:20', '04:15', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Friday', 1, '08:30', '09:25', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Friday', 2, '09:25', '10:20', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Friday', 3, '10:20', '10:40', 'TEA BREAK', 'Tea Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Friday', 4, '10:40', '11:35', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Friday', 5, '11:35', '12:30', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Friday', 6, '12:30', '01:30', 'LUNCH', 'Lunch Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Friday', 7, '01:30', '02:25', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Friday', 8, '02:25', '03:20', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (1, 'Friday', 9, '03:20', '04:15', 'FREE', 'Free');

-- Class 2
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Monday', 1, '08:30', '09:25', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Monday', 2, '09:25', '10:20', 'English', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Monday', 3, '10:20', '10:40', 'TEA BREAK', 'Tea Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Monday', 4, '10:40', '11:35', 'Biology', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Monday', 5, '11:35', '12:30', 'Physics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Monday', 6, '12:30', '01:30', 'LUNCH', 'Lunch Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Monday', 7, '01:30', '02:25', 'Computer Science', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Monday', 8, '02:25', '03:20', 'English', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Monday', 9, '03:20', '04:15', 'Chemistry', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Tuesday', 1, '08:30', '09:25', 'Biology', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Tuesday', 2, '09:25', '10:20', 'Chemistry', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Tuesday', 3, '10:20', '10:40', 'TEA BREAK', 'Tea Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Tuesday', 4, '10:40', '11:35', 'Mathematics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Tuesday', 5, '11:35', '12:30', 'English', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Tuesday', 6, '12:30', '01:30', 'LUNCH', 'Lunch Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Tuesday', 7, '01:30', '02:25', 'Physics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Tuesday', 8, '02:25', '03:20', 'Computer Science', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Tuesday', 9, '03:20', '04:15', 'Chemistry', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Wednesday', 1, '08:30', '09:25', 'Computer Science', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Wednesday', 2, '09:25', '10:20', 'Chemistry', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Wednesday', 3, '10:20', '10:40', 'TEA BREAK', 'Tea Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Wednesday', 4, '10:40', '11:35', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Wednesday', 5, '11:35', '12:30', 'Computer Science', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Wednesday', 6, '12:30', '01:30', 'LUNCH', 'Lunch Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Wednesday', 7, '01:30', '02:25', 'Mathematics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Wednesday', 8, '02:25', '03:20', 'Biology', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Wednesday', 9, '03:20', '04:15', 'Mathematics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Thursday', 1, '08:30', '09:25', 'Physics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Thursday', 2, '09:25', '10:20', 'Mathematics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Thursday', 3, '10:20', '10:40', 'TEA BREAK', 'Tea Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Thursday', 4, '10:40', '11:35', 'Physics', 'Theory');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Thursday', 5, '11:35', '12:30', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Thursday', 6, '12:30', '01:30', 'LUNCH', 'Lunch Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Thursday', 7, '01:30', '02:25', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Thursday', 8, '02:25', '03:20', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Thursday', 9, '03:20', '04:15', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Friday', 1, '08:30', '09:25', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Friday', 2, '09:25', '10:20', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Friday', 3, '10:20', '10:40', 'TEA BREAK', 'Tea Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Friday', 4, '10:40', '11:35', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Friday', 5, '11:35', '12:30', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Friday', 6, '12:30', '01:30', 'LUNCH', 'Lunch Break');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Friday', 7, '01:30', '02:25', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Friday', 8, '02:25', '03:20', 'FREE', 'Free');
INSERT INTO timetable (class_num, day_name, slot_num, time_start, time_end, subject, slot_type)
VALUES (2, 'Friday', 9, '03:20', '04:15', 'FREE', 'Free');

-- Useful query: View timetable for a specific class
-- SELECT day_name, time_start, time_end, subject, slot_type
-- FROM timetable WHERE class_num = 1 ORDER BY FIELD(day_name, 'Monday','Tuesday','Wednesday','Thursday','Friday','Saturday'), slot_num;

