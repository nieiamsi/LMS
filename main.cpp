#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <limits>
#include <mysql.h>

using namespace std;

struct CourseEnrollment {
    string courseCode;
    float percentage;
    char letterGrade;
    bool hasGrade;
};

struct Student {
    int id;
    string name;
    string password;
    vector<CourseEnrollment> enrollments;
};

struct Instructor {
    int id;
    string name;
    string password;
    string assignedCourseCode;
};

struct Course {
    string code;
    string title;
    int creditHours;
};

map<int, Student> studentMap;
map<int, Instructor> instructorMap;
map<string, Course> courseMap;

const string ADMIN_PASS = "admin123";

string getNonEmptyString(string prompt) {
    string input = "";
    while (true) {
        cout << prompt;
        getline(cin, input);
        if (!input.empty() && input != " ") {
            break;
        }
        cout << "Error: This cannot be empty! Please try again.\n";
    }
    return input;
}

int getValidInteger(string prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (value > 0) {
                return value;
            }
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Error: Please enter a valid number!\n";
    }
}

string sanitizeSQLInput(string input) {
    string cleanStr = "";
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '\'' || input[i] == '\"' || input[i] == '\\' || input[i] == ';') {
            cleanStr += "\\";
        }
        cleanStr += input[i];
    }
    return cleanStr;
}

MYSQL* connectDB() {
    MYSQL* conn = mysql_init(NULL);
    if (conn == NULL) return nullptr;

    if (mysql_real_connect(conn, "127.0.0.1", "root", "@helu921my", "LMS_DB", 3306, NULL, 0)) {
        return conn;
    } else {
        cerr << "Connection Error: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return nullptr;
    }
}

void initializeDatabaseTables() {
    MYSQL* conn = connectDB();
    if (!conn) return;

    string query1 = "CREATE TABLE IF NOT EXISTS Students ("
                    "student_id INT PRIMARY KEY, "
                    "full_name VARCHAR(100) NOT NULL, "
                    "password VARCHAR(50) NOT NULL)";
    mysql_query(conn, query1.c_str());

    string query2 = "CREATE TABLE IF NOT EXISTS Instructors ("
                    "instructor_id INT PRIMARY KEY, "
                    "instructor_name VARCHAR(100) NOT NULL, "
                    "password VARCHAR(50) NOT NULL, "
                    "assigned_course VARCHAR(50))";
    mysql_query(conn, query2.c_str());

    string query3 = "CREATE TABLE IF NOT EXISTS Courses ("
                    "course_code VARCHAR(20) PRIMARY KEY, "
                    "course_title VARCHAR(100) NOT NULL, "
                    "credit_hours INT NOT NULL)";
    mysql_query(conn, query3.c_str());

    string query4 = "CREATE TABLE IF NOT EXISTS Student_Courses ("
                    "student_id INT, "
                    "course_code VARCHAR(20), "
                    "numeric_grade FLOAT, "
                    "letter_grade CHAR(1), "
                    "has_grade TINYINT(1), "
                    "PRIMARY KEY (student_id, course_code))";
    mysql_query(conn, query4.c_str());

    mysql_close(conn);
}

void saveStudentToDB(Student s) {
    MYSQL* conn = connectDB();
    if (!conn) return;

    string cleanName = sanitizeSQLInput(s.name);
    string cleanPass = sanitizeSQLInput(s.password);

    string query = "INSERT INTO Students (student_id, full_name, password) "
                   "VALUES (" + to_string(s.id) + ", '" + cleanName + "', '" + cleanPass + "') "
                   "ON DUPLICATE KEY UPDATE full_name='" + cleanName + "', password='" + cleanPass + "'";
    mysql_query(conn, query.c_str());

    for (const auto& enc : s.enrollments) {
        string cleanCourse = sanitizeSQLInput(enc.courseCode);
        string enrollQuery = "INSERT INTO Student_Courses (student_id, course_code, numeric_grade, letter_grade, has_grade) "
                             "VALUES (" + to_string(s.id) + ", '" + cleanCourse + "', " + to_string(enc.percentage) +
                             ", '" + enc.letterGrade + "', " + (enc.hasGrade ? "1" : "0") + ") "
                             "ON DUPLICATE KEY UPDATE numeric_grade=" + to_string(enc.percentage) +
                             ", letter_grade='" + enc.letterGrade + "', has_grade=" + (enc.hasGrade ? "1" : "0");
        mysql_query(conn, enrollQuery.c_str());
    }

    mysql_close(conn);
}

