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

// ------------------------------------------------------------
// Cross platform console helpers
// ------------------------------------------------------------
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

// Pads (or, for values that are too long, truncates with a trailing
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
            cout << "This field cannot be empty.\n";
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

string getCurrentDate()
{
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    ostringstream oss;
    oss << setfill('0') << setw(2) << ltm->tm_mday << "/"
        << setfill('0') << setw(2) << (ltm->tm_mon + 1) << "/"
        << (ltm->tm_year + 1900);
    return oss.str();
}

string toLowerStr(string s)
{
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return (char)tolower(c); });
    return s;
}

bool login()
{
    string storedUser = "admin", storedPass = "1234";
    ifstream cred("config.txt");
    if (cred)
    {
        string u, p;
        if (getline(cred, u) && getline(cred, p) && !u.empty() && !p.empty())
        {
            storedUser = u;
            storedPass = p;
        }
    }
    cred.close();

    int attemptsLeft = 3;
    while (attemptsLeft > 0)
    {
        cls();
        line();
        printCentered("|| EAMS - LOGIN PAGE ||");
        line();

        string username = getLineInput("Username: ");
        cout << "Password: ";
        string password = hidePassword();

        if (username == storedUser && password == storedPass)
        {
            showBox("|| Login Successful ||");
            pauseScreen();
            return true;
        }

        attemptsLeft--;
        line();
        if (username != storedUser && password != storedPass)
            printCentered("|| Incorrect Username & Password ||");
        else if (username != storedUser)
            printCentered("|| Incorrect Username ||");
        else
            printCentered("|| Incorrect Password ||");
        printCentered("(" + to_string(attemptsLeft) + " attempt(s) left)");
        line();
        pauseScreen();
    }

    cls();
    line();
    printCentered("|| Too Many Failed Attempts - Access Denied ||");
    line();
    pauseScreen();
    return false;
}

// ------------------------------------------------------------
// Data models
// ------------------------------------------------------------
struct Employee
{
    int id = 0;
    string name;
    string password;
    int age = 0;
    string department;
    string position;
    string contact;
};

struct LeaveRequest
{
    int empId = 0;
    string empName;
    string leaveDate;
    string reason;
    string leaveType;
    string status; // Pending, Approved, Rejected
};

struct AttendanceRecord
{
    int empId = 0;
    string date;
    string status;
};

