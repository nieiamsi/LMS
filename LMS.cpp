#include <iostream>
#include <string>
#include <ctime>
using namespace std;
string currentuserid = "00000";
union grades
{
    char letter;
    float avg;
} struct student
{
    string id;
    string name;
    string email;
    char gender;
    int phonenumber;
    string password;
    grades grade;
    string status;
    int enroll[5];
    float mark[5];
} stud;
struct course
{
    int code[7];
    string name[7];
} void displaycourses()
{
    st.code = {1001, 1002, 1003, 1004, 1005, 1006, 1007};
    st.name = {"AI/ML", "Cloud Computing", "Cybersecurity", "Blockchain", "Data Science", "Introduction to Python", "Introduction to Javascript", "Computer Networking"};
    cout << "Enter the code for the courses you want to enroll.\nAI/ML(1001)\nCloud Computing(1002)\nBlockchain(1003)\nData Science(1004)\nIntroduction to Python(1005)\nIntroduction to Javascript(1006)\nComputer Networking(1007)\n";
}
void register()
{
    int n;
    ofstream file("student.txt", ios::app);
    cout << "Welcome, Register to continue.\n";
    cout << "Enter full name\n";
    cin.ignore();
    getline(cin, stud.name);
    cout << "Enter ID\n";
    cin >> stud.id;
    cout << "Enter gender (M/F)\n";
    cin >> stud.gender;
    cout << "Enter phone number\n";
    cin >> stud.phonenumber;
    cout << "Enter Email address\n";
    cin >> stud.email;
    cout << "Create password\n";
    cin >> stud.password;
    do
    {
        cout << "In how many courses do you want to enroll? (Max = 5, Min = 2)\n";
        cin >> n;
        if (n < 2 || n > 5)
        {
            cout << "Please enter valid amount of course\n";
        }
    } while (n < 2 || n > 5);
    displaycourses();
    for (int i = 0; i < n; i++)
    {

        cin >> stud.enroll[i];

        for (int j = o; j < +n; j++)
        {
            do
            {
                if (stud.enroll[i] == st.code[j])
                {
                    cout << "Registered for " << st.name[j] << "\n";
                }
                else if (j == n)
                {
                    cout << "The course code you entered is invalid. Try again.\n";
                }
            } while (j == n);
        }
        stud.mark[i] = 0.0;
    }
    stud.status = "Active";
    time_t now = time(0);
    char *dt = ctime(&now);
    file << stud.name << "\t" << stud.id << '\t' << stud.gender << "\t" << stud.phonenumber << "\t" << stud.email << "\t" << stud.password << "\t" << stud.status;
    for (int i = 0; i < n; i++)
    {
        file << "\t" << stud.enroll[i] << "\t" << stud.grade.mark[i];
    }
    file << dt;
    file.close();
    cout << "Registered Successfully!\n";
}
void login()
{
    string inputid, inputpass;
    ifstream file("student.txt");
    cout << "Enter ID\n";
    cin >> inputid;
    cout << "Enter password\n";
    cin >> inputpass;
    bool found = false;
    while (file >> fname >> fid >> fgender >> fphonenumber >> femail >> fpassword >> fstatus)
    {
        do
        {
            if (fid == inputid && fid == inputpass)
            {
                found = true;
                currentuserid = inputid;
                break;
            }
            if (found)
            {
                cout << "Login successfull! Welcome, " << fname;
            }
            else
            {
                cout << "Invalid ID or password. Try again.";
            }
        } while (!found);
    }
    file.close();
}