void saveInstructorToDB(Instructor ins) {
    MYSQL* conn = connectDB();
    if (!conn) return;

    string cleanName = sanitizeSQLInput(ins.name);
    string cleanPass = sanitizeSQLInput(ins.password);
    string cleanCourse = sanitizeSQLInput(ins.assignedCourseCode);

    string query = "INSERT INTO Instructors (instructor_id, instructor_name, password, assigned_course) "
                   "VALUES (" + to_string(ins.id) + ", '" + cleanName + "', '" + cleanPass + "', '" + cleanCourse + "') "
                   "ON DUPLICATE KEY UPDATE instructor_name='" + cleanName + "', password='" + cleanPass + "', assigned_course='" + cleanCourse + "'";

    mysql_query(conn, query.c_str());
    mysql_close(conn);
}

void saveCourseToDB(Course c) {
    MYSQL* conn = connectDB();
    if (!conn) return;

    string cleanCode = sanitizeSQLInput(c.code);
    string cleanTitle = sanitizeSQLInput(c.title);

    string query = "INSERT INTO Courses (course_code, course_title, credit_hours) "
                   "VALUES ('" + cleanCode + "', '" + cleanTitle + "', " + to_string(c.creditHours) + ") "
                   "ON DUPLICATE KEY UPDATE course_title='" + cleanTitle + "', credit_hours=" + to_string(c.creditHours);

    mysql_query(conn, query.c_str());
    mysql_close(conn);
}

void loadFromDB() {
    MYSQL* conn = connectDB();
    if (!conn) return;

    studentMap.clear();
    instructorMap.clear();
    courseMap.clear();

    if (!mysql_query(conn, "SELECT * FROM Students")) {
        MYSQL_RES *res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                Student s;
                s.id = stoi(row[0]);
                s.name = row[1] ? row[1] : "";
                s.password = row[2] ? row[2] : "";
                studentMap[s.id] = s;
            }
            mysql_free_result(res);
        }
    }

    if (!mysql_query(conn, "SELECT * FROM Student_Courses")) {
        MYSQL_RES *res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                int sId = stoi(row[0]);
                if (studentMap.count(sId)) {
                    CourseEnrollment enc;
                    enc.courseCode = row[1] ? row[1] : "";
                    enc.percentage = row[2] ? stof(row[2]) : 0.0f;
                    enc.letterGrade = row[3] ? row[3][0] : 'N';
                    enc.hasGrade = row[4] ? (stoi(row[4]) != 0) : false;
                    studentMap[sId].enrollments.push_back(enc);
                }
            }
            mysql_free_result(res);
        }
    }

    if (!mysql_query(conn, "SELECT * FROM Instructors")) {
        MYSQL_RES *res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                Instructor ins;
                ins.id = stoi(row[0]);
                ins.name = row[1] ? row[1] : "";
                ins.password = row[2] ? row[2] : "";
                ins.assignedCourseCode = row[3] ? row[3] : "";
                instructorMap[ins.id] = ins;
            }
            mysql_free_result(res);
        }
    }

    if (!mysql_query(conn, "SELECT * FROM Courses")) {
        MYSQL_RES *res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                Course c;
                c.code = row[0] ? row[0] : "";
                c.title = row[1] ? row[1] : "";
                c.creditHours = row[2] ? stoi(row[2]) : 3;
                courseMap[c.code] = c;
            }
            mysql_free_result(res);
        }
    }

    mysql_close(conn);
}

void GradeChange(int id, string course, float score) {
    ofstream file("grade.txt", ios::app);
    if (file.is_open()) {
        file << "Instructor changed student ID " << id << " in course " << course << " grade to: " << score << endl;
        file.close();
    }
}

char calculateLetterGrade(float score) {
    if (score >= 90) return 'A';
    if (score >= 80) return 'B';
    if (score >= 70) return 'C';
    if (score >= 60) return 'D';
    return 'F';
}

void printAllCourses() {
    cout << "\t Course List" << endl;
    if (courseMap.empty()) {
        cout << "No courses found in database" << endl;
        return;
    }
    for (auto const& [code, course] : courseMap) {
        cout << "Code: " << course.code << " | Title: " << course.title << " (" << course.creditHours << " Credits)" << endl;
    }
}

