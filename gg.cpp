/*
    ============================================================
    Employee Attendance Management System (EAMS)
    ------------------------------------------------------------
    Role-based version: one login screen routes the user to
    either the Admin Dashboard or their own Employee Dashboard.

    ADMIN can:
      - Add / View / Search / Update / Remove employees
      - View attendance (per employee, all records, or by date)
      - View & decide on leave requests (Approve / Reject)
      - Correct attendance records directly
      - View a full per-employee record (profile + attendance + leave)
      - Generate an attendance report (with file export)
      - Change the admin password

    EMPLOYEE can:
      - Mark today's attendance (once per day)
      - View their own attendance history
      - Apply for leave
      - View the status of their leave requests
      - View their own profile

    Data is stored in plain tab-delimited text files so nothing
    is lost when the program closes:
      employees.txt  - employee records (including password)
      attendance.txt - attendance records
      leaves.txt     - leave requests
      config.txt     - admin username/password

    Approving a leave request automatically sets that employee's
    attendance for that date to "Leave".

    Compile (Windows / MinGW):  g++ -std=c++17 -O2 eams.cpp -o eams.exe
    Compile (Linux / macOS):    g++ -std=c++17 -O2 eams.cpp -o eams
    Default admin login:        admin / 1234
    ============================================================
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <cctype>
#include <cstdlib>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

// ============================================================
// Cross platform console helpers
// ============================================================
#ifndef _WIN32
// Minimal stand-in for conio.h's _getch() on Linux/macOS so the
// same masked-password code works outside Windows too. Falls
// back to plain getchar() if stdin isn't a real terminal.
int _getch()
{
    termios oldt;
    if (tcgetattr(STDIN_FILENO, &oldt) != 0)
        return getchar();
    termios newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    int ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

void cls()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseScreen()
{
#ifdef _WIN32
    system("pause");
#else
    cout << "Press any key to continue . . .";
    _getch();
    cout << endl;
#endif
}

void line(int l = 100)
{
    cout << string(l, '=') << endl;
}

void printCentered(const string &text, int width = 100)
{
    int padding = (width - (int)text.length()) / 2;
    if (padding > 0)
        cout << string(padding, ' ');
    cout << text << endl;
}

void showBox(const string &msg)
{
    line();
    printCentered(msg);
    line();
}

// Pads (or, for values too long to fit, truncates with a trailing
// space) so table columns always stay aligned - plain setw() only
// pads short values and lets long ones collide with the next column.
string fitWidth(const string &s, size_t width)
{
    if (s.length() >= width)
        return width > 1 ? s.substr(0, width - 1) + " " : s.substr(0, width);
    return s + string(width - s.length(), ' ');
}

string fitWidth(int n, size_t width)
{
    return fitWidth(to_string(n), width);
}

string hidePassword()
{
    string password;
    while (true)
    {
        int ch = _getch();

        if (ch == 13 || ch == 10 || ch == EOF) // Enter
            break;

        if (ch == 8 || ch == 127) // Backspace
        {
            if (!password.empty())
            {
                password.pop_back();
                cout << "\b \b";
            }
            continue;
        }

        password += (char)ch;
        cout << "*";
    }
    cout << endl;
    return password;
}

int getIntInput(const string &prompt,
                 int minVal = numeric_limits<int>::min(),
                 int maxVal = numeric_limits<int>::max())
{
    int value;
    while (true)
    {
        cout << prompt;
        if (cin >> value && value >= minVal && value <= maxVal)
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        if (cin.eof())
        {
            cout << "\nInput closed unexpectedly. Exiting.\n";
            exit(0);
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a whole number";
        if (minVal != numeric_limits<int>::min() || maxVal != numeric_limits<int>::max())
            cout << " between " << minVal << " and " << maxVal;
        cout << ".\n";
    }
}

string getLineInput(const string &prompt)
{
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

string getRequiredLineInput(const string &prompt)
{
    string value;
    do
    {
        value = getLineInput(prompt);
        if (value.empty())
            cout << "This field cannot be empty. Please enter a value.\n";
    } while (value.empty());
    return value;
}

char getCharInput(const string &prompt)
{
    char value;
    cout << prompt;
    if (!(cin >> value))
    {
        cout << "\nInput closed unexpectedly. Exiting.\n";
        exit(0);
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return value;
}

// Format: YYYY-MM-DD
string getCurrentDate()
{
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    ostringstream oss;
    oss << (ltm->tm_year + 1900) << "-"
        << setfill('0') << setw(2) << (ltm->tm_mon + 1) << "-"
        << setfill('0') << setw(2) << ltm->tm_mday;
    return oss.str();
}

string toLowerStr(string s)
{
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return (char)tolower(c); });
    return s;
}

int welcomeMenu()
{
    cls();
    line();
    printCentered("|| EMPLOYEE ATTENDANCE MANAGEMENT SYSTEM (EAMS) ||");
    printCentered("Today: " + getCurrentDate());
    line();
    cout << "1) Login\n"
         << "2) Exit\n";
    line();
    return getIntInput("Enter your choice: ", 1, 2);
}

// ============================================================
// Data models
// ============================================================
struct Employee
{
    string id;
    string name;
    string password;
    int age = 0;
    string department;
    string position;
    string contact;
};

struct AttendanceRecord
{
    string empId;
    string empName;
    string date;
    string status; // Present / Absent / Leave
};

struct LeaveRequest
{
    int requestId = 0;
    string empId;
    string empName;
    string leaveDate;
    string reason;
    string leaveType;
    string status; // Pending / Approved / Rejected
};

enum class Role
{
    NONE,
    ADMIN,
    EMPLOYEE
};

// No default member initializers here on purpose: LoginResult is
// always built with all three fields via brace-init, and leaving
// it a plain aggregate keeps that portable across compilers.
struct LoginResult
{
    Role role;
    string employeeId;
    string employeeName;
};

// ============================================================
// Core system
// ============================================================
class EAMS
{
private:
    vector<Employee> employees;
    vector<AttendanceRecord> attendance;
    vector<LeaveRequest> leaves;
    string adminUser, adminPass;

    const string EMP_FILE = "employees.txt";
    const string ATT_FILE = "attendance.txt";
    const string LEAVE_FILE = "leaves.txt";
    const string CFG_FILE = "config.txt";

    static vector<string> splitFields(const string &ln, char delim)
    {
        vector<string> parts;
        stringstream ss(ln);
        string field;
        while (getline(ss, field, delim))
            parts.push_back(field);
        return parts;
    }

public:
    EAMS()
    {
        loadAdminCredentials();
        loadEmployees();
        loadAttendance();
        loadLeaves();
    }

    // ---------------- Persistence ----------------
    void loadAdminCredentials()
    {
        adminUser = "admin";
        adminPass = "1234";
        ifstream cred(CFG_FILE);
        if (cred)
        {
            string u, p;
            if (getline(cred, u) && getline(cred, p) && !u.empty() && !p.empty())
            {
                adminUser = u;
                adminPass = p;
            }
        }
    }

    void loadEmployees()
    {
        employees.clear();
        ifstream fin(EMP_FILE);
        if (!fin)
            return;

        string ln;
        while (getline(fin, ln))
        {
            if (ln.empty())
                continue;
            vector<string> f = splitFields(ln, '\t');
            if (f.size() < 7)
                continue;

            Employee e;
            e.id = f[0];
            e.name = f[1];
            e.password = f[2];
            try
            {
                e.age = stoi(f[3]);
            }
            catch (...)
            {
                e.age = 0;
            }
            e.department = f[4];
            e.position = f[5];
            e.contact = f[6];
            employees.push_back(e);
        }
    }

    void saveEmployees()
    {
        ofstream fout(EMP_FILE, ios::trunc);
        for (auto &e : employees)
        {
            fout << e.id << "\t" << e.name << "\t" << e.password << "\t" << e.age << "\t"
                 << e.department << "\t" << e.position << "\t" << e.contact << "\n";
        }
    }

    void loadAttendance()
    {
        attendance.clear();
        ifstream fin(ATT_FILE);
        if (!fin)
            return;

        string ln;
        while (getline(fin, ln))
        {
            if (ln.empty())
                continue;
            vector<string> f = splitFields(ln, '\t');
            if (f.size() < 4)
                continue;

            AttendanceRecord a;
            a.empId = f[0];
            a.empName = f[1];
            a.date = f[2];
            a.status = f[3];
            attendance.push_back(a);
        }
    }

    void saveAttendance()
    {
        ofstream fout(ATT_FILE, ios::trunc);
        for (auto &a : attendance)
            fout << a.empId << "\t" << a.empName << "\t" << a.date << "\t" << a.status << "\n";
    }

    void loadLeaves()
    {
        leaves.clear();
        ifstream fin(LEAVE_FILE);
        if (!fin)
            return;

        string ln;
        while (getline(fin, ln))
        {
            if (ln.empty())
                continue;
            vector<string> f = splitFields(ln, '\t');
            if (f.size() < 7)
                continue;

            LeaveRequest lr;
            try
            {
                lr.requestId = stoi(f[0]);
            }
            catch (...)
            {
                continue;
            }
            lr.empId = f[1];
            lr.empName = f[2];
            lr.leaveDate = f[3];
            lr.reason = f[4];
            lr.leaveType = f[5];
            lr.status = f[6];
            leaves.push_back(lr);
        }
    }

    void saveLeaves()
    {
        ofstream fout(LEAVE_FILE, ios::trunc);
        for (auto &lr : leaves)
        {
            fout << lr.requestId << "\t" << lr.empId << "\t" << lr.empName << "\t"
                 << lr.leaveDate << "\t" << lr.reason << "\t" << lr.leaveType << "\t" << lr.status << "\n";
        }
    }

    // ---------------- Helpers ----------------
    Employee *findEmployeeById(const string &id)
    {
        for (auto &e : employees)
            if (e.id == id)
                return &e;
        return nullptr;
    }

    bool isEmployeeIdUnique(const string &id)
    {
        return findEmployeeById(id) == nullptr;
    }

    int nextRequestId()
    {
        int mx = 0;
        for (auto &lr : leaves)
            if (lr.requestId > mx)
                mx = lr.requestId;
        return mx + 1;
    }

    AttendanceRecord *findAttendance(const string &empId, const string &date)
    {
        for (auto &a : attendance)
            if (a.empId == empId && a.date == date)
                return &a;
        return nullptr;
    }

    string askAttendanceStatus()
    {
        cout << "\nAttendance status:\n"
             << " 1) Present\n"
             << " 2) Absent\n"
             << " 3) Leave\n";
        int s = getIntInput("Select status: ", 1, 3);
        switch (s)
        {
        case 1:
            return "Present";
        case 2:
            return "Absent";
        default:
            return "Leave";
        }
    }

    string askLeaveType()
    {
        cout << "\nLeave type:\n"
             << " 1) Sick Leave\n"
             << " 2) Casual Leave\n"
             << " 3) Other\n";
        int t = getIntInput("Select leave type: ", 1, 3);
        switch (t)
        {
        case 1:
            return "Sick Leave";
        case 2:
            return "Casual Leave";
        default:
            return "Other";
        }
    }

    void logoutMessage()
    {
        cls();
        cout << "Logging out...\n";
        pauseScreen();
    }

    // ================= LOGIN =================
    LoginResult login()
    {
        int attemptsLeft = 3;
        while (attemptsLeft > 0)
        {
            cls();
            line();
            printCentered("|| EAMS - LOGIN ||");
            line();

            string id = getLineInput("User ID: ");
            cout << "Password: ";
            string pass = hidePassword();

            if (id == adminUser && pass == adminPass)
            {
                showBox("|| Welcome, Admin ||");
                pauseScreen();
                return LoginResult{Role::ADMIN, "", "Admin"};
            }

            Employee *e = findEmployeeById(id);
            if (e && e->password == pass)
            {
                showBox("|| Welcome, " + e->name + " ||");
                pauseScreen();
                return LoginResult{Role::EMPLOYEE, e->id, e->name};
            }

            attemptsLeft--;
            line();
            printCentered("|| Invalid User ID or Password ||");
            printCentered("(" + to_string(attemptsLeft) + " attempt(s) left)");
            line();
            pauseScreen();
        }

        cls();
        line();
        printCentered("|| Too Many Failed Attempts - Access Denied ||");
        line();
        pauseScreen();
        return LoginResult{Role::NONE, "", ""};
    }

    // ================= ADMIN DASHBOARD =================
    void adminDashboard()
    {
        while (true)
        {
            cls();
            line();
            printCentered("|| ADMIN DASHBOARD ||");
            printCentered("Today: " + getCurrentDate() + "   |   Total Employees: " + to_string(employees.size()));
            line();
            cout << " 1) Add Employee\n"
                 << " 2) View All Employees\n"
                 << " 3) Search Employee\n"
                 << " 4) View Employee Attendance\n"
                 << " 5) View Leave Requests\n"
                 << " 6) Approve Leave\n"
                 << " 7) Reject Leave\n"
                 << " 8) Update Employee Information\n"
                 << " 9) Update Attendance Records\n"
                 << "10) View Employee Records\n"
                 << "11) Remove Employee\n"
                 << "12) Attendance Report\n"
                 << "13) Change Admin Password\n"
                 << "14) Logout\n";
            line();
            int choice = getIntInput("Enter number to select given option: ");

            switch (choice)
            {
            case 1:
                addEmployee();
                break;
            case 2:
                viewEmployees();
                break;
            case 3:
                searchEmployee();
                break;
            case 4:
                viewEmployeeAttendance();
                break;
            case 5:
                viewLeaveRequests();
                break;
            case 6:
                approveLeave();
                break;
            case 7:
                rejectLeave();
                break;
            case 8:
                updateEmployeeInfo();
                break;
            case 9:
                updateAttendanceRecords();
                break;
            case 10:
                viewEmployeeRecords();
                break;
            case 11:
                removeEmployee();
                break;
            case 12:
                attendanceReport();
                break;
            case 13:
                changeAdminPassword();
                break;
            case 14:
                logoutMessage();
                return;
            default:
                cls();
                showBox("|| Invalid Input ||");
                pauseScreen();
            }
        }
    }

    // ---------------- Employee management (admin side) ----------------
    void addEmployee()
    {
        cls();
        line();
        printCentered("|| Add New Employee ||");
        line();

        string id;
        while (true)
        {
            id = getRequiredLineInput("Enter new Employee ID (e.g. EMP001): ");
            if (id == adminUser)
            {
                cout << "That ID is reserved for the admin account. Please choose another.\n";
                continue;
            }
            if (!isEmployeeIdUnique(id))
            {
                cout << "Invalid Employee ID - that ID is already in use. Please choose another.\n";
                continue;
            }
            break;
        }

        Employee e;
        e.id = id;
        e.name = getRequiredLineInput("Enter employee name: ");

        string pass;
        do
        {
            cout << "Set a password for this employee: ";
            pass = hidePassword();
            if (pass.empty())
                cout << "Password cannot be empty. Please try again.\n";
        } while (pass.empty());
        e.password = pass;

        e.age = getIntInput("Enter employee age: ", 15, 100);
        e.department = getLineInput("Enter department: ");
        e.position = getLineInput("Enter position/job: ");
        e.contact = getLineInput("Enter contact information: ");

        employees.push_back(e);
        saveEmployees();

        showBox("|| Employee Added Successfully (ID: " + e.id + ") ||");
        pauseScreen();
    }

    void printEmployeeTableHeader()
    {
        cout << fitWidth("ID", 10) << fitWidth("NAME", 18) << fitWidth("AGE", 5)
             << fitWidth("DEPARTMENT", 16) << fitWidth("POSITION", 16) << "CONTACT" << endl;
        line();
    }

    void printEmployeeRow(const Employee &e)
    {
        cout << fitWidth(e.id, 10) << fitWidth(e.name, 18) << fitWidth(e.age, 5)
             << fitWidth(e.department, 16) << fitWidth(e.position, 16) << e.contact << endl;
    }

    void viewEmployees()
    {
        cls();
        line();
        if (employees.empty())
        {
            printCentered("|| No Employees Found ||");
            line();
            pauseScreen();
            return;
        }

        printCentered("|| All Employees ||");
        line();
        printEmployeeTableHeader();
        for (auto &e : employees)
            printEmployeeRow(e);
        line();
        pauseScreen();
    }

    void searchEmployee()
    {
        cls();
        line();
        printCentered("|| Search Employee ||");
        line();

        if (employees.empty())
        {
            printCentered("|| No Employees Found ||");
            line();
            pauseScreen();
            return;
        }

        cout << "1) Search by Employee ID\n2) Search by Name\n";
        int choice = getIntInput("Choose an option: ", 1, 2);

        vector<Employee *> results;
        if (choice == 1)
        {
            string id = getLineInput("Enter Employee ID: ");
            Employee *e = findEmployeeById(id);
            if (e)
                results.push_back(e);
        }
        else
        {
            string key = toLowerStr(getLineInput("Enter name (or part of it): "));
            for (auto &e : employees)
                if (toLowerStr(e.name).find(key) != string::npos)
                    results.push_back(&e);
        }

        cls();
        line();
        if (results.empty())
        {
            printCentered("|| No Matching Employee ||");
        }
        else
        {
            printCentered("|| Search Results ||");
            line();
            printEmployeeTableHeader();
            for (auto *e : results)
                printEmployeeRow(*e);
        }
        line();
        pauseScreen();
    }

    void updateEmployeeInfo()
    {
        cls();
        line();
        printCentered("|| Update Employee Information ||");
        line();

        if (employees.empty())
        {
            printCentered("|| No Employees Found ||");
            line();
            pauseScreen();
            return;
        }

        string id = getLineInput("Enter Employee ID to update: ");
        Employee *e = findEmployeeById(id);
        if (!e)
        {
            showBox("|| Invalid Employee ID - Employee Not Found ||");
            pauseScreen();
            return;
        }

        cout << "\nEditing \"" << e->name << "\" (" << e->id << ") - press Enter to keep the current value.\n\n";

        string input = getLineInput("Name [" + e->name + "]: ");
        if (!input.empty())
            e->name = input;

        cout << "Password [leave blank to keep current]: ";
        string newPass = hidePassword();
        if (!newPass.empty())
            e->password = newPass;

        input = getLineInput("Age [" + to_string(e->age) + "]: ");
        if (!input.empty())
        {
            try
            {
                int newAge = stoi(input);
                if (newAge > 0 && newAge < 150)
                    e->age = newAge;
                else
                    cout << "Age out of range, keeping previous value.\n";
            }
            catch (...)
            {
                cout << "Invalid age ignored.\n";
            }
        }

        input = getLineInput("Department [" + e->department + "]: ");
        if (!input.empty())
            e->department = input;

        input = getLineInput("Position [" + e->position + "]: ");
        if (!input.empty())
            e->position = input;

        input = getLineInput("Contact [" + e->contact + "]: ");
        if (!input.empty())
            e->contact = input;

        // Keep the denormalized name in attendance/leave records in sync
        for (auto &a : attendance)
            if (a.empId == e->id)
                a.empName = e->name;
        for (auto &lr : leaves)
            if (lr.empId == e->id)
                lr.empName = e->name;

        saveEmployees();
        saveAttendance();
        saveLeaves();

        showBox("|| Employee Updated ||");
        pauseScreen();
    }

    void removeEmployee()
    {
        cls();
        line();
        printCentered("|| Remove Employee ||");
        line();

        if (employees.empty())
        {
            printCentered("|| No Employees Found ||");
            line();
            pauseScreen();
            return;
        }

        string id = getLineInput("Enter Employee ID to remove: ");
        Employee *e = findEmployeeById(id);
        if (!e)
        {
            showBox("|| Invalid Employee ID - Employee Not Found ||");
            pauseScreen();
            return;
        }

        cout << "\nName: " << e->name << "   Department: " << e->department << endl;
        char confirm = getCharInput("Are you sure you want to remove this employee? (Y/N): ");

        if (confirm == 'y' || confirm == 'Y')
        {
            employees.erase(remove_if(employees.begin(), employees.end(),
                                       [&id](const Employee &emp)
                                       { return emp.id == id; }),
                             employees.end());
            saveEmployees();

            char removeHist = getCharInput("Also remove this employee's attendance and leave history? (Y/N): ");
            if (removeHist == 'y' || removeHist == 'Y')
            {
                attendance.erase(remove_if(attendance.begin(), attendance.end(),
                                            [&id](const AttendanceRecord &a)
                                            { return a.empId == id; }),
                                  attendance.end());
                leaves.erase(remove_if(leaves.begin(), leaves.end(),
                                        [&id](const LeaveRequest &lr)
                                        { return lr.empId == id; }),
                              leaves.end());
                saveAttendance();
                saveLeaves();
            }
            showBox("|| Employee Removed ||");
        }
        else
        {
            showBox("|| Removal Cancelled ||");
        }
        pauseScreen();
    }

    // ---------------- Attendance (admin side) ----------------
    void printAttendanceTableHeader(bool showEmpColumns)
    {
        if (showEmpColumns)
            cout << fitWidth("ID", 10) << fitWidth("NAME", 18) << fitWidth("DATE", 14) << "STATUS" << endl;
        else
            cout << fitWidth("DATE", 14) << "STATUS" << endl;
        line();
    }

    void printAttendanceRowFull(const AttendanceRecord &a)
    {
        cout << fitWidth(a.empId, 10) << fitWidth(a.empName, 18) << fitWidth(a.date, 14) << a.status << endl;
    }

    void printAttendanceRowSimple(const AttendanceRecord &a)
    {
        cout << fitWidth(a.date, 14) << a.status << endl;
    }

    void viewEmployeeAttendance()
    {
        cls();
        line();
        printCentered("|| View Employee Attendance ||");
        line();

        if (attendance.empty())
        {
            printCentered("|| No Attendance Records Found ||");
            line();
            pauseScreen();
            return;
        }

        cout << "1) View by Specific Employee\n"
             << "2) View All Attendance Records\n"
             << "3) View by Date\n";
        int choice = getIntInput("Choose an option: ", 1, 3);

        if (choice == 1)
        {
            string id = getLineInput("Enter Employee ID: ");
            Employee *e = findEmployeeById(id);
            cls();
            line();
            if (!e)
            {
                showBox("|| Invalid Employee ID - Employee Not Found ||");
                pauseScreen();
                return;
            }
            printCentered("|| Attendance for " + e->name + " (" + e->id + ") ||");
            line();
            printAttendanceTableHeader(false);
            int p = 0, ab = 0, lv = 0;
            for (auto &a : attendance)
            {
                if (a.empId == id)
                {
                    printAttendanceRowSimple(a);
                    if (a.status == "Present")
                        p++;
                    else if (a.status == "Absent")
                        ab++;
                    else if (a.status == "Leave")
                        lv++;
                }
            }
            int total = p + ab + lv;
            line();
            if (total == 0)
                cout << "No attendance records found for this employee.\n";
            else
            {
                double pct = (double)p / total * 100.0;
                cout << fixed << setprecision(2);
                cout << "Present: " << p << "   Absent: " << ab << "   Leave: " << lv
                     << "   |   Attendance: " << pct << "%\n";
            }
            line();
        }
        else if (choice == 2)
        {
            cls();
            line();
            printCentered("|| All Attendance Records ||");
            line();
            printAttendanceTableHeader(true);
            for (auto &a : attendance)
                printAttendanceRowFull(a);
            line();
        }
        else
        {
            string date = getLineInput("Enter date (YYYY-MM-DD): ");
            cls();
            line();
            printCentered("|| Attendance on " + date + " ||");
            line();
            printAttendanceTableHeader(true);
            bool found = false;
            for (auto &a : attendance)
            {
                if (a.date == date)
                {
                    printAttendanceRowFull(a);
                    found = true;
                }
            }
            if (!found)
                cout << "No attendance records found for this date.\n";
            line();
        }
        pauseScreen();
    }

    void updateAttendanceRecords()
    {
        cls();
        line();
        printCentered("|| Update Attendance Records ||");
        line();

        if (employees.empty())
        {
            printCentered("|| No Employees Found ||");
            line();
            pauseScreen();
            return;
        }

        string id = getLineInput("Enter Employee ID: ");
        Employee *e = findEmployeeById(id);
        if (!e)
        {
            showBox("|| Invalid Employee ID - Employee Not Found ||");
            pauseScreen();
            return;
        }

        string date = getRequiredLineInput("Enter date to update (YYYY-MM-DD): ");
        AttendanceRecord *rec = findAttendance(id, date);

        if (rec)
        {
            cout << "\nCurrent status for " << e->name << " on " << date << ": " << rec->status << "\n";
        }
        else
        {
            cout << "\nNo existing attendance record for " << e->name << " on " << date << ".\n";
            char create = getCharInput("Create a new record for this date? (Y/N): ");
            if (create != 'y' && create != 'Y')
            {
                pauseScreen();
                return;
            }
        }

        string newStatus = askAttendanceStatus();

        if (rec)
        {
            rec->status = newStatus;
        }
        else
        {
            AttendanceRecord newRec;
            newRec.empId = e->id;
            newRec.empName = e->name;
            newRec.date = date;
            newRec.status = newStatus;
            attendance.push_back(newRec);
        }
        saveAttendance();

        showBox("|| Attendance Record Updated ||");
        pauseScreen();
    }

    // ---------------- Leave management (admin side) ----------------
    void printLeaveTableHeader()
    {
        cout << fitWidth("REQ#", 6) << fitWidth("EMP ID", 10) << fitWidth("NAME", 16)
             << fitWidth("DATE", 12) << fitWidth("REASON", 22) << fitWidth("TYPE", 14) << "STATUS" << endl;
        line();
    }

    void printLeaveRow(const LeaveRequest &lr)
    {
        cout << fitWidth(lr.requestId, 6) << fitWidth(lr.empId, 10) << fitWidth(lr.empName, 16)
             << fitWidth(lr.leaveDate, 12) << fitWidth(lr.reason, 22) << fitWidth(lr.leaveType, 14) << lr.status << endl;
    }

    void viewLeaveRequests()
    {
        cls();
        line();
        printCentered("|| All Leave Requests ||");
        line();

        if (leaves.empty())
        {
            printCentered("|| No Leave Requests Found ||");
            line();
            pauseScreen();
            return;
        }

        printLeaveTableHeader();
        for (auto &lr : leaves)
            printLeaveRow(lr);
        line();
        pauseScreen();
    }

    // Shared by Approve Leave and Reject Leave - only the resulting
    // status (and the action's title) differ between the two.
    void processLeaveDecision(const string &newStatus, const string &actionLabel)
    {
        cls();
        line();
        printCentered("|| " + actionLabel + " ||");
        line();

        vector<LeaveRequest *> pending;
        for (auto &lr : leaves)
            if (lr.status == "Pending")
                pending.push_back(&lr);

        if (pending.empty())
        {
            printCentered("|| No Pending Leave Requests ||");
            line();
            pauseScreen();
            return;
        }

        printLeaveTableHeader();
        for (auto *lr : pending)
            printLeaveRow(*lr);
        line();

        int reqId = getIntInput("Enter Request # to " + toLowerStr(actionLabel) + ": ");
        LeaveRequest *target = nullptr;
        for (auto *lr : pending)
            if (lr->requestId == reqId)
            {
                target = lr;
                break;
            }

        if (!target)
        {
            showBox("|| Request Not Found (or Already Decided) ||");
            pauseScreen();
            return;
        }

        target->status = newStatus;
        saveLeaves();

        if (newStatus == "Approved")
        {
            // Reflect the approved leave directly in the attendance record
            AttendanceRecord *rec = findAttendance(target->empId, target->leaveDate);
            if (rec)
            {
                rec->status = "Leave";
            }
            else
            {
                AttendanceRecord newRec;
                newRec.empId = target->empId;
                newRec.empName = target->empName;
                newRec.date = target->leaveDate;
                newRec.status = "Leave";
                attendance.push_back(newRec);
            }
            saveAttendance();
        }

        showBox("|| Leave Request " + newStatus + " ||");
        pauseScreen();
    }

    void approveLeave() { processLeaveDecision("Approved", "Approve Leave"); }
    void rejectLeave() { processLeaveDecision("Rejected", "Reject Leave"); }

    // ---------------- Combined record view ----------------
    void viewEmployeeRecords()
    {
        cls();
        line();
        printCentered("|| View Employee Records ||");
        line();

        if (employees.empty())
        {
            printCentered("|| No Employees Found ||");
            line();
            pauseScreen();
            return;
        }

        string id = getLineInput("Enter Employee ID: ");
        Employee *e = findEmployeeById(id);
        cls();
        line();
        if (!e)
        {
            showBox("|| Invalid Employee ID - Employee Not Found ||");
            pauseScreen();
            return;
        }

        printCentered("|| Full Record: " + e->name + " (" + e->id + ") ||");
        line();
        cout << "Name       : " << e->name << "\n"
             << "Age        : " << e->age << "\n"
             << "Department : " << e->department << "\n"
             << "Position   : " << e->position << "\n"
             << "Contact    : " << e->contact << "\n";
        line();

        printCentered("Attendance History");
        line();
        printAttendanceTableHeader(false);
        int p = 0, ab = 0, lv = 0;
        for (auto &a : attendance)
        {
            if (a.empId == id)
            {
                printAttendanceRowSimple(a);
                if (a.status == "Present")
                    p++;
                else if (a.status == "Absent")
                    ab++;
                else if (a.status == "Leave")
                    lv++;
            }
        }
        int total = p + ab + lv;
        if (total == 0)
            cout << "(No attendance records)\n";
        else
        {
            double pct = (double)p / total * 100.0;
            cout << fixed << setprecision(2);
            cout << "Present: " << p << "  Absent: " << ab << "  Leave: " << lv
                 << "  (" << pct << "% present)\n";
        }
        line();

        printCentered("Leave History");
        line();
        bool anyLeave = false;
        for (auto &lr : leaves)
        {
            if (lr.empId == id)
            {
                cout << fitWidth(lr.leaveDate, 12) << fitWidth(lr.leaveType, 14)
                     << fitWidth(lr.status, 10) << lr.reason << endl;
                anyLeave = true;
            }
        }
        if (!anyLeave)
            cout << "(No leave requests)\n";
        line();

        pauseScreen();
    }

    // ---------------- Reporting ----------------
    void printReportHeader(ostream &os)
    {
        os << fitWidth("ID", 10) << fitWidth("NAME", 18)
           << fitWidth("PRESENT", 9) << fitWidth("ABSENT", 8) << fitWidth("LEAVE", 8) << "PERCENT" << "\n";
    }

    void printReportRow(ostream &os, const Employee &e)
    {
        int p = 0, ab = 0, lv = 0;
        for (auto &rec : attendance)
        {
            if (rec.empId == e.id)
            {
                if (rec.status == "Present")
                    p++;
                else if (rec.status == "Absent")
                    ab++;
                else if (rec.status == "Leave")
                    lv++;
            }
        }
        int total = p + ab + lv;
        double pct = total == 0 ? 0.0 : (double)p / total * 100.0;

        ostringstream pctStr;
        pctStr << fixed << setprecision(2) << pct << "%";

        os << fitWidth(e.id, 10) << fitWidth(e.name, 18)
           << fitWidth(p, 9) << fitWidth(ab, 8) << fitWidth(lv, 8) << pctStr.str() << "\n";
    }

    void attendanceReport()
    {
        cls();
        line();
        printCentered("|| Attendance Report ||");
        line();

        if (employees.empty())
        {
            printCentered("|| No Employees Found ||");
            line();
            pauseScreen();
            return;
        }

        printReportHeader(cout);
        line();
        for (auto &e : employees)
            printReportRow(cout, e);
        line();

        char exportChoice = getCharInput("Export this report to report.txt? (Y/N): ");
        if (exportChoice == 'y' || exportChoice == 'Y')
        {
            ofstream rpt("report.txt", ios::trunc);
            rpt << "Employee Attendance Report - Generated on " << getCurrentDate() << "\n";
            rpt << string(60, '=') << "\n";
            printReportHeader(rpt);
            rpt << string(60, '=') << "\n";
            for (auto &e : employees)
                printReportRow(rpt, e);
            rpt.close();
            cout << "Report exported to report.txt\n";
        }
        pauseScreen();
    }

    // ---------------- Admin account ----------------
    void changeAdminPassword()
    {
        cls();
        line();
        printCentered("|| Change Admin Password ||");
        line();

        cout << "Current Password: ";
        string current = hidePassword();
        if (current != adminPass)
        {
            showBox("|| Incorrect Password ||");
            pauseScreen();
            return;
        }

        string newUser = getLineInput("New Admin User ID [" + adminUser + "]: ");
        if (newUser.empty())
        {
            newUser = adminUser;
        }
        else if (findEmployeeById(newUser) != nullptr)
        {
            cout << "That ID belongs to an existing employee. Keeping current admin ID.\n";
            newUser = adminUser;
        }

        cout << "New Password (leave blank to keep current): ";
        string newPass = hidePassword();
        if (newPass.empty())
            newPass = adminPass;

        adminUser = newUser;
        adminPass = newPass;

        ofstream cfgOut(CFG_FILE, ios::trunc);
        cfgOut << adminUser << "\n"
               << adminPass << "\n";
        cfgOut.close();

        showBox("|| Credentials Updated ||");
        pauseScreen();
    }

    // ================= EMPLOYEE DASHBOARD =================
    void employeeDashboard(const string &empId)
    {
        while (true)
        {
            Employee *e = findEmployeeById(empId);
            if (!e)
            {
                // Safety net in case an admin removed this employee mid-session
                showBox("|| Your Employee Record No Longer Exists ||");
                pauseScreen();
                return;
            }

            cls();
            line();
            printCentered("|| EMPLOYEE DASHBOARD ||");
            printCentered("Welcome, " + e->name + " (" + e->id + ")   |   Today: " + getCurrentDate());
            line();
            cout << "1) Mark Today's Attendance\n"
                 << "2) View My Attendance\n"
                 << "3) Apply for Leave\n"
                 << "4) View Leave Status\n"
                 << "5) View My Profile\n"
                 << "6) Logout\n";
            line();
            int choice = getIntInput("Enter number to select given option: ");

            switch (choice)
            {
            case 1:
                markTodayAttendance(empId);
                break;
            case 2:
                viewMyAttendance(empId);
                break;
            case 3:
                applyForLeave(empId);
                break;
            case 4:
                viewLeaveStatus(empId);
                break;
            case 5:
                viewMyProfile(empId);
                break;
            case 6:
                logoutMessage();
                return;
            default:
                cls();
                showBox("|| Invalid Input ||");
                pauseScreen();
            }
        }
    }

    void markTodayAttendance(const string &empId)
    {
        cls();
        line();
        printCentered("|| Mark Today's Attendance ||");
        line();

        Employee *e = findEmployeeById(empId);
        string today = getCurrentDate();

        if (findAttendance(empId, today))
        {
            printCentered("|| You Have Already Marked Attendance For Today ||");
            line();
            pauseScreen();
            return;
        }

        cout << "Marking attendance for: " << e->name << " on " << today << "\n";
        string status = askAttendanceStatus();

        AttendanceRecord a;
        a.empId = empId;
        a.empName = e->name;
        a.date = today;
        a.status = status;
        attendance.push_back(a);
        saveAttendance();

        showBox("|| Attendance marked successfully for today. ||");
        pauseScreen();
    }

    void viewMyAttendance(const string &empId)
    {
        cls();
        line();
        printCentered("|| My Attendance ||");
        line();

        printAttendanceTableHeader(false);
        int p = 0, ab = 0, lv = 0;
        for (auto &a : attendance)
        {
            if (a.empId == empId)
            {
                printAttendanceRowSimple(a);
                if (a.status == "Present")
                    p++;
                else if (a.status == "Absent")
                    ab++;
                else if (a.status == "Leave")
                    lv++;
            }
        }
        int total = p + ab + lv;
        line();
        if (total == 0)
            cout << "You have no attendance records yet.\n";
        else
        {
            double pct = (double)p / total * 100.0;
            cout << fixed << setprecision(2);
            cout << "Present: " << p << "  Absent: " << ab << "  Leave: " << lv
                 << "  |  Attendance: " << pct << "%\n";
        }
        line();
        pauseScreen();
    }

    void applyForLeave(const string &empId)
    {
        cls();
        line();
        printCentered("|| Apply For Leave ||");
        line();

        Employee *e = findEmployeeById(empId);

        LeaveRequest lr;
        lr.requestId = nextRequestId();
        lr.empId = empId;
        lr.empName = e->name;
        lr.leaveDate = getRequiredLineInput("Enter leave date (YYYY-MM-DD): ");
        lr.reason = getRequiredLineInput("Enter reason for leave: ");
        lr.leaveType = askLeaveType();
        lr.status = "Pending";

        leaves.push_back(lr);
        saveLeaves();

        showBox("|| Leave Request Submitted (Status: Pending) ||");
        pauseScreen();
    }

    void viewLeaveStatus(const string &empId)
    {
        cls();
        line();
        printCentered("|| My Leave Requests ||");
        line();

        bool any = false;
        cout << fitWidth("REQ#", 6) << fitWidth("DATE", 12) << fitWidth("TYPE", 14)
             << fitWidth("STATUS", 10) << "REASON" << endl;
        line();
        for (auto &lr : leaves)
        {
            if (lr.empId == empId)
            {
                cout << fitWidth(lr.requestId, 6) << fitWidth(lr.leaveDate, 12)
                     << fitWidth(lr.leaveType, 14) << fitWidth(lr.status, 10) << lr.reason << endl;
                any = true;
            }
        }
        if (!any)
            cout << "You have not applied for any leave yet.\n";
        line();
        pauseScreen();
    }

    void viewMyProfile(const string &empId)
    {
        cls();
        line();
        printCentered("|| My Profile ||");
        line();

        Employee *e = findEmployeeById(empId);
        cout << "Employee ID : " << e->id << "\n"
             << "Name        : " << e->name << "\n"
             << "Age         : " << e->age << "\n"
             << "Department  : " << e->department << "\n"
             << "Position    : " << e->position << "\n"
             << "Contact     : " << e->contact << "\n";
        line();
        pauseScreen();
    }
};

// ============================================================
int main()
{
    EAMS manager;

    while (true)
    {
        int choice = welcomeMenu();
        if (choice == 2)
        {
            cls();
            cout << "Thank you for using EAMS. Goodbye!" << endl;
            return 0;
        }

        LoginResult result = manager.login();
        if (result.role == Role::ADMIN)
            manager.adminDashboard();
        else if (result.role == Role::EMPLOYEE)
            manager.employeeDashboard(result.employeeId);
        // Role::NONE (failed login) simply loops back to the welcome menu
    }
    return 0;
}