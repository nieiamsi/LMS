CREATE DATABASE IF NOT EXISTS GlobalLMS;
USE GlobalLMS;

-- Table to store organization names
CREATE TABLE IF NOT EXISTS organizations (
    id INT AUTO_INCREMENT PRIMARY KEY,
    org_name VARCHAR(255) UNIQUE NOT NULL
);

-- Table to store student data
-- Prefixing will be handled by the C++ logic, but the table structure remains the same
CREATE TABLE IF NOT EXISTS students (
    student_id VARCHAR(50) PRIMARY KEY,
    org_prefix VARCHAR(100),
    full_name VARCHAR(255),
    email VARCHAR(255),
    phone VARCHAR(50),
    course_code VARCHAR(50),
    password VARCHAR(255),
    mark FLOAT DEFAULT 0,
    grade VARCHAR(5) DEFAULT 'N/A'
);

-- Table for Instructor Credentials (uploaded via CSV)
CREATE TABLE IF NOT EXISTS instructors (
    username VARCHAR(100) PRIMARY KEY,
    password VARCHAR(255),
    course_assigned VARCHAR(50)
);