void studentMenu(int id) {
    int choice;
    while (true) {
        cout << " \t Student Menu: " << studentMap[id].name  << endl;
        cout << "1. Enroll in a Course\n2. View My Profile\n3. Download Modules\n4. Logout\nChoice: ";
        if (!(cin >> choice)) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            printAllCourses();
            string cCode = getNonEmptyString("Enter Course Code to join: ");
            if (courseMap.count(cCode)) {
                bool alreadyEnrolled = false;
                for (const auto& enc : studentMap[id].enrollments) {
                    if (enc.courseCode == cCode) {
                        alreadyEnrolled = true;
                        break;
                    }
                }
                if (alreadyEnrolled) {
                    cout << "Error: You are already enrolled in this course!" << endl;
                } else {
                    CourseEnrollment newEnc;
                    newEnc.courseCode = cCode;
                    newEnc.hasGrade = false;
                    newEnc.percentage = 0.0f;
                    newEnc.letterGrade = 'N';

                    studentMap[id].enrollments.push_back(newEnc);
                    saveStudentToDB(studentMap[id]);
                    cout << "Successfully enrolled in course!" << endl;
                }
            } else {
                cout << "Error: This course code does not exist!" << endl;
            }
        } else if (choice == 2) {
            cout << "\t MY PROFILE ";
            cout << "\nID: " << studentMap[id].id;
            cout << "\nName: " << studentMap[id].name;
            cout << "\n\n Enrolled Courses & Grades ";
            if (studentMap[id].enrollments.empty()) {
                cout << "\nNone";
            } else {
                for (const auto& enc : studentMap[id].enrollments) {
                    cout << "\nCourse: " << enc.courseCode;
                    if (enc.hasGrade) {
                        cout << "  Grade: " << enc.percentage << "% (" << enc.letterGrade << ")";
                    } else {
                        cout << "  Grade: Not graded yet";
                    }
                }
            }

        } else if (choice == 3) {
            if (studentMap[id].enrollments.empty()) {
                cout << "Error: Please enroll in at least one course first!" << endl;
            } else {
                cout << "\nYour courses: ";
                for (const auto& enc : studentMap[id].enrollments) cout << enc.courseCode << " ";
                string targetCourse = getNonEmptyString("\nWhich course module do you want to download? ");

                bool found = false;
                for (const auto& enc : studentMap[id].enrollments) {
                    if (enc.courseCode == targetCourse) {
                        found = true;
                        break;
                    }
                }

                if (found) {
                    string fileName = targetCourse + "_Module.txt";
                    ofstream downloadFile(fileName);
                    if (downloadFile.is_open()) {
                        downloadFile << " Learning Material for " << targetCourse << " \n";
                        downloadFile << "Welcome " << studentMap[id].name << "!\n";
                        downloadFile << "This is your downloaded course study guide.";
                        downloadFile.close();
                        cout << "Successfully downloaded real file as: " << fileName << "!" << endl;
                    } else {
                        cout << "Error: Could not create file." << endl;
                    }
                } else {
                    cout << "Error: You are not enrolled in that course!" << endl;
                }
            }
        } else if (choice == 4) {
            cout << "Logging out" << endl;
            break;
        }
    }
}

void instructorMenu(int insId) {
    int choice;
    while (true) {
        cout << "\n Instructor Menu: " << instructorMap[insId].name << " ---" << endl;
        cout << "Your Assigned Course: " << (instructorMap[insId].assignedCourseCode.empty() ? "None" : instructorMap[insId].assignedCourseCode) << endl;
        cout << "1. Give Student a Grade\n2. View My Students\n3. Logout\nChoice: ";
        if (!(cin >> choice)) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            int targetId = getValidInteger("Enter Student ID: ");
            if (studentMap.count(targetId)) {

                int matchIndex = -1;
                for (size_t i = 0; i < studentMap[targetId].enrollments.size(); ++i) {
                    if (studentMap[targetId].enrollments[i].courseCode == instructorMap[insId].assignedCourseCode) {
                        matchIndex = i;
                        break;
                    }
                }

                if (matchIndex == -1) {
                    cout << "Error: You cannot grade this student! They are not in your assigned course." << endl;
                    continue;
                }

                float score;
                while (true) {
                    cout << "Enter Grade Percentage (0 - 100): ";
                    if (cin >> score && score >= 0 && score <= 100) {
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        break;
                    }
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Error: Please enter a score between 0 and 100!\n";
                }

                studentMap[targetId].enrollments[matchIndex].percentage = score;
                studentMap[targetId].enrollments[matchIndex].letterGrade = calculateLetterGrade(score);
                studentMap[targetId].enrollments[matchIndex].hasGrade = true;

                saveStudentToDB(studentMap[targetId]);
                GradeChange(targetId, instructorMap[insId].assignedCourseCode, score);
                cout << "Successfully recorded!" << endl;
            } else {
                cout << "Error: Student does not exist!" << endl;
            }
        } else if (choice == 2) {
            cout << "\n Student list " << endl;
            bool found = false;
            for (auto const& [sid, student] : studentMap) {
                for (const auto& enc : student.enrollments) {
                    if (enc.courseCode == instructorMap[insId].assignedCourseCode) {
                        cout << "ID: " << student.id << " | Name: " << student.name;
                        if (enc.hasGrade) cout << " | Grade: " << enc.percentage << "%";
                        else cout << " | Grade: Not assigned";
                        cout << endl;
                        found = true;
                    }
                }
            }
            if (!found) cout << "No students are in your course yet." << endl;
        } else if (choice == 3) {
            cout << "Logging out" << endl;
            break;
        }
    }
}

