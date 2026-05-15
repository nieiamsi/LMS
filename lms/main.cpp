#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <limits>
// UPDATED: Standard MySQL header for Connector/C
#include <mysql.h>

using namespace std;

union GradeData {
    float percentage;
    char letterGrade;
};

struct Student {
    int id;
    string name;
    string password;
    string enrolledCourse;
    GradeData grade;
    char calculatedLetter;
    bool hasGrade;
};

map<int, Student> studentMap;
const string ADMIN_PASS = "admin123";

// UPDATED: New Database Connection Helper using MYSQL pointer
MYSQL* connectDB() {
    MYSQL* conn = mysql_init(NULL);
    if (conn == NULL) return nullptr;

    // Use these credentials (matching your image_78b299.png)
    // The NULL, 0 at the end handles the modern connection handshake
    if (mysql_real_connect(conn, "127.0.0.1", "root", "@helu921my", "LMS_DB", 3306, NULL, 0)) {
        return conn;
    } else {
        cerr << "Connection Error: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return nullptr;
    }
}

// UPDATED: Modified saveToDB using standard MySQL functions
void saveToDB(Student s) {
    MYSQL* conn = connectDB();
    if (!conn) return;

    string query = "INSERT INTO Students (student_id, full_name, password, enrolled_course, numeric_grade, letter_grade, has_grade) "
                   "VALUES (" + to_string(s.id) + ", '" + s.name + "', '" + s.password + "', '" + s.enrolledCourse + "', " +
                   to_string(s.grade.percentage) + ", '" + s.calculatedLetter + "', " + (s.hasGrade ? "1" : "0") + ") "
                   "ON DUPLICATE KEY UPDATE enrolled_course=VALUES(enrolled_course), numeric_grade=VALUES(numeric_grade), "
                   "letter_grade=VALUES(letter_grade), has_grade=VALUES(has_grade)";

    if (mysql_query(conn, query.c_str())) {
        cerr << "Save Error: " << mysql_error(conn) << endl;
    }
    mysql_close(conn);
}

// UPDATED: Modified loadFromDB using standard MySQL functions
void loadFromDB() {
    MYSQL* conn = connectDB();
    if (!conn) return;

    if (mysql_query(conn, "SELECT * FROM Students")) {
        cerr << "Load Error: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(res))) {
        Student s;
        s.id = stoi(row[0]);
        s.name = row[1];
        s.password = row[2];
        s.enrolledCourse = row[3];
        s.grade.percentage = stof(row[4]);
        s.calculatedLetter = row[5][0];
        s.hasGrade = (stoi(row[6]) != 0);
        studentMap[s.id] = s;
    }

    mysql_free_result(res);
    mysql_close(conn);
}

// --- Rest of your code remains the same ---

void logGradeChange(int id, float score) {
    ofstream file("audit_log.txt", ios::app);
    if (file.is_open()) {
        file << "Instructor modified ID: " << id << " | New Mark: " << score << endl;
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

void studentMenu(int id) {
    int choice;
    while (true) {
        cout << "\n--- Dashboard: " << studentMap[id].name << " ---" << endl;
        cout << "1. Enroll in Course\n2. View Profile\n3. Download Modules\n4. Logout\nChoice: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            cout << "Enter Course Code: ";
            getline(cin, studentMap[id].enrolledCourse);
            saveToDB(studentMap[id]);
            cout << "Enrolled successfully!" << endl;
        } else if (choice == 2) {
            cout << "\n[ PROFILE ]" << "\nID: " << studentMap[id].id << "\nName: " << studentMap[id].name;
            cout << "\nCourse: " << (studentMap[id].enrolledCourse.empty() ? "None" : studentMap[id].enrolledCourse);
            if (studentMap[id].hasGrade)
                cout << "\nResult: " << studentMap[id].grade.percentage << "% (" << studentMap[id].calculatedLetter << ")";
            cout << endl;
        } else if (choice == 3) {
            cout << "Downloading... Success!" << endl;
        } else if (choice == 4) break;
    }
}

void instructorMenu() {
    string pass;
    cout << "Instructor Password: "; cin >> pass;
    if (pass != ADMIN_PASS) return;

    int choice;
    while (true) {
        cout << "\n--- Instructor Panel ---\n1. Update Student Grade\n2. Logout\nChoice: ";
        cin >> choice;
        if (choice == 1) {
            int id; float score;
            cout << "Enter Student ID: "; cin >> id;
            if (studentMap.count(id)) {
                cout << "Enter Mark: "; cin >> score;
                studentMap[id].grade.percentage = score;
                studentMap[id].calculatedLetter = calculateLetterGrade(score);
                studentMap[id].hasGrade = true;
                saveToDB(studentMap[id]);
                logGradeChange(id, score);
                cout << "Grade updated and synced to DB." << endl;
            } else cout << "Not found." << endl;
        } else if (choice == 2) break;
    }
}

int main() {
    loadFromDB();
    int mainChoice;
    while (true) {
        cout << "\n--- LMS Login ---\n1. Registration\n2. Student Login\n3. Instructor Login\n4. Exit\nChoice: ";
        cin >> mainChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (mainChoice == 1) {
            Student s;
            cout << "ID: "; cin >> s.id; cin.ignore();
            if (studentMap.count(s.id)) { cout << "ID exists!" << endl; continue; }
            cout << "Full Name: "; getline(cin, s.name);
            cout << "Password: "; getline(cin, s.password);
            s.enrolledCourse = ""; s.hasGrade = false; s.grade.percentage = 0; s.calculatedLetter = 'N';
            studentMap[s.id] = s;
            saveToDB(s);
            cout << "Registered and saved to DB." << endl;
        } else if (mainChoice == 2) {
            int id; string p;
            cout << "ID: "; cin >> id; cin.ignore();
            cout << "Password: "; getline(cin, p);
            if (studentMap.count(id) && studentMap[id].password == p) studentMenu(id);
            else cout << "Invalid credentials." << endl;
        } else if (mainChoice == 3) instructorMenu();
        else if (mainChoice == 4) break;
    }
    return 0;
}
