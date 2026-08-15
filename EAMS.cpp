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

string generateEmployeeId(int id)
{
    ostringstream oss;
    oss << "EMP" << setfill('0') << setw(3) << id;
    return oss.str();
}

string generateLeaveId(int id)
{
    ostringstream oss;
    oss << "L" << setfill('0') << setw(3) << id;
    return oss.str();
}

string getCurrentTime()
{
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    ostringstream oss;
    int hour = ltm->tm_hour;
    int min = ltm->tm_min;
    string ampm = hour >= 12 ? "PM" : "AM";
    hour = hour % 12;
    if (hour == 0) hour = 12;
    oss << setfill('0') << setw(2) << hour << ":" << setfill('0') << setw(2) << min << " " << ampm;
    return oss.str();
}

bool isValidContactNumber(const string &contact)
{
    if (contact.length() != 10)
        return false;
    for (char c : contact)
    {
        if (!isdigit(c))
            return false;
    }
    return true;
}

string getValidContactInput(const string &prompt)
{
    while (true)
    {
        string contact = getLineInput(prompt);
        if (isValidContactNumber(contact))
            return contact;
        cout << "Invalid contact number. Please enter exactly 10 digits." << endl;
    }
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
    string email;
    string address;
    string status; // Active, Inactive
    bool firstLogin = true; // true if employee hasn't changed password yet
};

struct LeaveRequest
{
    string leaveId;
    int empId = 0;
    string empName;
    string fromDate;
    string toDate;
    string reason;
    string leaveType;
    string status; // Pending, Approved, Rejected
};

struct AttendanceRecord
{
    int empId = 0;
    string date;
    string timeIn;
    string timeOut;
    string status;
};

struct Department
{
    string name;
    string description;
};

