CREATE DATABASE LMS_DB;
USE LMS_DB;

-- Primary storage for student profiles
CREATE TABLE Students (
    student_id INT PRIMARY KEY,
    full_name VARCHAR(100) NOT NULL,
    password VARCHAR(100) NOT NULL,
    enrolled_course VARCHAR(50),
    numeric_grade FLOAT DEFAULT 0.0,
    letter_grade CHAR(1) DEFAULT 'N',
    has_grade BOOLEAN DEFAULT FALSE
);
