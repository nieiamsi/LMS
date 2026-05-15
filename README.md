# Learning Management System (LMS)

A robust, memory-efficient C++ application designed to bridge the gap between students and instructors. This system utilizes a dual-tier storage strategy, combining the relational power of MySQL with the immutable security of file-stream audit logging.

## Core Functionalities

### For Students

- Secure Registration & Login: Create personal profiles with unique IDs and passwords.
- Course Enrollment: Enroll in academic courses by entering specific course codes.
- Personal Dashboard: Real-time view of student information, enrolled courses, and academic results (marks and letter grades).
- Resource Access: Simulated module download system for course materials.

### For Instructors

- Secure Access: Role-based authentication using administrative credentials.
- Grade Management: Search for students by ID and update or append marks.
- Automated Evaluation: The system automatically calculates letter grades (A-F) based on numeric input.

## Technical Implementation

### 1. Memory Optimization

The project focuses on efficient RAM usage by employing specific C++ data structures:

- Structs: Encapsulates complex student data into a single, manageable object.
- Unions: Implements GradeData unions to store either floating-point percentages or character-based letter grades in the same memory location, reducing the application's memory footprint.
- STL Maps: Utilizes std::map for lookup efficiency and automatic ID sorting.

### 2. Dual-Tier Storage Strategy

To ensure maximum data integrity and redundancy:

- Primary Storage (MySQL): All searchable and structured data (profiles, grades, enrollments) is stored in a relational MySQL database.
- Secondary Storage (File Handling): Audit Trail, an ofstream based audit_log.txt records every instructor modification. This file is immutable (append-only) to provide a secure backup independent of the database.
- Content Delivery: Uses ifstream logic to simulate the retrieval of lecture notes and modules.

## Prerequisites & Setup

- Compiler: MinGW-w64 (GCC).
- IDE: CodeBlocks.
- Database: MySQL Community Server.
- Connector: MySQL Connector/C++ 8.x.