// ------------------------------------------------------------
// Core system
// ------------------------------------------------------------
class EAMS
{
private:
    vector<Employee> employees;
    vector<AttendanceRecord> attendance;
    vector<LeaveRequest> leaveRequests;
    const string EMP_FILE = "employees.txt";
    const string ATT_FILE = "attendance.txt";
    const string LEAVE_FILE = "leave_requests.txt";
    const string CFG_FILE = "config.txt";
    int currentUserId = 0;
    string currentUserRole = "";

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
        loadEmployees();
        loadAttendance();
        loadLeaveRequests();
    }

    // ---------------- Persistence ----------------
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
            try
            {
                e.id = stoi(f[0]);
            }
            catch (...)
            {
                continue;
            }
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
            if (f.size() < 3)
                continue;

            AttendanceRecord a;
            try
            {
                a.empId = stoi(f[0]);
            }
            catch (...)
            {
                continue;
            }
            a.date = f[1];
            a.status = f[2];
            attendance.push_back(a);
        }
    }

    void saveAttendance()
    {
        ofstream fout(ATT_FILE, ios::trunc);
        for (auto &a : attendance)
            fout << a.empId << "\t" << a.date << "\t" << a.status << "\n";
    }

    void loadLeaveRequests()
    {
        leaveRequests.clear();
        ifstream fin(LEAVE_FILE);
        if (!fin)
            return;

        string ln;
        while (getline(fin, ln))
        {
            if (ln.empty())
                continue;
            vector<string> f = splitFields(ln, '\t');
            if (f.size() < 6)
                continue;

            LeaveRequest lr;
            try
            {
                lr.empId = stoi(f[0]);
            }
            catch (...)
            {
                continue;
            }
            lr.empName = f[1];
            lr.leaveDate = f[2];
            lr.reason = f[3];
            lr.leaveType = f[4];
            lr.status = f[5];
            leaveRequests.push_back(lr);
        }
    }

    void saveLeaveRequests()
    {
        ofstream fout(LEAVE_FILE, ios::trunc);
        for (auto &lr : leaveRequests)
            fout << lr.empId << "\t" << lr.empName << "\t" << lr.leaveDate << "\t"
                 << lr.reason << "\t" << lr.leaveType << "\t" << lr.status << "\n";
    }

    // ---------------- Helpers ----------------
    int nextEmployeeId()
    {
        int mx = 0;
        for (auto &e : employees)
            if (e.id > mx)
                mx = e.id;
        return mx + 1;
    }

    Employee *findById(int id)
    {
        for (auto &e : employees)
            if (e.id == id)
                return &e;
        return nullptr;
    }

    string askStatus()
    {
        cout << "\nAttendance status:\n"
             << " 1) Present\n"
             << " 2) Absent\n"
             << " 3) Leave\n"
             << " 4) Half Day\n";
        int s = getIntInput("Select status: ", 1, 4);
        switch (s)
        {
        case 1:
            return "Present";
        case 2:
            return "Absent";
        case 3:
            return "Leave";
        default:
            return "Half Day";
        }
    }

    // ---------------- Menu ----------------
    int mainMenu()
    {
        cls();
        line();
        printCentered("|| EAMS MAIN MENU ||");
        printCentered("Today: " + getCurrentDate() + "   |   Total Employees: " + to_string(employees.size()));
        line();
        cout << " 1) Add Employee\n"
             << " 2) Display All Employees\n"
             << " 3) Search Employee\n"
             << " 4) Update Employee\n"
             << " 5) Remove Employee\n"
             << " 6) Mark Attendance\n"
             << " 7) View Attendance Records\n"
             << " 8) Attendance Report\n"
             << " 9) Change Admin Password\n"
             << "10) Exit\n";
        line();
        return getIntInput("Enter number to select given option: ");
    }

    // ---------------- Employee management ----------------
    void addEmployee()
    {
        int count = getIntInput("Enter number of employees to add: ", 1, 200);
        vector<Employee> batch;
        int startId = nextEmployeeId();

        for (int i = 0; i < count; i++)
        {
            cls();
            line();
            printCentered("|| Enter Employee Details ||");
            line();
            printCentered("!! Employee " + to_string(i + 1) + " of " + to_string(count) + " !!");

            Employee e;
            e.id = startId + i;
            e.name = getRequiredLineInput("Enter employee name: ");
            e.age = getIntInput("Enter employee age: ", 15, 100);
            e.department = getLineInput("Enter department: ");
            e.position = getLineInput("Enter position/designation: ");
            e.contact = getLineInput("Enter contact number: ");

            batch.push_back(e);
        }

        cls();
        line();
        printCentered("|| Save Details ||");
        line();
        char save = getCharInput("Save these employee record(s) to file? (Y/N): ");

        if (save == 'y' || save == 'Y')
        {
            for (auto &e : batch)
                employees.push_back(e);
            saveEmployees();
            showBox("|| Employee Record(s) Saved ||");
        }
        else
        {
            showBox("|| Employee Record(s) Discarded ||");
        }
        pauseScreen();
    }

    void printEmployeeTableHeader()
    {
        cout << fitWidth("ID", 6) << fitWidth("NAME", 22) << fitWidth("AGE", 6)
             << fitWidth("DEPARTMENT", 18) << fitWidth("POSITION", 20) << "CONTACT" << endl;
        line();
    }

    void printEmployeeRow(const Employee &e)
    {
        cout << fitWidth(e.id, 6) << fitWidth(e.name, 22) << fitWidth(e.age, 6)
             << fitWidth(e.department, 18) << fitWidth(e.position, 20) << e.contact << endl;
    }

    void displayEmployees()
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

        printCentered("|| Employee Information ||");
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

        cout << "1) Search by ID\n2) Search by Name\n";
        int choice = getIntInput("Choose an option: ", 1, 2);

        vector<Employee *> results;
        if (choice == 1)
        {
            int id = getIntInput("Enter Employee ID: ");
            Employee *e = findById(id);
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

    void updateEmployee()
    {
        cls();
        line();
        printCentered("|| Update Employee ||");
        line();

        if (employees.empty())
        {
            printCentered("|| No Employees Found ||");
            line();
            pauseScreen();
            return;
        }

        int id = getIntInput("Enter Employee ID to update: ");
        Employee *e = findById(id);
        if (!e)
        {
            showBox("|| Employee Not Found ||");
            pauseScreen();
            return;
        }

        cout << "\nEditing \"" << e->name << "\" - press Enter to keep the current value.\n\n";

        string input = getLineInput("Name [" + e->name + "]: ");
        if (!input.empty())
            e->name = input;

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

        saveEmployees();
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

        int id = getIntInput("Enter Employee ID to remove: ");
        Employee *e = findById(id);
        if (!e)
        {
            showBox("|| Employee Not Found ||");
            pauseScreen();
            return;
        }

        cout << "\nName: " << e->name << "   Department: " << e->department << endl;
        char confirm = getCharInput("Are you sure you want to remove this employee? (Y/N): ");

        if (confirm == 'y' || confirm == 'Y')
        {
            employees.erase(remove_if(employees.begin(), employees.end(),
                                       [id](const Employee &emp)
                                       { return emp.id == id; }),
                             employees.end());
            saveEmployees();

            char removeAtt = getCharInput("Also remove this employee's attendance history? (Y/N): ");
            if (removeAtt == 'y' || removeAtt == 'Y')
            {
                attendance.erase(remove_if(attendance.begin(), attendance.end(),
                                            [id](const AttendanceRecord &a)
                                            { return a.empId == id; }),
                                  attendance.end());
                saveAttendance();
            }
            showBox("|| Employee Removed ||");
        }
        else
        {
            showBox("|| Removal Cancelled ||");
        }
        pauseScreen();
    }

    // ---------------- Attendance management ----------------
    void markAttendance()
    {
        cls();
        line();
        printCentered("|| Mark Attendance ||");
        line();

        if (employees.empty())
        {
            printCentered("|| No Employees Found ||");
            line();
            pauseScreen();
            return;
        }

        int id = getIntInput("Enter Employee ID: ");
        Employee *e = findById(id);
        if (!e)
        {
            showBox("|| Employee Not Found ||");
            pauseScreen();
            return;
        }

        cout << "\nMarking attendance for: " << e->name << " (" << e->department << ")\n\n";

        string today = getCurrentDate();
        char useToday = getCharInput("Use today's date (" + today + ")? (Y/N): ");
        string date = (useToday == 'y' || useToday == 'Y') ? today : getLineInput("Enter date (DD/MM/YYYY): ");

        for (auto &a : attendance)
        {
            if (a.empId == id && a.date == date)
            {
                cout << "\nAttendance is already marked as \"" << a.status << "\" for this date.\n";
                char update = getCharInput("Update it? (Y/N): ");
                if (update != 'y' && update != 'Y')
                {
                    pauseScreen();
                    return;
                }
                a.status = askStatus();
                saveAttendance();
                showBox("|| Attendance Updated ||");
                pauseScreen();
                return;
            }
        }

        AttendanceRecord a;
        a.empId = id;
        a.date = date;
        a.status = askStatus();
        attendance.push_back(a);
        saveAttendance();

        showBox("|| Attendance Recorded ||");
        pauseScreen();
    }

    void viewAttendance()
    {
        cls();
        line();
        printCentered("|| View Attendance ||");
        line();

        if (attendance.empty())
        {
            printCentered("|| No Attendance Records Found ||");
            line();
            pauseScreen();
            return;
        }

        cout << "1) View by Employee\n2) View by Date\n";
        int choice = getIntInput("Choose an option: ", 1, 2);

        if (choice == 1)
        {
            int id = getIntInput("Enter Employee ID: ");
            Employee *e = findById(id);
            cls();
            line();
            if (!e)
            {
                showBox("|| Employee Not Found ||");
                pauseScreen();
                return;
            }

            printCentered("|| Attendance for " + e->name + " ||");
            line();
            cout << fitWidth("DATE", 15) << "STATUS" << endl;
            line();

            int p = 0, ab = 0, lv = 0, hd = 0;
            for (auto &a : attendance)
            {
                if (a.empId == id)
                {
                    cout << fitWidth(a.date, 15) << a.status << endl;
                    if (a.status == "Present")
                        p++;
                    else if (a.status == "Absent")
                        ab++;
                    else if (a.status == "Leave")
                        lv++;
                    else if (a.status == "Half Day")
                        hd++;
                }
            }

            int total = p + ab + lv + hd;
            line();
            if (total == 0)
            {
                cout << "No attendance records found for this employee." << endl;
            }
            else
            {
                cout << "Present: " << p << "   Absent: " << ab
                     << "   Leave: " << lv << "   Half Day: " << hd << endl;
                double pct = ((p + hd * 0.5) / total) * 100.0;
                cout << fixed << setprecision(2);
                cout << "Attendance Percentage: " << pct << "%" << endl;
            }
            line();
        }
        else
        {
            string date = getLineInput("Enter date (DD/MM/YYYY): ");
            cls();
            line();
            printCentered("|| Attendance on " + date + " ||");
            line();
            cout << fitWidth("ID", 6) << fitWidth("NAME", 22) << "STATUS" << endl;
            line();

            bool found = false;
            for (auto &a : attendance)
            {
                if (a.date == date)
                {
                    Employee *e = findById(a.empId);
                    string name = e ? e->name : "(Unknown)";
                    cout << fitWidth(a.empId, 6) << fitWidth(name, 22) << a.status << endl;
                    found = true;
                }
            }
            if (!found)
                cout << "No attendance records found for this date." << endl;
            line();
        }
        pauseScreen();
    }

    void printReportRow(ostream &os, const Employee &e)
    {
        int p = 0, ab = 0, lv = 0, hd = 0;
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
                else if (rec.status == "Half Day")
                    hd++;
            }
        }
        int total = p + ab + lv + hd;
        double pct = total == 0 ? 0.0 : ((p + hd * 0.5) / total) * 100.0;

        ostringstream pctStr;
        pctStr << fixed << setprecision(2) << pct << "%";

        os << fitWidth(e.id, 6) << fitWidth(e.name, 20)
           << fitWidth(p, 9) << fitWidth(ab, 8) << fitWidth(lv, 8)
           << fitWidth(hd, 10) << pctStr.str() << "\n";
    }

    void printReportHeader(ostream &os)
    {
        os << fitWidth("ID", 6) << fitWidth("NAME", 20)
           << fitWidth("PRESENT", 9) << fitWidth("ABSENT", 8) << fitWidth("LEAVE", 8)
           << fitWidth("HALFDAY", 10) << "PERCENT" << "\n";
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
            rpt << string(70, '=') << "\n";
            printReportHeader(rpt);
            rpt << string(70, '=') << "\n";
            for (auto &e : employees)
                printReportRow(rpt, e);
            rpt.close();
            cout << "Report exported to report.txt\n";
        }

        pauseScreen();
    }

    // ---------------- Account ----------------
    void changePassword()
    {
        cls();
        line();
        printCentered("|| Change Admin Password ||");
        line();

        string storedUser = "admin", storedPass = "1234";
        ifstream cred(CFG_FILE);
        if (cred)
        {
            string u, p;
            if (getline(cred, u) && getline(cred, p) && !u.empty() && !p.empty())
            {
                storedUser = u;
                storedPass = p;
            }
        }
        cred.close();

        cout << "Current Password: ";
        string current = hidePassword();
        if (current != storedPass)
        {
            showBox("|| Incorrect Password ||");
            pauseScreen();
            return;
        }

        string newUser = getLineInput("New Username [" + storedUser + "]: ");
        if (newUser.empty())
            newUser = storedUser;

        cout << "New Password (leave blank to keep current): ";
        string newPass = hidePassword();
        if (newPass.empty())
            newPass = storedPass;

        ofstream cfgOut(CFG_FILE, ios::trunc);
        cfgOut << newUser << "\n"
               << newPass << "\n";
        cfgOut.close();

        showBox("|| Credentials Updated ||");
        pauseScreen();
    }
};

// ------------------------------------------------------------
int main()
{
    if (!login())
        return 0;

    EAMS manager;
    while (true)
    {
        int choice = manager.mainMenu();
        switch (choice)
        {
        case 1:
            manager.addEmployee();
            break;
        case 2:
            manager.displayEmployees();
            break;
        case 3:
            manager.searchEmployee();
            break;
        case 4:
            manager.updateEmployee();
            break;
        case 5:
            manager.removeEmployee();
            break;
        case 6:
            manager.markAttendance();
            break;
        case 7:
            manager.viewAttendance();
            break;
        case 8:
            manager.attendanceReport();
            break;
        case 9:
            manager.changePassword();
            break;
        case 10:
            cls();
            cout << "Thank you for using EAMS. Goodbye!" << endl;
            return 0;
        default:
            cls();
            showBox("|| Invalid Input ||");
            pauseScreen();
        }
    }
    return 0;
}