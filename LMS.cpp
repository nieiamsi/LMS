#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>

// MySQL Connector Headers (Based on your downloaded Connector/C++)
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

// --- Data Structures ---

struct Student
{
    string id, name, email, phone, course, password, grade = "N/A";
    float mark = 0.0;

    // Sorting logic: Sort students by ID
    bool operator<(const Student &other) const { return id < other.id; }
};

struct Instructor
{
    string username, password, courseCode;
};

// --- Global State ---
string orgName, orgPrefix;

// --- MySQL Logic ---
sql::Connection *connectDB()
{
    try
    {
        sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
        sql::Connection *con = driver->connect("tcp://127.0.0.1:3306", "root", "PASSWORD_HERE");
        con->setSchema("GlobalLMS");
        return con;
    }
    catch (sql::SQLException &e)
    {
        return nullptr; // Fallback to file-handling if DB is down
    }
}

// --- Utility Functions ---

void syncToMySQL(Student s)
{
    sql::Connection *con = connectDB();
    if (con)
    {
        sql::PreparedStatement *pstmt = con->prepareStatement(
            "INSERT INTO students (id, name, org, mark) VALUES (?, ?, ?, ?) ON DUPLICATE KEY UPDATE mark=?");
        pstmt->setString(1, s.id);
        pstmt->setString(2, s.name);
        pstmt->setString(3, orgPrefix);
        pstmt->setDouble(4, s.mark);
        pstmt->setDouble(5, s.mark);
        pstmt->execute();
        delete pstmt;
        delete con;
    }
}

string calculateGrade(float mark)
{
    if (mark >= 90)
        return "A+";
    if (mark >= 80)
        return "A";
    if (mark >= 70)
        return "B";
    if (mark >= 60)
        return "C";
    if (mark >= 50)
        return "D";
    return "F";
}

// --- core modules ---

void saveStudents(vector<Student> &students)
{
    sort(students.begin(), students.end()); // Keep file sorted by ID
    ofstream file(orgPrefix + "students.txt");
    for (const auto &s : students)
    {
        file << s.id << "|" << s.name << "|" << s.email << "|" << s.phone << "|"
             << s.course << "|" << s.password << "|" << s.mark << "|" << s.grade << endl;
    }
    file.close();
}

vector<Student> loadStudents()
{
    vector<Student> students;
    ifstream file(orgPrefix + "students.txt");
    string line;
    while (getline(file, line))
    {
        stringstream ss(line);
        Student s;
        string mStr;
        getline(ss, s.id, '|');
        getline(ss, s.name, '|');
        getline(ss, s.email, '|');
        getline(ss, s.phone, '|');
        getline(ss, s.course, '|');
        getline(ss, s.password, '|');
        getline(ss, mStr, '|');
        getline(ss, s.grade, '|');
        s.mark = stof(mStr);
        students.push_back(s);
    }
    return students;
}

// 1. Organization Phase: Upload Instructor CSV
void organizationSetup()
{
    cout << "\n--- Welcome to LMS Setup ---\nEnter Company Name: ";
    getline(cin >> ws, orgName);
    orgPrefix = orgName + "_";

    string csvFile;
    cout << "Enter Instructor CSV filename (e.g., inst.csv): ";
    cin >> csvFile;

    ifstream file(csvFile);
    if (!file.is_open())
    {
        cout << "Error: CSV not found. Proceeding with empty instructor list.\n";
        return;
    }

    ofstream instFile(orgPrefix + "instructors.txt");
    string line;
    while (getline(file, line))
    {
        instFile << line << endl; // Format: username,password,course
    }
    cout << "Download Successful. Credentials stored with prefix: " << orgPrefix << endl;
    instFile.close();
    file.close();
}

// 2. Student Registration
void studentRegistration()
{
    vector<Student> students = loadStudents();
    Student s;
    cout << "\n--- Student Registration ---\nID: ";
    cin >> s.id;
    cout << "Name: ";
    getline(cin >> ws, s.name);
    cout << "Email: ";
    cin >> s.email;
    cout << "Phone: ";
    cin >> s.phone;
    cout << "Course: ";
    cin >> s.course;
    cout << "Password: ";
    cin >> s.password;

    students.push_back(s);
    saveStudents(students);
    syncToMySQL(s);
    cout << "Registration Successful!\n";
}

// 3. Instructor Portal: CSV Upload and Mark Update
void instructorPortal()
{
    string user, pass, line;
    cout << "\n--- Instructor Login ---\nUsername: ";
    cin >> user;
    cout << "Password: ";
    cin >> pass;

    bool auth = false;
    ifstream instFile(orgPrefix + "instructors.txt");
    while (getline(instFile, line))
    {
        if (line.find(user) != string::npos && line.find(pass) != string::npos)
        {
            auth = true;
            break;
        }
    }
    if (!auth)
    {
        cout << "Invalid Login!\n";
        return;
    }

    int choice;
    vector<Student> students = loadStudents();
    cout << "1. Upload Marks CSV\n2. Manual Update Mark\nChoice: ";
    cin >> choice;

    if (choice == 1)
    {
        string mCsv;
        cout << "Enter Marks CSV filename: ";
        cin >> mCsv;
        ifstream mFile(mCsv);
        string id, markVal;
        while (getline(mFile, line))
        {
            stringstream ss(line);
            getline(ss, id, ',');
            getline(ss, markVal, ',');
            for (auto &s : students)
            {
                if (s.id == id)
                {
                    s.mark = stof(markVal);
                    s.grade = calculateGrade(s.mark);
                }
            }
        }
        cout << "Marks synced to " << orgPrefix << "students.txt successfully!\n";
    }
    else
    {
        string targetID;
        float newMark;
        cout << "Enter Student ID: ";
        cin >> targetID;
        cout << "Enter New Mark: ";
        cin >> newMark;
        for (auto &s : students)
        {
            if (s.id == targetID)
            {
                s.mark = newMark;
                s.grade = calculateGrade(s.mark);
            }
        }
    }
    saveStudents(students);
}

// 4. Student Dashboard
void studentDashboard()
{
    string id, pass;
    cout << "\n--- Student Login ---\nID: ";
    cin >> id;
    cout << "Password: ";
    cin >> pass;

    vector<Student> students = loadStudents();
    for (auto &s : students)
    {
        if (s.id == id && s.password == pass)
        {
            while (true)
            {
                cout << "\n--- Welcome " << s.name << " ---\n";
                cout << "Course: " << s.course << "\nMark: " << s.mark << " | Grade: " << s.grade << endl;
                cout << "1. Edit Profile\n2. Download Modules\n3. Logout\nChoice: ";
                int ch;
                cin >> ch;
                if (ch == 1)
                {
                    cout << "New Email: ";
                    cin >> s.email;
                    cout << "New Phone: ";
                    cin >> s.phone;
                    saveStudents(students);
                }
                else if (ch == 2)
                {
                    cout << "Downloading modules... Success!\n";
                }
                else
                    break;
            }
            return;
        }
    }
    cout << "Login Failed.\n";
}

int main()
{
    organizationSetup();
    while (true)
    {
        cout << "\n--- " << orgName << " LMS ---\n";
        cout << "1. Student Register\n2. Student Login\n3. Instructor Portal\n4. Exit\nChoice: ";
        int m;
        cin >> m;
        if (m == 1)
            studentRegistration();
        else if (m == 2)
            studentDashboard();
        else if (m == 3)
            instructorPortal();
        else
            break;
    }
    return 0;
}