struct Position
{
    string name;
    string description;
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
    vector<Department> departments;
    vector<Position> positions;
    const string EMP_FILE = "employees.txt";
    const string ATT_FILE = "attendance.txt";
    const string LEAVE_FILE = "leave_requests.txt";
    const string DEPT_FILE = "departments.txt";
    const string POS_FILE = "positions.txt";
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
        loadDepartments();
        loadPositions();
    }

    Employee *findById(int id)
    {
        for (auto &e : employees)
            if (e.id == id)
                return &e;
        return nullptr;
    }

    // ---------------- Persistence ----------------
    void loadEmployees()
    {
        employees.clear();
        ifstream fin(EMP_FILE);
        if (!fin)
            return;

        string ln;
        int lineNum = 0;
        while (getline(fin, ln))
        {
            lineNum++;
            // Skip header lines (first 4 lines: border, title, border, header, separator)
            if (lineNum <= 5)
                continue;
            // Skip border line at end
            if (ln.find('=') != string::npos)
                continue;
            if (ln.empty() || ln.find('-') != string::npos)
                continue;
            
            // Parse the formatted line
            vector<string> f = splitFields(ln, ' ');
            // Filter out empty strings
            vector<string> fields;
            for (auto &field : f)
            {
                if (!field.empty())
                    fields.push_back(field);
            }
            
            if (fields.size() < 6)
                continue;

            Employee e;
            // Parse EMP001 format
            string empIdStr = fields[0];
            if (empIdStr.length() >= 4 && empIdStr.substr(0, 3) == "EMP")
            {
                try
                {
                    e.id = stoi(empIdStr.substr(3));
                }
                catch (...)
                {
                    continue;
                }
            }
            else
            {
                try
                {
                    e.id = stoi(empIdStr);
                }
                catch (...)
                {
                    continue;
                }
            }
            
            e.name = fields[1];
            e.department = fields[2];
            e.position = fields[3];
            e.contact = fields[4];
            e.status = fields[5];
            e.password = "1234"; // Default password for backward compatibility
            e.age = 25; // Default age for backward compatibility
            e.email = (fields.size() >= 7) ? fields[6] : "";
            e.firstLogin = (fields.size() >= 8) ? (fields[7] == "Yes") : true; // Default to first login for old records
            e.address = ""; // Default empty address
            
            employees.push_back(e);
        }
    }

    void saveEmployees()
    {
        ofstream fout(EMP_FILE, ios::trunc);
        string border = string(120, '=');
        string separator = string(120, '-');
        
        fout << border << endl;
        fout << fitWidth("", 45) << "EAMS - EMPLOYEE RECORDS" << endl;
        fout << border << endl;
        fout << fitWidth("ID", 10) << fitWidth("NAME", 20) << fitWidth("DEPARTMENT", 18) 
             << fitWidth("POSITION", 18) << fitWidth("CONTACT", 14) << fitWidth("EMAIL", 20) << fitWidth("STATUS", 10) << "FIRST LOGIN" << endl;
        fout << separator << endl;
        
        for (auto &e : employees)
        {
            string empId = "EMP" + string(3 - to_string(e.id).length(), '0') + to_string(e.id);
            string firstLoginStr = e.firstLogin ? "Yes" : "No";
            fout << fitWidth(empId, 10) << fitWidth(e.name, 20) << fitWidth(e.department, 18) 
                 << fitWidth(e.position, 18) << fitWidth(e.contact, 14) << fitWidth(e.email, 20) 
                 << fitWidth(e.status, 10) << firstLoginStr << endl;
        }
        
        fout << border << endl;
    }

    void loadAttendance()
    {
        attendance.clear();
        ifstream fin(ATT_FILE);
        if (!fin)
            return;

        string ln;
        int lineNum = 0;
        while (getline(fin, ln))
        {
            lineNum++;
            // Skip header lines (first 5 lines)
            if (lineNum <= 5)
                continue;
            // Skip border line at end
            if (ln.find('=') != string::npos)
                continue;
            if (ln.empty() || ln.find('-') != string::npos)
                continue;
            
            // Parse the formatted line
            vector<string> f = splitFields(ln, ' ');
            // Filter out empty strings
            vector<string> fields;
            for (auto &field : f)
            {
                if (!field.empty())
                    fields.push_back(field);
            }
            
            if (fields.size() < 6)
                continue;

            AttendanceRecord a;
            // Parse EMP001 format
            string empIdStr = fields[0];
            if (empIdStr.length() >= 4 && empIdStr.substr(0, 3) == "EMP")
            {
                try
                {
                    a.empId = stoi(empIdStr.substr(3));
                }
                catch (...)
                {
                    continue;
                }
            }
            else
            {
                try
                {
                    a.empId = stoi(empIdStr);
                }
                catch (...)
                {
                    continue;
                }
            }
            
            // fields[1] is name (skip)
            a.date = fields[2];
            a.timeIn = fields[3];
            a.timeOut = fields[4];
            a.status = fields[5];
            
            attendance.push_back(a);
        }
    }

    void saveAttendance()
    {
        ofstream fout(ATT_FILE, ios::trunc);
        string border = string(100, '=');
        string separator = string(100, '-');
        
        fout << border << endl;
        fout << fitWidth("", 32) << "EAMS - ATTENDANCE RECORDS" << endl;
        fout << border << endl;
        fout << fitWidth("EMP ID", 10) << fitWidth("NAME", 20) << fitWidth("DATE", 15) 
             << fitWidth("TIME IN", 12) << fitWidth("TIME OUT", 12) << "STATUS" << endl;
        fout << separator << endl;
        
        for (auto &a : attendance)
        {
            Employee *e = findById(a.empId);
            string empId = e ? "EMP" + string(3 - to_string(a.empId).length(), '0') + to_string(a.empId) : "Unknown";
            string empName = e ? e->name : "Unknown";
            
            fout << fitWidth(empId, 10) << fitWidth(empName, 20) << fitWidth(a.date, 15) 
                 << fitWidth(a.timeIn, 12) << fitWidth(a.timeOut, 12) << a.status << endl;
        }
        
        fout << border << endl;
    }

    void loadLeaveRequests()
    {
        leaveRequests.clear();
        ifstream fin(LEAVE_FILE);
        if (!fin)
            return;

        string ln;
        int lineNum = 0;
        while (getline(fin, ln))
        {
            lineNum++;
            // Skip header lines (first 5 lines)
            if (lineNum <= 5)
                continue;
            // Skip border line at end
            if (ln.find('=') != string::npos)
                continue;
            if (ln.empty() || ln.find('-') != string::npos)
                continue;
            
            // Parse the formatted line
            vector<string> f = splitFields(ln, ' ');
            // Filter out empty strings
            vector<string> fields;
            for (auto &field : f)
            {
                if (!field.empty())
                    fields.push_back(field);
            }
            
            if (fields.size() < 7)
                continue;

            LeaveRequest lr;
            lr.leaveId = fields[0];
            
            // Parse EMP001 format
            string empIdStr = fields[1];
            if (empIdStr.length() >= 4 && empIdStr.substr(0, 3) == "EMP")
            {
                try
                {
                    lr.empId = stoi(empIdStr.substr(3));
                }
                catch (...)
                {
                    continue;
                }
            }
            else
            {
                try
                {
                    lr.empId = stoi(empIdStr);
                }
                catch (...)
                {
                    continue;
                }
            }
            
            lr.empName = fields[2];
            lr.fromDate = fields[3];
            lr.toDate = fields[4];
            lr.leaveType = fields[5];
            lr.status = fields[6];
            lr.reason = "Leave request"; // Default reason
            
            leaveRequests.push_back(lr);
        }
    }

    void saveLeaveRequests()
    {
        ofstream fout(LEAVE_FILE, ios::trunc);
        string border = string(110, '=');
        string separator = string(110, '-');
        
        fout << border << endl;
        fout << fitWidth("", 35) << "EAMS - LEAVE RECORDS" << endl;
        fout << border << endl;
        fout << fitWidth("LEAVE ID", 12) << fitWidth("EMP ID", 10) << fitWidth("NAME", 20) 
             << fitWidth("FROM DATE", 15) << fitWidth("TO DATE", 15) << fitWidth("TYPE", 12) << "STATUS" << endl;
        fout << separator << endl;
        
        for (auto &lr : leaveRequests)
        {
            string empId = "EMP" + string(3 - to_string(lr.empId).length(), '0') + to_string(lr.empId);
            
            fout << fitWidth(lr.leaveId, 12) << fitWidth(empId, 10) << fitWidth(lr.empName, 20) 
                 << fitWidth(lr.fromDate, 15) << fitWidth(lr.toDate, 15) << fitWidth(lr.leaveType, 12) << lr.status << endl;
        }
        
        fout << border << endl;
    }

    void loadDepartments()
    {
        departments.clear();
        ifstream fin(DEPT_FILE);
        if (!fin)
            return;

        string ln;
        int lineNum = 0;
        while (getline(fin, ln))
        {
            lineNum++;
            // Skip header lines (first 4 lines)
            if (lineNum <= 4)
                continue;
            // Skip border line at end
            if (ln.find('=') != string::npos)
                continue;
            if (ln.empty() || ln.find('-') != string::npos)
                continue;
            
            // Parse the formatted line
            vector<string> f = splitFields(ln, ' ');
            // Filter out empty strings
            vector<string> fields;
            for (auto &field : f)
            {
                if (!field.empty())
                    fields.push_back(field);
            }
            
            if (fields.size() < 2)
                continue;

            Department d;
            d.name = fields[0];
            // Join remaining fields as description
            string desc = "";
            for (size_t i = 1; i < fields.size(); i++)
            {
                if (i > 1) desc += " ";
                desc += fields[i];
            }
            d.description = desc;
            departments.push_back(d);
        }
    }

    void saveDepartments()
    {
        ofstream fout(DEPT_FILE, ios::trunc);
        string border = string(70, '=');
        string separator = string(70, '-');
        
        fout << border << endl;
        fout << fitWidth("", 22) << "EAMS - DEPARTMENT RECORDS" << endl;
        fout << border << endl;
        fout << fitWidth("NAME", 30) << "DESCRIPTION" << endl;
        fout << separator << endl;
        
        for (auto &d : departments)
        {
            fout << fitWidth(d.name, 30) << d.description << endl;
        }
        
        fout << border << endl;
    }

    void loadPositions()
    {
        positions.clear();
        ifstream fin(POS_FILE);
        if (!fin)
            return;

        string ln;
        int lineNum = 0;
        while (getline(fin, ln))
        {
            lineNum++;
            // Skip header lines (first 4 lines)
            if (lineNum <= 4)
                continue;
            // Skip border line at end
            if (ln.find('=') != string::npos)
                continue;
            if (ln.empty() || ln.find('-') != string::npos)
                continue;
            
            // Parse the formatted line
            vector<string> f = splitFields(ln, ' ');
            // Filter out empty strings
            vector<string> fields;
            for (auto &field : f)
            {
                if (!field.empty())
                    fields.push_back(field);
            }
            
            if (fields.size() < 2)
                continue;

            Position p;
            p.name = fields[0];
            // Join remaining fields as description
            string desc = "";
            for (size_t i = 1; i < fields.size(); i++)
            {
                if (i > 1) desc += " ";
                desc += fields[i];
            }
            p.description = desc;
            positions.push_back(p);
        }
    }

    void savePositions()
    {
        ofstream fout(POS_FILE, ios::trunc);
        string border = string(70, '=');
        string separator = string(70, '-');
        
        fout << border << endl;
        fout << fitWidth("", 22) << "EAMS - POSITION RECORDS" << endl;
        fout << border << endl;
        fout << fitWidth("NAME", 30) << "DESCRIPTION" << endl;
        fout << separator << endl;
        
        for (auto &p : positions)
        {
            fout << fitWidth(p.name, 30) << p.description << endl;
        }
        
        fout << border << endl;
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
    int adminDashboard()
    {
        cls();
        line();
        printCentered("|| ADMIN DASHBOARD ||");
        printCentered("Today: " + getCurrentDate() + "   |   Total Employees: " + to_string(employees.size()));
        line();
        cout << " 1) Employee Management\n"
             << " 2) View Employees & Attendance\n"
             << " 3) Review & Update Employee Requests\n"
             << " 4) Logout\n";
        line();
        return getIntInput("Enter number to select given option: ", 1, 4);
    }

    int employeeManagementMenu()
    {
        cls();
        line();
        printCentered("|| EMPLOYEE MANAGEMENT ||");
        line();
        cout << " 1) Add New Employee\n"
             << " 2) Add Department\n"
             << " 3) Manage Departments\n"
             << " 4) Add Position\n"
             << " 5) Manage Positions\n"
             << " 6) Update Employee Information\n"
             << " 7) Remove Employee\n"
             << " 8) Back to Admin Dashboard\n";
        line();
        return getIntInput("Enter number to select given option: ", 1, 8);
    }

    int viewEmployeesAttendanceMenu()
    {
        cls();
        line();
        printCentered("|| VIEW EMPLOYEES & ATTENDANCE ||");
        line();
        cout << " 1) View All Employee Records\n"
             << " 2) Search Employee\n"
             << " 3) View Employee Attendance\n"
             << " 4) View Daily Attendance\n"
             << " 5) View Monthly Attendance\n"
             << " 6) View Overall Attendance Report\n"
             << " 7) View Leave Records\n"
             << " 8) Back to Admin Dashboard\n";
        line();
        return getIntInput("Enter number to select given option: ", 1, 8);
    }

    int reviewUpdateRequestsMenu()
    {
        cls();
        line();
        printCentered("|| REVIEW & UPDATE EMPLOYEE REQUESTS ||");
        line();
        cout << " 1) View Leave Requests\n"
             << " 2) Approve Leave Request\n"
             << " 3) Reject Leave Request\n"
             << " 4) Update Attendance Records\n"
             << " 5) Update Employee Information\n"
             << " 6) Back to Admin Dashboard\n";
        line();
        return getIntInput("Enter number to select given option: ", 1, 6);
    }

    int employeeDashboard(int empId)
    {
        cls();
        line();
        Employee *e = findById(empId);
        string empName = e ? e->name : "Unknown";
        printCentered("|| EMPLOYEE DASHBOARD ||");
        printCentered("Welcome, " + empName + " (ID: " + to_string(empId) + ")");
        printCentered("Today: " + getCurrentDate());
        line();
        cout << " 1) Mark Today's Attendance\n"
             << " 2) View My Attendance\n"
             << " 3) Apply for Leave\n"
             << " 4) View Leave Status\n"
             << " 5) View My Profile\n"
             << " 6) Logout\n";
        line();
        return getIntInput("Enter number to select given option: ", 1, 6);
    }

    // ---------------- Employee management ----------------
    void addEmployee()
    {
        // Check if departments exist
        if (departments.empty())
        {
            cls();
            line();
            printCentered("|| No department found. Please add a department first. ||");
            line();
            pauseScreen();
            return;
        }

        // Check if positions exist
        if (positions.empty())
        {
            cls();
            line();
            printCentered("|| No position found. Please add a position first. ||");
            line();
            pauseScreen();
            return;
        }

        int count = getIntInput("Enter number of employees to add: ", 1, 200);
        vector<Employee> batch;
        int startId = nextEmployeeId();

        for (int i = 0; i < count; i++)
        {
            cls();
            line();
            printCentered("|| ADD NEW EMPLOYEE ||");
            line();
            printCentered("!! Employee " + to_string(i + 1) + " of " + to_string(count) + " !!");
            line();

            Employee e;
            e.id = startId + i;
            string empId = generateEmployeeId(e.id);
            
            cout << "Employee ID   : " << empId << " [SYSTEM GENERATED]" << endl;
            e.name = getRequiredLineInput("Name          : ");
            
            // Department selection
            cout << "\nAvailable Departments:" << endl;
            for (size_t j = 0; j < departments.size(); j++)
            {
                cout << " " << (j + 1) << ") " << departments[j].name << endl;
            }
            int deptChoice = getIntInput("Select department: ", 1, (int)departments.size());
            e.department = departments[deptChoice - 1].name;
            
            // Position selection
            cout << "\nAvailable Positions:" << endl;
            for (size_t j = 0; j < positions.size(); j++)
            {
                cout << " " << (j + 1) << ") " << positions[j].name << endl;
            }
            int posChoice = getIntInput("Select position: ", 1, (int)positions.size());
            e.position = positions[posChoice - 1].name;
            
            e.contact = getValidContactInput("Contact       : ");
            e.email = getLineInput("Email         : ");
            e.address = getLineInput("Address       : ");
            e.status = "Active";
            e.firstLogin = true;
            e.password = empId; // Initial password is same as employee ID
            e.age = 25; // Default age

            batch.push_back(e);
        }

        cls();
        line();
        printCentered("|| EMPLOYEE DETAILS SAVED ||");
        line();
        
        for (size_t i = 0; i < batch.size(); i++)
        {
            string empId = generateEmployeeId(batch[i].id);
            cout << "\nEmployee " << (i + 1) << ":" << endl;
            cout << "  Employee ID   : " << empId << endl;
            cout << "  Name          : " << batch[i].name << endl;
            cout << "  Department    : " << batch[i].department << endl;
            cout << "  Position      : " << batch[i].position << endl;
            cout << "  Contact       : " << batch[i].contact << endl;
            cout << "  Email         : " << batch[i].email << endl;
            cout << "  Address       : " << batch[i].address << endl;
            cout << "  Initial Password : " << empId << endl;
        }
        
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
        cout << fitWidth("ID", 6) << fitWidth("NAME", 20) << fitWidth("AGE", 6)
             << fitWidth("DEPARTMENT", 18) << fitWidth("POSITION", 18) << "CONTACT" << endl;
        line();
    }

    void printEmployeeRow(const Employee &e)
    {
        cout << fitWidth(e.id, 6) << fitWidth(e.name, 20) << fitWidth(e.age, 6)
             << fitWidth(e.department, 18) << fitWidth(e.position, 18) << e.contact << endl;
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

        input = getLineInput("Password [" + e->password + "]: ");
        if (!input.empty())
            e->password = input;

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

    // ---------------- Leave Management ----------------
    void viewLeaveRequests()
    {
        cls();
        line();
        printCentered("|| Leave Requests ||");
        line();

        if (leaveRequests.empty())
        {
            printCentered("|| No Leave Requests Found ||");
            line();
            pauseScreen();
            return;
        }

        cout << fitWidth("LEAVE ID", 12) << fitWidth("EMP ID", 8) << fitWidth("NAME", 20) << fitWidth("FROM DATE", 15)
             << fitWidth("TYPE", 12) << fitWidth("REASON", 25) << "STATUS" << endl;
        line();

        for (auto &lr : leaveRequests)
        {
            string empId = "EMP" + string(3 - to_string(lr.empId).length(), '0') + to_string(lr.empId);
            cout << fitWidth(lr.leaveId, 12) << fitWidth(empId, 8) << fitWidth(lr.empName, 20) << fitWidth(lr.fromDate, 15)
                 << fitWidth(lr.leaveType, 12) << fitWidth(lr.reason, 25) << lr.status << endl;
        }
        line();
        pauseScreen();
    }

    void approveLeave()
    {
        cls();
        line();
        printCentered("|| Approve Leave ||");
        line();

        vector<LeaveRequest*> pending;
        for (auto &lr : leaveRequests)
            if (lr.status == "Pending")
                pending.push_back(&lr);

        if (pending.empty())
        {
            printCentered("|| No Pending Leave Requests ||");
            line();
            pauseScreen();
            return;
        }

        cout << fitWidth("#", 4) << fitWidth("LEAVE ID", 10) << fitWidth("EMP ID", 8) << fitWidth("NAME", 20) << fitWidth("FROM DATE", 15)
             << fitWidth("TYPE", 12) << fitWidth("REASON", 25) << endl;
        line();

        for (size_t i = 0; i < pending.size(); i++)
        {
            cout << fitWidth(i + 1, 4) << fitWidth(pending[i]->leaveId, 10) << fitWidth(pending[i]->empId, 8) << fitWidth(pending[i]->empName, 20)
                 << fitWidth(pending[i]->fromDate, 15) << fitWidth(pending[i]->leaveType, 12)
                 << fitWidth(pending[i]->reason, 25) << endl;
        }
        line();

        int choice = getIntInput("Enter request number to approve (0 to cancel): ", 0, (int)pending.size());
        if (choice == 0)
        {
            showBox("|| Cancelled ||");
            pauseScreen();
            return;
        }

        LeaveRequest *lr = pending[choice - 1];
        lr->status = "Approved";
        saveLeaveRequests();

        // Update attendance record for the leave date
        bool found = false;
        for (auto &a : attendance)
        {
            if (a.empId == lr->empId && a.date == lr->fromDate)
            {
                a.status = "Leave";
                found = true;
                break;
            }
        }
        if (!found)
        {
            AttendanceRecord a;
            a.empId = lr->empId;
            a.date = lr->fromDate;
            a.timeIn = "--";
            a.timeOut = "--";
            a.status = "Leave";
            attendance.push_back(a);
        }
        saveAttendance();

        showBox("|| Leave Approved ||");
        pauseScreen();
    }

    void rejectLeave()
    {
        cls();
        line();
        printCentered("|| Reject Leave ||");
        line();

        vector<LeaveRequest*> pending;
        for (auto &lr : leaveRequests)
            if (lr.status == "Pending")
                pending.push_back(&lr);

        if (pending.empty())
        {
            printCentered("|| No Pending Leave Requests ||");
            line();
            pauseScreen();
            return;
        }

        cout << fitWidth("#", 4) << fitWidth("LEAVE ID", 10) << fitWidth("EMP ID", 8) << fitWidth("NAME", 20) << fitWidth("FROM DATE", 15)
             << fitWidth("TYPE", 12) << fitWidth("REASON", 25) << endl;
        line();

        for (size_t i = 0; i < pending.size(); i++)
        {
            cout << fitWidth(i + 1, 4) << fitWidth(pending[i]->leaveId, 10) << fitWidth(pending[i]->empId, 8) << fitWidth(pending[i]->empName, 20)
                 << fitWidth(pending[i]->fromDate, 15) << fitWidth(pending[i]->leaveType, 12)
                 << fitWidth(pending[i]->reason, 25) << endl;
        }
        line();

        int choice = getIntInput("Enter request number to reject (0 to cancel): ", 0, (int)pending.size());
        if (choice == 0)
        {
            showBox("|| Cancelled ||");
            pauseScreen();
            return;
        }

        LeaveRequest *lr = pending[choice - 1];
        lr->status = "Rejected";
        saveLeaveRequests();

        showBox("|| Leave Rejected ||");
        pauseScreen();
    }

    void applyLeave(int empId)
    {
        cls();
        line();
        printCentered("|| Apply for Leave ||");
        line();

        Employee *e = findById(empId);
        if (!e)
        {
            showBox("|| Employee Not Found ||");
            pauseScreen();
            return;
        }

        string fromDate = getLineInput("Enter from date (DD/MM/YYYY): ");
        string toDate = getLineInput("Enter to date (DD/MM/YYYY): ");
        string reason = getRequiredLineInput("Enter reason for leave: ");

        cout << "\nLeave Type:\n"
             << " 1) Sick Leave\n"
             << " 2) Casual Leave\n"
             << " 3) Annual Leave\n"
             << " 4) Other\n";
        int typeChoice = getIntInput("Select leave type: ", 1, 4);
        string leaveType;
        switch (typeChoice)
        {
        case 1:
            leaveType = "Sick Leave";
            break;
        case 2:
            leaveType = "Casual Leave";
            break;
        case 3:
            leaveType = "Annual Leave";
            break;
        default:
            leaveType = "Other";
        }

        // Check for duplicate leave request
        for (auto &lr : leaveRequests)
        {
            if (lr.empId == empId && lr.fromDate == fromDate)
            {
                showBox("|| Leave request already exists for this date ||");
                pauseScreen();
                return;
            }
        }

        // Generate leave ID
        int leaveNum = leaveRequests.size() + 1;
        string leaveId = generateLeaveId(leaveNum);

        LeaveRequest lr;
        lr.leaveId = leaveId;
        lr.empId = empId;
        lr.empName = e->name;
        lr.fromDate = fromDate;
        lr.toDate = toDate;
        lr.reason = reason;
        lr.leaveType = leaveType;
        lr.status = "Pending";
        leaveRequests.push_back(lr);
        saveLeaveRequests();

        showBox("|| Leave Request Submitted ||");
        pauseScreen();
    }

    void viewLeaveStatus(int empId)
    {
        cls();
        line();
        printCentered("|| My Leave Status ||");
        line();

        vector<LeaveRequest*> myRequests;
        for (auto &lr : leaveRequests)
            if (lr.empId == empId)
                myRequests.push_back(&lr);

        if (myRequests.empty())
        {
            printCentered("|| No Leave Requests Found ||");
            line();
            pauseScreen();
            return;
        }

        cout << fitWidth("LEAVE ID", 12) << fitWidth("FROM DATE", 15) << fitWidth("TO DATE", 15) << fitWidth("TYPE", 15) << fitWidth("REASON", 20) << "STATUS" << endl;
        line();

        for (auto *lr : myRequests)
        {
            cout << fitWidth(lr->leaveId, 12) << fitWidth(lr->fromDate, 15) << fitWidth(lr->toDate, 15) << fitWidth(lr->leaveType, 15)
                 << fitWidth(lr->reason, 20) << lr->status << endl;
        }
        line();
        pauseScreen();
    }

    // ---------------- Employee Functions ----------------
    void changePassword(int empId)
    {
        cls();
        line();
        printCentered("|| CHANGE PASSWORD ||");
        line();

        Employee *e = findById(empId);
        if (!e)
        {
            showBox("|| Employee Not Found ||");
            pauseScreen();
            return;
        }

        string empIdStr = generateEmployeeId(empId);
        string oldPassword = e->password;

        while (true)
        {
            cout << "\nEnter New Password    : ";
            string newPass = hidePassword();
            cout << "Confirm New Password  : ";
            string confirmPass = hidePassword();

            if (newPass.empty())
            {
                cout << "\nPassword cannot be empty. Please try again." << endl;
                pauseScreen();
                continue;
            }

            if (newPass != confirmPass)
            {
                cout << "\nPasswords do not match. Please try again." << endl;
                pauseScreen();
                continue;
            }

            if (newPass == oldPassword)
            {
                cout << "\nNew password cannot be the same as the old password. Please try again." << endl;
                pauseScreen();
                continue;
            }

            // Password change successful
            e->password = newPass;
            e->firstLogin = false;
            saveEmployees();

            cls();
            line();
            printCentered("|| Password changed successfully ||");
            line();
            cout << "\nWelcome, " << empIdStr << "!" << endl;
            pauseScreen();
            return;
        }
    }

    void markTodayAttendance(int empId)
    {
        cls();
        line();
        printCentered("|| Mark Today's Attendance ||");
        line();

        Employee *e = findById(empId);
        if (!e)
        {
            showBox("|| Employee Not Found ||");
            pauseScreen();
            return;
        }

        string today = getCurrentDate();

        // Check if attendance already marked for today
        for (auto &a : attendance)
        {
            if (a.empId == empId && a.date == today)
            {
                cout << "\nAttendance already marked as: " << a.status << endl;
                showBox("|| Attendance Already Marked ||");
                pauseScreen();
                return;
            }
        }

        cout << "\nMarking attendance for: " << e->name << endl;
        cout << "Date: " << today << endl;

        cout << "\nAttendance status:\n"
             << " 1) Present\n"
             << " 2) Absent\n"
             << " 3) Leave\n";
        int s = getIntInput("Select status: ", 1, 3);
        string status;
        switch (s)
        {
        case 1:
            status = "Present";
            break;
        case 2:
            status = "Absent";
            break;
        case 3:
            status = "Leave";
            break;
        }

        AttendanceRecord a;
        a.empId = empId;
        a.date = today;
        a.timeIn = (status == "Present") ? getCurrentTime() : "--";
        a.timeOut = (status == "Present") ? "05:00 PM" : "--";
        a.status = status;
        attendance.push_back(a);
        saveAttendance();

        showBox("|| Attendance marked successfully for today ||");
        pauseScreen();
    }

    void viewMyAttendance(int empId)
    {
        cls();
        line();
        printCentered("|| My Attendance ||");
        line();

        Employee *e = findById(empId);
        if (!e)
        {
            showBox("|| Employee Not Found ||");
            pauseScreen();
            return;
        }

        cout << fitWidth("DATE", 15) << "STATUS" << endl;
        line();

        int p = 0, ab = 0, lv = 0;
        for (auto &a : attendance)
        {
            if (a.empId == empId)
            {
                cout << fitWidth(a.date, 15) << a.status << endl;
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
        {
            cout << "No attendance records found." << endl;
        }
        else
        {
            cout << "Present: " << p << "   Absent: " << ab << "   Leave: " << lv << endl;
            double pct = ((double)p / total) * 100.0;
            cout << fixed << setprecision(2);
            cout << "Attendance Percentage: " << pct << "%" << endl;
        }
        line();
        pauseScreen();
    }

    void viewMyProfile(int empId)
    {
        cls();
        line();
        printCentered("|| My Profile ||");
        line();

        Employee *e = findById(empId);
        if (!e)
        {
            showBox("|| Employee Not Found ||");
            pauseScreen();
            return;
        }

        cout << "Employee ID: " << e->id << endl;
        cout << "Name: " << e->name << endl;
        cout << "Age: " << e->age << endl;
        cout << "Department: " << e->department << endl;
        cout << "Position: " << e->position << endl;
        cout << "Contact: " << e->contact << endl;
        line();
        pauseScreen();
    }

    // ---------------- Admin Specific Functions ----------------
    void viewEmployeeAttendance()
    {
        cls();
        line();
        printCentered("|| View Employee Attendance ||");
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

        cls();
        line();
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
        pauseScreen();
    }

    void updateAttendanceRecords()
    {
        cls();
        line();
        printCentered("|| Update Attendance Records ||");
        line();

        if (attendance.empty())
        {
            printCentered("|| No Attendance Records Found ||");
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

        string date = getLineInput("Enter date (DD/MM/YYYY): ");

        AttendanceRecord *target = nullptr;
        for (auto &a : attendance)
        {
            if (a.empId == id && a.date == date)
            {
                target = &a;
                break;
            }
        }

        if (!target)
        {
            cout << "No attendance record found for this date." << endl;
            char create = getCharInput("Create new record? (Y/N): ");
            if (create != 'y' && create != 'Y')
            {
                pauseScreen();
                return;
            }
            AttendanceRecord a;
            a.empId = id;
            a.date = date;
            a.status = askStatus();
            attendance.push_back(a);
            saveAttendance();
            showBox("|| Attendance Record Created ||");
            pauseScreen();
            return;
        }

        cout << "\nCurrent status: " << target->status << endl;
        target->status = askStatus();
        saveAttendance();
        showBox("|| Attendance Record Updated ||");
        pauseScreen();
    }

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

        printEmployeeTableHeader();
        for (auto &e : employees)
            printEmployeeRow(e);
        line();
        pauseScreen();
    }

    // ---------------- Department Management ----------------
    void addDepartment()
    {
        cls();
        line();
        printCentered("|| Add Department ||");
        line();

        string name = getRequiredLineInput("Enter department name: ");
        
        // Check for duplicate
        for (auto &d : departments)
        {
            if (toLowerStr(d.name) == toLowerStr(name))
            {
                showBox("|| Department Already Exists ||");
                pauseScreen();
                return;
            }
        }

        string description = getLineInput("Enter description (optional): ");

        Department d;
        d.name = name;
        d.description = description;
        departments.push_back(d);
        saveDepartments();

        showBox("|| Department Added Successfully ||");
        pauseScreen();
    }

    void manageDepartments()
    {
        while (true)
        {
            cls();
            line();
            printCentered("|| Manage Departments ||");
            line();

            if (departments.empty())
            {
                printCentered("|| No Departments Found ||");
                line();
                pauseScreen();
                return;
            }

            cout << fitWidth("#", 4) << fitWidth("NAME", 25) << "DESCRIPTION" << endl;
            line();

            for (size_t i = 0; i < departments.size(); i++)
            {
                cout << fitWidth(i + 1, 4) << fitWidth(departments[i].name, 25) << departments[i].description << endl;
            }
            line();

            cout << " 1) Edit Department\n"
                 << " 2) Delete Department\n"
                 << " 3) Back\n";
            line();

            int choice = getIntInput("Enter number to select given option: ", 1, 3);
            if (choice == 3)
                break;

            if (choice == 1 || choice == 2)
            {
                int deptNum = getIntInput("Enter department number: ", 1, (int)departments.size());
                if (choice == 1)
                {
                    string newName = getLineInput("New name [" + departments[deptNum - 1].name + "]: ");
                    if (!newName.empty())
                        departments[deptNum - 1].name = newName;

                    string newDesc = getLineInput("New description [" + departments[deptNum - 1].description + "]: ");
                    if (!newDesc.empty())
                        departments[deptNum - 1].description = newDesc;

                    saveDepartments();
                    showBox("|| Department Updated ||");
                }
                else
                {
                    char confirm = getCharInput("Delete this department? (Y/N): ");
                    if (confirm == 'y' || confirm == 'Y')
                    {
                        departments.erase(departments.begin() + deptNum - 1);
                        saveDepartments();
                        showBox("|| Department Deleted ||");
                    }
                }
                pauseScreen();
            }
        }
    }

    // ---------------- Position Management ----------------
    void addPosition()
    {
        cls();
        line();
        printCentered("|| Add Position ||");
        line();

        string name = getRequiredLineInput("Enter position name: ");
        
        // Check for duplicate
        for (auto &p : positions)
        {
            if (toLowerStr(p.name) == toLowerStr(name))
            {
                showBox("|| Position Already Exists ||");
                pauseScreen();
                return;
            }
        }

        string description = getLineInput("Enter description (optional): ");

        Position p;
        p.name = name;
        p.description = description;
        positions.push_back(p);
        savePositions();

        showBox("|| Position Added Successfully ||");
        pauseScreen();
    }

    void managePositions()
    {
        while (true)
        {
            cls();
            line();
            printCentered("|| Manage Positions ||");
            line();

            if (positions.empty())
            {
                printCentered("|| No Positions Found ||");
                line();
                pauseScreen();
                return;
            }

            cout << fitWidth("#", 4) << fitWidth("NAME", 25) << "DESCRIPTION" << endl;
            line();

            for (size_t i = 0; i < positions.size(); i++)
            {
                cout << fitWidth(i + 1, 4) << fitWidth(positions[i].name, 25) << positions[i].description << endl;
            }
            line();

            cout << " 1) Edit Position\n"
                 << " 2) Delete Position\n"
                 << " 3) Back\n";
            line();

            int choice = getIntInput("Enter number to select given option: ", 1, 3);
            if (choice == 3)
                break;

            if (choice == 1 || choice == 2)
            {
                int posNum = getIntInput("Enter position number: ", 1, (int)positions.size());
                if (choice == 1)
                {
                    string newName = getLineInput("New name [" + positions[posNum - 1].name + "]: ");
                    if (!newName.empty())
                        positions[posNum - 1].name = newName;

                    string newDesc = getLineInput("New description [" + positions[posNum - 1].description + "]: ");
                    if (!newDesc.empty())
                        positions[posNum - 1].description = newDesc;

                    savePositions();
                    showBox("|| Position Updated ||");
                }
                else
                {
                    char confirm = getCharInput("Delete this position? (Y/N): ");
                    if (confirm == 'y' || confirm == 'Y')
                    {
                        positions.erase(positions.begin() + posNum - 1);
                        savePositions();
                        showBox("|| Position Deleted ||");
                    }
                }
                pauseScreen();
            }
        }
    }

    // ---------------- View Functions ----------------
    void viewDailyAttendance()
    {
        cls();
        line();
        printCentered("|| View Daily Attendance ||");
        line();

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
        pauseScreen();
    }

    void viewLeaveRecords()
    {
        cls();
        line();
        printCentered("|| Leave Records ||");
        line();

        if (leaveRequests.empty())
        {
            printCentered("|| No Leave Records Found ||");
            line();
            pauseScreen();
            return;
        }

        cout << fitWidth("LEAVE ID", 12) << fitWidth("EMP ID", 8) << fitWidth("NAME", 20) << fitWidth("FROM DATE", 15)
             << fitWidth("TYPE", 12) << fitWidth("REASON", 25) << "STATUS" << endl;
        line();

        for (auto &lr : leaveRequests)
        {
            string empId = "EMP" + string(3 - to_string(lr.empId).length(), '0') + to_string(lr.empId);
            cout << fitWidth(lr.leaveId, 12) << fitWidth(empId, 8) << fitWidth(lr.empName, 20) << fitWidth(lr.fromDate, 15)
                 << fitWidth(lr.leaveType, 12) << fitWidth(lr.reason, 25) << lr.status << endl;
        }
        line();
        pauseScreen();
    }

    void viewMonthlyAttendance()
    {
        cls();
        line();
        printCentered("|| View Monthly Attendance ||");
        line();

        string month = getLineInput("Enter month (MM/YYYY): ");
        cls();
        line();
        printCentered("|| Attendance for " + month + " ||");
        line();
        cout << fitWidth("ID", 6) << fitWidth("NAME", 22) << fitWidth("DATE", 15) << "STATUS" << endl;
        line();

        bool found = false;
        for (auto &a : attendance)
        {
            if (a.date.length() >= 7 && a.date.substr(3, 7) == month)
            {
                Employee *e = findById(a.empId);
                string name = e ? e->name : "(Unknown)";
                cout << fitWidth(a.empId, 6) << fitWidth(name, 22) << fitWidth(a.date, 15) << a.status << endl;
                found = true;
            }
        }
        if (!found)
            cout << "No attendance records found for this month." << endl;
        line();
        pauseScreen();
    }
};

// ------------------------------------------------------------
// Login function (must be after EAMS class definition)
// ------------------------------------------------------------
struct LoginResult
{
    string role;
    int userId;
    bool firstLogin;
};

LoginResult login(EAMS &manager)
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

        string userId = getLineInput("User ID: ");
        cout << "Password: ";
        string password = hidePassword();

        // Check admin credentials
        if (userId == storedUser && password == storedPass)
        {
            showBox("|| Admin Login Successful ||");
            pauseScreen();
            return {"admin", 0, false};
        }

        // Check employee credentials
        int empId;
        try
        {
            empId = stoi(userId);
        }
        catch (...)
        {
            empId = -1;
        }

        Employee *emp = manager.findById(empId);
        if (emp && emp->password == password)
        {
            showBox("|| Employee Login Successful ||");
            pauseScreen();
            return {"employee", empId, emp->firstLogin};
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
    return {"", -1, false};
}

// ------------------------------------------------------------
int main()
{
    EAMS manager;
    
    while (true)
    {
        pair<string, int> loginResult = login(manager);
        string role = loginResult.first;
        int userId = loginResult.second;
        
        if (role.empty())
        {
            // Login failed, exit program
            return 0;
        }
        
        if (role == "admin")
        {
            // Admin Dashboard
            while (true)
            {
                int choice = manager.adminDashboard();
                if (choice == 4) // Logout
                {
                    showBox("|| Logged Out Successfully ||");
                    pauseScreen();
                    break; // Exit admin loop, return to login
                }
                
                if (choice == 1) // Employee Management
                {
                    while (true)
                    {
                        int subChoice = manager.employeeManagementMenu();
                        if (subChoice == 8) // Back to Admin Dashboard
                            break;
                        
                        switch (subChoice)
                        {
                        case 1:
                            manager.addEmployee();
                            break;
                        case 2:
                            manager.addDepartment();
                            break;
                        case 3:
                            manager.manageDepartments();
                            break;
                        case 4:
                            manager.addPosition();
                            break;
                        case 5:
                            manager.managePositions();
                            break;
                        case 6:
                            manager.updateEmployee();
                            break;
                        case 7:
                            manager.removeEmployee();
                            break;
                        default:
                            cls();
                            showBox("|| Invalid Input ||");
                            pauseScreen();
                        }
                    }
                }
                else if (choice == 2) // View Employees & Attendance
                {
                    while (true)
                    {
                        int subChoice = manager.viewEmployeesAttendanceMenu();
                        if (subChoice == 8) // Back to Admin Dashboard
                            break;
                        
                        switch (subChoice)
                        {
                        case 1:
                            manager.displayEmployees();
                            break;
                        case 2:
                            manager.searchEmployee();
                            break;
                        case 3:
                            manager.viewEmployeeAttendance();
                            break;
                        case 4:
                            manager.viewDailyAttendance();
                            break;
                        case 5:
                            manager.viewMonthlyAttendance();
                            break;
                        case 6:
                            manager.attendanceReport();
                            break;
                        case 7:
                            manager.viewLeaveRecords();
                            break;
                        default:
                            cls();
                            showBox("|| Invalid Input ||");
                            pauseScreen();
                        }
                    }
                }
                else if (choice == 3) // Review & Update Employee Requests
                {
                    while (true)
                    {
                        int subChoice = manager.reviewUpdateRequestsMenu();
                        if (subChoice == 6) // Back to Admin Dashboard
                            break;
                        
                        switch (subChoice)
                        {
                        case 1:
                            manager.viewLeaveRequests();
                            break;
                        case 2:
                            manager.approveLeave();
                            break;
                        case 3:
                            manager.rejectLeave();
                            break;
                        case 4:
                            manager.updateAttendanceRecords();
                            break;
                        case 5:
                            manager.updateEmployee();
                            break;
                        default:
                            cls();
                            showBox("|| Invalid Input ||");
                            pauseScreen();
                        }
                    }
                }
            }
        }
        else if (role == "employee")
        {
            // Employee Dashboard
            while (true)
            {
                int choice = manager.employeeDashboard(userId);
                switch (choice)
                {
                case 1:
                    manager.markTodayAttendance(userId);
                    break;
                case 2:
                    manager.viewMyAttendance(userId);
                    break;
                case 3:
                    manager.applyLeave(userId);
                    break;
                case 4:
                    manager.viewLeaveStatus(userId);
                    break;
                case 5:
                    manager.viewMyProfile(userId);
                    break;
                case 6:
                    showBox("|| Logged Out Successfully ||");
                    pauseScreen();
                    break; // Exit employee loop, return to login
                default:
                    cls();
                    showBox("|| Invalid Input ||");
                    pauseScreen();
                }
                
                if (choice == 6)
                    break; // Logout
            }
        }
    }
    
    return 0;
}