void adminMenu() {
    string pass = getNonEmptyString("Enter Admin Password: ");
    if (pass != ADMIN_PASS) {
        cout << "Error: Wrong admin password!" << endl;
        return;
    }

    int choice;
    while (true) {
        cout << "\n  Admin Menu " << endl;
        cout << "1. Create a New Course\n2. Create a New Instructor\n3. Back to Main Menu\nChoice: ";
        if (!(cin >> choice)) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            Course c;
            c.code = getNonEmptyString("Enter New Course Code (e.g. CS101): ");
            if (courseMap.count(c.code)) {
                cout << "Error: This course already exists!" << endl;
                continue;
            }
            c.title = getNonEmptyString("Enter Course Title: ");
            while (true) {
                cout << "Enter Course Credits (1-5): ";
                if (cin >> c.creditHours && c.creditHours > 0 && c.creditHours <= 5) {
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Error: Enter a value between 1 and 5!\n";
            }
            courseMap[c.code] = c;
            saveCourseToDB(c);
            cout << "Successfully recorded!" << endl;
        } else if (choice == 2) {
            Instructor ins;
            ins.id = getValidInteger("Enter Instructor ID: ");
            if (instructorMap.count(ins.id)) {
                cout << "Error: The instructor already exists!" << endl;
                continue;
            }
            ins.name = getNonEmptyString("Enter Instructor Full Name: ");
            ins.password = getNonEmptyString("Enter Instructor Password: ");

            printAllCourses();
            ins.assignedCourseCode = getNonEmptyString("Enter Course Code to assign to this instructor: ");

            instructorMap[ins.id] = ins;
            saveInstructorToDB(ins);
            cout << "Successfully recorded!" << endl;
        } else if (choice == 3) {
            break;
        }
    }
}

int main() {
    initializeDatabaseTables();
    loadFromDB();

    int mainChoice;
    while (true) {
        cout << "\n   Learning Management System   " << endl;
        cout << "1. Register New Student\n2. Student Login\n3. Instructor Login\n4. Admin Menu\n5. Exit\nChoice: ";

        if (!(cin >> mainChoice)) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Error: Choose a valid number option!" << endl;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (mainChoice == 1) {
            Student s;
            s.id = getValidInteger("Enter Student ID: ");
            if (studentMap.count(s.id)) {
                cout << "Error: The student already exists!" << endl;
                continue;
            }

            s.name = getNonEmptyString("Enter Full Name: ");
            s.password = getNonEmptyString("Enter Password: ");

            studentMap[s.id] = s;
            saveStudentToDB(s);
            cout << "Successfully recorded!" << endl;

        } else if (mainChoice == 2) {
            int id = getValidInteger("Enter Student ID: ");
            string p = getNonEmptyString("Enter Password: ");

            if (studentMap.count(id) && studentMap[id].password == p) {
                studentMenu(id);
            } else {
                cout << "Error: Wrong ID or password!" << endl;
            }

        } else if (mainChoice == 3) {
            int id = getValidInteger("Enter Instructor ID: ");
            string p = getNonEmptyString("Enter Password: ");

            if (instructorMap.count(id) && instructorMap[id].password == p) {
                instructorMenu(id);
            } else {
                cout << "Error: Wrong ID or password!" << endl;
            }

        } else if (mainChoice == 4) {
            adminMenu();

        } else if (mainChoice == 5) {
            cout << "Goodbye!" << endl;
            break;
        } else {
            cout << "Error: Invalid selection!" << endl;
        }
    }
    return 0;
}
