#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <conio.h>

using namespace std;

// ==========================================
// STRUCTS / CLASSES
// ==========================================

struct Employee {
    string employeeId;
    string name;
    int age;
    string gender;
    string phone;
    string department;
    string position;
};

struct Attendance {
    string employeeId;
    string employeeName;
    string date;
    string checkIn;
    string checkOut;
    string status;
};

// Global vectors to store data
vector<Employee> employees;
vector<Attendance> attendanceRecords;

// ==========================================
// UTILITY FUNCTIONS
// ==========================================

void clearScreen() {
    system("cls");
}

void printHeader(const string& title) {
    int width = 50;
    cout << "\n";
    cout << string(width, '=') << "\n";
    cout << string((width - title.length()) / 2, ' ') << title << "\n";
    cout << string(width, '=') << "\n";
}

void printBoxHeader(const string& title) {
    cout << "\n";
    cout << "+" << string(48, '=') << "+\n";
    cout << "|" << string((50 - title.length()) / 2, ' ') << title << string((50 - title.length()) / 2, ' ') << "|\n";
    cout << "+" << string(48, '=') << "+\n";
}

void printDashedLine() {
    cout << string(50, '-') << "\n";
}

void printStarLine() {
    cout << string(50, '*') << "\n";
}

void printDottedLine() {
    cout << string(50, '.') << "\n";
}

void printSeparator() {
    cout << "\n" << string(50, '~') << "\n";
}

void printMenuBorder() {
    cout << "+" << string(48, '-') << "+\n";
}

void printMenuFooter() {
    cout << "+" << string(48, '-') << "+\n";
}

void printMenuItem(int num, const string& text) {
    cout << "| " << setw(2) << num << ". " << setw(42) << left << text << "|\n";
}

void printMenuOption(const string& text) {
    cout << "| " << setw(45) << left << text << "|\n";
}

string toLower(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

string toUpper(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void pauseScreen() {
    cout << "\n------------------------------------------\n";
    cout << "    Press Enter to continue...\n";
    cout << "------------------------------------------\n";
    cin.ignore();
    cin.get();
}

// ==========================================
// INPUT VALIDATION FUNCTIONS
// ==========================================

int getInteger(const string& prompt, int min = 0, int max = 999999) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            if (value >= min && value <= max) {
                clearInputBuffer();
                return value;
            } else {
                cout << "[WARNING] Value must be between " << min << " and " << max << "!\n";
                cout << "Please enter again.\n";
            }
        } else {
            cout << "[WARNING] Invalid input!\n";
            cout << "Please enter a valid number.\n";
            clearInputBuffer();
        }
    }
}

string getNonEmptyString(const string& prompt) {
    string value;
    while (true) {
        cout << prompt;
        getline(cin, value);
        
        // Remove leading/trailing whitespace
        size_t start = value.find_first_not_of(" \t");
        size_t end = value.find_last_not_of(" \t");
        
        if (start == string::npos) {
            cout << "[WARNING] This field cannot be empty!\n";
            cout << "Please enter again.\n";
        } else {
            value = value.substr(start, end - start + 1);
            if (value.empty()) {
                cout << "[WARNING] This field cannot be empty!\n";
                cout << "Please enter again.\n";
            } else {
                return value;
            }
        }
    }
}

string getPhone(const string& prompt) {
    string phone;
    while (true) {
        phone = getNonEmptyString(prompt);
        
        // Check if phone contains only digits
        bool valid = true;
        for (char c : phone) {
            if (!isdigit(c)) {
                valid = false;
                break;
            }
        }
        
        if (!valid) {
            cout << "[WARNING] Phone number must contain only digits!\n";
            cout << "Please enter again.\n";
        } else if (phone.length() < 7 || phone.length() > 15) {
            cout << "[WARNING] Phone number must be between 7 and 15 digits!\n";
            cout << "Please enter again.\n";
        } else {
            return phone;
        }
    }
}

string getValidDate(const string& prompt) {
    string date;
    while (true) {
        date = getNonEmptyString(prompt);
        
        // Simple date validation (DD/MM/YYYY format)
        if (date.length() != 10) {
            cout << "[WARNING] Invalid date format! Use DD/MM/YYYY\n";
            cout << "Please enter again.\n";
            continue;
        }
        
        if (date[2] != '/' || date[5] != '/') {
            cout << "[WARNING] Invalid date format! Use DD/MM/YYYY\n";
            cout << "Please enter again.\n";
            continue;
        }
        
        bool valid = true;
        for (int i = 0; i < 10; i++) {
            if (i == 2 || i == 5) continue;
            if (!isdigit(date[i])) {
                valid = false;
                break;
            }
        }
        
        if (!valid) {
            cout << "[WARNING] Invalid date! Use numbers only.\n";
            cout << "Please enter again.\n";
            continue;
        }
        
        // Extract day, month, year
        int day = stoi(date.substr(0, 2));
        int month = stoi(date.substr(3, 2));
        int year = stoi(date.substr(6, 4));
        
        if (day < 1 || day > 31) {
            cout << "[WARNING] Invalid day! Must be between 1 and 31.\n";
            cout << "Please enter again.\n";
            continue;
        }
        
        if (month < 1 || month > 12) {
            cout << "[WARNING] Invalid month! Must be between 1 and 12.\n";
            cout << "Please enter again.\n";
            continue;
        }
        
        if (year < 1900 || year > 2100) {
            cout << "[WARNING] Invalid year! Must be between 1900 and 2100.\n";
            cout << "Please enter again.\n";
            continue;
        }
        
        return date;
    }
}

string getValidTime(const string& prompt) {
    string time;
    while (true) {
        time = getNonEmptyString(prompt);
        
        // Simple time validation (HH:MM format)
        if (time.length() != 5) {
            cout << "[WARNING] Invalid time format! Use HH:MM\n";
            cout << "Please enter again.\n";
            continue;
        }
        
        if (time[2] != ':') {
            cout << "[WARNING] Invalid time format! Use HH:MM\n";
            cout << "Please enter again.\n";
            continue;
        }
        
        bool valid = true;
        for (int i = 0; i < 5; i++) {
            if (i == 2) continue;
            if (!isdigit(time[i])) {
                valid = false;
                break;
            }
        }
        
        if (!valid) {
            cout << "[WARNING] Invalid time! Use numbers only.\n";
            cout << "Please enter again.\n";
            continue;
        }
        
        // Extract hour and minute
        int hour = stoi(time.substr(0, 2));
        int minute = stoi(time.substr(3, 2));
        
        if (hour < 0 || hour > 23) {
            cout << "[WARNING] Invalid hour! Must be between 00 and 23.\n";
            cout << "Please enter again.\n";
            continue;
        }
        
        if (minute < 0 || minute > 59) {
            cout << "[WARNING] Invalid minute! Must be between 00 and 59.\n";
            cout << "Please enter again.\n";
            continue;
        }
        
        return time;
    }
}

string getAttendanceStatus(const string& prompt) {
    while (true) {
        string status = getNonEmptyString(prompt);
        string lowerStatus = toLower(status);
        
        if (lowerStatus == "present") {
            return "Present";
        } else if (lowerStatus == "absent") {
            return "Absent";
        } else if (lowerStatus == "late") {
            return "Late";
        } else if (lowerStatus == "leave") {
            return "Leave";
        } else {
            cout << "[WARNING] Invalid status! Must be: Present, Absent, Late, or Leave\n";
            cout << "Please enter again.\n";
        }
    }
}

int getMenuChoice(const string& prompt, int min, int max) {
    int choice;
    while (true) {
        cout << prompt;
        if (cin >> choice) {
            if (choice >= min && choice <= max) {
                clearInputBuffer();
                return choice;
            } else {
                cout << "[WARNING] Invalid choice!\n";
                cout << "Please enter a number from the menu.\n";
                cout << "Try again:\n";
            }
        } else {
            cout << "[WARNING] Invalid choice!\n";
            cout << "Please enter a number from the menu.\n";
            cout << "Try again:\n";
            clearInputBuffer();
        }
    }
}

bool confirmAction(const string& message) {
    while (true) {
        string response = getNonEmptyString(message + " (y/n): ");
        string lowerResponse = toLower(response);
        
        if (lowerResponse == "y" || lowerResponse == "yes") {
            return true;
        } else if (lowerResponse == "n" || lowerResponse == "no") {
            return false;
        } else {
            cout << "[WARNING] Please enter 'y' for yes or 'n' for no.\n";
        }
    }
}

// ==========================================
// EMPLOYEE FILE FUNCTIONS
// ==========================================

void saveEmployeesToBinary() {
    ofstream outFile("employees.dat", ios::binary);
    if (!outFile) {
        cout << "[ERROR] Could not open employees.dat for writing!\n";
        return;
    }
    
    size_t count = employees.size();
    outFile.write(reinterpret_cast<char*>(&count), sizeof(count));
    
    for (size_t i = 0; i < employees.size(); i++) {
        Employee emp = employees[i];
        size_t idLen = emp.employeeId.length();
        outFile.write(reinterpret_cast<char*>(&idLen), sizeof(idLen));
        outFile.write(emp.employeeId.c_str(), idLen);
        
        size_t nameLen = emp.name.length();
        outFile.write(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        outFile.write(emp.name.c_str(), nameLen);
        
        outFile.write(reinterpret_cast<char*>(&emp.age), sizeof(emp.age));
        
        size_t genderLen = emp.gender.length();
        outFile.write(reinterpret_cast<char*>(&genderLen), sizeof(genderLen));
        outFile.write(emp.gender.c_str(), genderLen);
        
        size_t phoneLen = emp.phone.length();
        outFile.write(reinterpret_cast<char*>(&phoneLen), sizeof(phoneLen));
        outFile.write(emp.phone.c_str(), phoneLen);
        
        size_t deptLen = emp.department.length();
        outFile.write(reinterpret_cast<char*>(&deptLen), sizeof(deptLen));
        outFile.write(emp.department.c_str(), deptLen);
        
        size_t posLen = emp.position.length();
        outFile.write(reinterpret_cast<char*>(&posLen), sizeof(posLen));
        outFile.write(emp.position.c_str(), posLen);
    }
    
    outFile.close();
}

void saveEmployeesToText() {
    ofstream outFile("employees.txt");
    if (!outFile) {
        cout << "[ERROR] Could not open employees.txt for writing!\n";
        return;
    }
    
    outFile << "Employee ID,Name,Age,Gender,Phone,Department,Position\n";
    for (const auto& emp : employees) {
        outFile << emp.employeeId << "," << emp.name << "," << emp.age << ","
                << emp.gender << "," << emp.phone << "," << emp.department << ","
                << emp.position << "\n";
    }
    
    outFile.close();
}

void loadEmployeesFromBinary() {
    ifstream inFile("employees.dat", ios::binary);
    if (!inFile) {
        return; // File doesn't exist yet
    }
    
    size_t count;
    inFile.read(reinterpret_cast<char*>(&count), sizeof(count));
    
    employees.clear();
    
    for (size_t i = 0; i < count; i++) {
        Employee emp;
        
        size_t idLen;
        inFile.read(reinterpret_cast<char*>(&idLen), sizeof(idLen));
        emp.employeeId.resize(idLen);
        inFile.read(&emp.employeeId[0], idLen);
        
        size_t nameLen;
        inFile.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        emp.name.resize(nameLen);
        inFile.read(&emp.name[0], nameLen);
        
        inFile.read(reinterpret_cast<char*>(&emp.age), sizeof(emp.age));
        
        size_t genderLen;
        inFile.read(reinterpret_cast<char*>(&genderLen), sizeof(genderLen));
        emp.gender.resize(genderLen);
        inFile.read(&emp.gender[0], genderLen);
        
        size_t phoneLen;
        inFile.read(reinterpret_cast<char*>(&phoneLen), sizeof(phoneLen));
        emp.phone.resize(phoneLen);
        inFile.read(&emp.phone[0], phoneLen);
        
        size_t deptLen;
        inFile.read(reinterpret_cast<char*>(&deptLen), sizeof(deptLen));
        emp.department.resize(deptLen);
        inFile.read(&emp.department[0], deptLen);
        
        size_t posLen;
        inFile.read(reinterpret_cast<char*>(&posLen), sizeof(posLen));
        emp.position.resize(posLen);
        inFile.read(&emp.position[0], posLen);
        
        employees.push_back(emp);
    }
    
    inFile.close();
}

void loadEmployeesFromText() {
    ifstream inFile("employees.txt");
    if (!inFile) {
        return; // File doesn't exist yet
    }
    
    string line;
    getline(inFile, line); // Skip header
    
    employees.clear();
    
    while (getline(inFile, line)) {
        if (line.empty()) continue;
        
        stringstream ss(line);
        Employee emp;
        string temp;
        
        getline(ss, emp.employeeId, ',');
        getline(ss, emp.name, ',');
        getline(ss, temp, ',');
        emp.age = stoi(temp);
        getline(ss, emp.gender, ',');
        getline(ss, emp.phone, ',');
        getline(ss, emp.department, ',');
        getline(ss, emp.position, ',');
        
        employees.push_back(emp);
    }
    
    inFile.close();
}

void saveEmployees() {
    saveEmployeesToBinary();
    saveEmployeesToText();
}

void loadEmployees() {
    // Try binary first, fallback to text
    ifstream binaryCheck("employees.dat", ios::binary);
    if (binaryCheck) {
        loadEmployeesFromBinary();
    } else {
        loadEmployeesFromText();
    }
    binaryCheck.close();
}

// ==========================================
// ATTENDANCE FILE FUNCTIONS
// ==========================================

void saveAttendanceToBinary() {
    ofstream outFile("attendance.dat", ios::binary);
    if (!outFile) {
        cout << "[ERROR] Could not open attendance.dat for writing!\n";
        return;
    }
    
    size_t count = attendanceRecords.size();
    outFile.write(reinterpret_cast<char*>(&count), sizeof(count));
    
    for (size_t i = 0; i < attendanceRecords.size(); i++) {
        Attendance att = attendanceRecords[i];
        size_t idLen = att.employeeId.length();
        outFile.write(reinterpret_cast<char*>(&idLen), sizeof(idLen));
        outFile.write(att.employeeId.c_str(), idLen);
        
        size_t nameLen = att.employeeName.length();
        outFile.write(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        outFile.write(att.employeeName.c_str(), nameLen);
        
        size_t dateLen = att.date.length();
        outFile.write(reinterpret_cast<char*>(&dateLen), sizeof(dateLen));
        outFile.write(att.date.c_str(), dateLen);
        
        size_t inLen = att.checkIn.length();
        outFile.write(reinterpret_cast<char*>(&inLen), sizeof(inLen));
        outFile.write(att.checkIn.c_str(), inLen);
        
        size_t outLen = att.checkOut.length();
        outFile.write(reinterpret_cast<char*>(&outLen), sizeof(outLen));
        outFile.write(att.checkOut.c_str(), outLen);
        
        size_t statusLen = att.status.length();
        outFile.write(reinterpret_cast<char*>(&statusLen), sizeof(statusLen));
        outFile.write(att.status.c_str(), statusLen);
    }
    
    outFile.close();
}

void saveAttendanceToText() {
    ofstream outFile("attendance.txt");
    if (!outFile) {
        cout << "[ERROR] Could not open attendance.txt for writing!\n";
        return;
    }
    
    outFile << "Employee ID,Employee Name,Date,Check In,Check Out,Status\n";
    for (const auto& att : attendanceRecords) {
        outFile << att.employeeId << "," << att.employeeName << "," << att.date << ","
                << att.checkIn << "," << att.checkOut << "," << att.status << "\n";
    }
    
    outFile.close();
}

void loadAttendanceFromBinary() {
    ifstream inFile("attendance.dat", ios::binary);
    if (!inFile) {
        return; // File doesn't exist yet
    }
    
    size_t count;
    inFile.read(reinterpret_cast<char*>(&count), sizeof(count));
    
    attendanceRecords.clear();
    
    for (size_t i = 0; i < count; i++) {
        Attendance att;
        
        size_t idLen;
        inFile.read(reinterpret_cast<char*>(&idLen), sizeof(idLen));
        att.employeeId.resize(idLen);
        inFile.read(&att.employeeId[0], idLen);
        
        size_t nameLen;
        inFile.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        att.employeeName.resize(nameLen);
        inFile.read(&att.employeeName[0], nameLen);
        
        size_t dateLen;
        inFile.read(reinterpret_cast<char*>(&dateLen), sizeof(dateLen));
        att.date.resize(dateLen);
        inFile.read(&att.date[0], dateLen);
        
        size_t inLen;
        inFile.read(reinterpret_cast<char*>(&inLen), sizeof(inLen));
        att.checkIn.resize(inLen);
        inFile.read(&att.checkIn[0], inLen);
        
        size_t outLen;
        inFile.read(reinterpret_cast<char*>(&outLen), sizeof(outLen));
        att.checkOut.resize(outLen);
        inFile.read(&att.checkOut[0], outLen);
        
        size_t statusLen;
        inFile.read(reinterpret_cast<char*>(&statusLen), sizeof(statusLen));
        att.status.resize(statusLen);
        inFile.read(&att.status[0], statusLen);
        
        attendanceRecords.push_back(att);
    }
    
    inFile.close();
}

void loadAttendanceFromText() {
    ifstream inFile("attendance.txt");
    if (!inFile) {
        return; // File doesn't exist yet
    }
    
    string line;
    getline(inFile, line); // Skip header
    
    attendanceRecords.clear();
    
    while (getline(inFile, line)) {
        if (line.empty()) continue;
        
        stringstream ss(line);
        Attendance att;
        
        getline(ss, att.employeeId, ',');
        getline(ss, att.employeeName, ',');
        getline(ss, att.date, ',');
        getline(ss, att.checkIn, ',');
        getline(ss, att.checkOut, ',');
        getline(ss, att.status, ',');
        
        attendanceRecords.push_back(att);
    }
    
    inFile.close();
}

void saveAttendance() {
    saveAttendanceToBinary();
    saveAttendanceToText();
}

void loadAttendance() {
    // Try binary first, fallback to text
    ifstream binaryCheck("attendance.dat", ios::binary);
    if (binaryCheck) {
        loadAttendanceFromBinary();
    } else {
        loadAttendanceFromText();
    }
    binaryCheck.close();
}

// ==========================================
// EMPLOYEE FUNCTIONS
// ==========================================

int findEmployeeIndex(const string& employeeId) {
    for (size_t i = 0; i < employees.size(); i++) {
        if (employees[i].employeeId == employeeId) {
            return i;
        }
    }
    return -1;
}

bool employeeIdExists(const string& employeeId) {
    return findEmployeeIndex(employeeId) != -1;
}

void addEmployee() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "         ADD EMPLOYEE\n";
    cout << "========================================\n";
    
    string employeeId;
    while (true) {
        employeeId = getNonEmptyString("Enter Employee ID: ");
        if (employeeIdExists(employeeId)) {
            cout << "\n+======================================+\n";
            cout << "| [WARNING] This Employee ID already exists! |\n";
            cout << "| Please enter a different ID.        |\n";
            cout << "+======================================+\n";
        } else {
            break;
        }
    }
    
    string name = getNonEmptyString("Enter Name: ");
    int age = getInteger("Enter Age: ", 18, 100);
    string gender = getNonEmptyString("Enter Gender (Male/Female/Other): ");
    string phone = getPhone("Enter Phone: ");
    string department = getNonEmptyString("Enter Department: ");
    string position = getNonEmptyString("Enter Position: ");
    
    Employee emp;
    emp.employeeId = employeeId;
    emp.name = name;
    emp.age = age;
    emp.gender = gender;
    emp.phone = phone;
    emp.department = department;
    emp.position = position;
    
    employees.push_back(emp);
    saveEmployees();
    
    cout << "\n+======================================+\n";
    cout << "|       Employee added successfully! |\n";
    cout << "+======================================+\n";
}

void viewEmployees() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "         EMPLOYEE LIST\n";
    cout << "========================================\n";
    if (employees.empty()) {
        cout << "\n+======================================+\n";
        cout << "|      No employees found!           |\n";
        cout << "+======================================+\n";
        return;
    }
    
    cout << "\n+-------------+---------------------+------+----------+---------------+------------------+------------------+\n";
    cout << "|    ID      |        Name         | Age  |  Gender  |     Phone     |    Department    |     Position     |\n";
    cout << "+-------------+---------------------+------+----------+---------------+------------------+------------------+\n";
    
    for (const auto& emp : employees) {
        cout << "| " << left << setw(11) << emp.employeeId << " | " 
             << setw(19) << emp.name << " | " << setw(4) << emp.age << " | " 
             << setw(8) << emp.gender << " | " << setw(13) << emp.phone << " | " 
             << setw(16) << emp.department << " | " << setw(16) << emp.position << " |\n";
    }
    
    cout << "+-------------+---------------------+------+----------+---------------+------------------+------------------+\n";
}

void searchEmployee() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "         SEARCH EMPLOYEE\n";
    cout << "========================================\n";
    cout << "\n+------------------------------------------+\n";
    cout << "|  [1] Search by Employee ID                |\n";
    cout << "|  [2] Search by Name                       |\n";
    cout << "|  [0] Back                                 |\n";
    cout << "+------------------------------------------+\n";
    
    int choice = getMenuChoice("\nEnter choice: ", 0, 2);
    
    if (choice == 0) return;
    
    if (choice == 1) {
        string employeeId = getNonEmptyString("Enter Employee ID: ");
        int index = findEmployeeIndex(employeeId);
        
        if (index == -1) {
            cout << "\n+======================================+\n";
            cout << "|  [WARNING] Employee ID not found!  |\n";
            cout << "+======================================+\n";
            return;
        }
        
        const auto& emp = employees[index];
        cout << "\n+======================================+\n";
        cout << "|           EMPLOYEE DETAILS          |\n";
        cout << "+======================================+\n";
        cout << "| Employee ID  : " << setw(23) << emp.employeeId << "|\n";
        cout << "| Name         : " << setw(23) << emp.name << "|\n";
        cout << "| Age          : " << setw(23) << emp.age << "|\n";
        cout << "| Gender       : " << setw(23) << emp.gender << "|\n";
        cout << "| Phone        : " << setw(23) << emp.phone << "|\n";
        cout << "| Department   : " << setw(23) << emp.department << "|\n";
        cout << "| Position     : " << setw(23) << emp.position << "|\n";
        cout << "+======================================+\n";
    } else {
        string searchName = getNonEmptyString("Enter Name: ");
        string lowerSearch = toLower(searchName);
        
        bool found = false;
        for (const auto& emp : employees) {
            if (toLower(emp.name).find(lowerSearch) != string::npos) {
                if (!found) {
                    cout << "\n+======================================+\n";
                    cout << "|            SEARCH RESULTS          |\n";
                    cout << "+======================================+\n";
                    cout << "\n+-------------+---------------------+------+----------+---------------+------------------+------------------+\n";
                    cout << "|    ID      |        Name         | Age  |  Gender  |     Phone     |    Department    |     Position     |\n";
                    cout << "+-------------+---------------------+------+----------+---------------+------------------+------------------+\n";
                    found = true;
                }
                cout << "| " << left << setw(11) << emp.employeeId << " | " 
                     << setw(19) << emp.name << " | " << setw(4) << emp.age << " | " 
                     << setw(8) << emp.gender << " | " << setw(13) << emp.phone << " | " 
                     << setw(16) << emp.department << " | " << setw(16) << emp.position << " |\n";
            }
        }
        
        if (found) {
            cout << "+-------------+---------------------+------+----------+---------------+------------------+------------------+\n";
        }
        
        if (!found) {
            cout << "\n+======================================+\n";
            cout << "|  No employee found with that name! |\n";
            cout << "+======================================+\n";
        }
    }
}

void updateEmployee() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "         UPDATE EMPLOYEE\n";
    cout << "========================================\n";
    
    string employeeId = getNonEmptyString("Enter Employee ID to update: ");
    int index = findEmployeeIndex(employeeId);
    
    if (index == -1) {
        cout << "\n+======================================+\n";
        cout << "|  [WARNING] Employee ID not found!  |\n";
        cout << "+======================================+\n";
        return;
    }
    
    auto& emp = employees[index];
    
    cout << "\n+======================================+\n";
    cout << "|           CURRENT DETAILS          |\n";
    cout << "+======================================+\n";
    cout << "| Name         : " << setw(23) << emp.name << "|\n";
    cout << "| Age          : " << setw(23) << emp.age << "|\n";
    cout << "| Gender       : " << setw(23) << emp.gender << "|\n";
    cout << "| Phone        : " << setw(23) << emp.phone << "|\n";
    cout << "| Department   : " << setw(23) << emp.department << "|\n";
    cout << "| Position     : " << setw(23) << emp.position << "|\n";
    cout << "+======================================+\n";
    
    cout << "\n+======================================+\n";
    cout << "|  Enter new details (blank = keep current) |\n";
    cout << "+======================================+\n";
    
    string newName;
    cout << "Enter Name [" << emp.name << "]: ";
    getline(cin, newName);
    if (!newName.empty()) {
        size_t start = newName.find_first_not_of(" \t");
        size_t end = newName.find_last_not_of(" \t");
        if (start != string::npos) {
            emp.name = newName.substr(start, end - start + 1);
        }
    }
    
    string ageInput;
    cout << "Enter Age [" << emp.age << "]: ";
    getline(cin, ageInput);
    if (!ageInput.empty()) {
        try {
            int newAge = stoi(ageInput);
            if (newAge >= 18 && newAge <= 100) {
                emp.age = newAge;
            } else {
                cout << "[WARNING] Invalid age range. Keeping current value.\n";
            }
        } catch (...) {
            cout << "[WARNING] Invalid age. Keeping current value.\n";
        }
    }
    
    string newGender;
    cout << "Enter Gender [" << emp.gender << "]: ";
    getline(cin, newGender);
    if (!newGender.empty()) {
        size_t start = newGender.find_first_not_of(" \t");
        size_t end = newGender.find_last_not_of(" \t");
        if (start != string::npos) {
            emp.gender = newGender.substr(start, end - start + 1);
        }
    }
    
    string newPhone;
    cout << "Enter Phone [" << emp.phone << "]: ";
    getline(cin, newPhone);
    if (!newPhone.empty()) {
        bool valid = true;
        for (char c : newPhone) {
            if (!isdigit(c)) {
                valid = false;
                break;
            }
        }
        if (valid && newPhone.length() >= 7 && newPhone.length() <= 15) {
            emp.phone = newPhone;
        } else {
            cout << "[WARNING] Invalid phone format. Keeping current value.\n";
        }
    }
    
    string newDept;
    cout << "Enter Department [" << emp.department << "]: ";
    getline(cin, newDept);
    if (!newDept.empty()) {
        size_t start = newDept.find_first_not_of(" \t");
        size_t end = newDept.find_last_not_of(" \t");
        if (start != string::npos) {
            emp.department = newDept.substr(start, end - start + 1);
        }
    }
    
    string newPos;
    cout << "Enter Position [" << emp.position << "]: ";
    getline(cin, newPos);
    if (!newPos.empty()) {
        size_t start = newPos.find_first_not_of(" \t");
        size_t end = newPos.find_last_not_of(" \t");
        if (start != string::npos) {
            emp.position = newPos.substr(start, end - start + 1);
        }
    }
    
    saveEmployees();
    cout << "\n+======================================+\n";
    cout << "|      Employee updated successfully! |\n";
    cout << "+======================================+\n";
}

void deleteEmployee() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "         DELETE EMPLOYEE\n";
    cout << "========================================\n";
    
    string employeeId = getNonEmptyString("Enter Employee ID to delete: ");
    int index = findEmployeeIndex(employeeId);
    
    if (index == -1) {
        cout << "\n+======================================+\n";
        cout << "|  [WARNING] Employee ID not found!  |\n";
        cout << "+======================================+\n";
        return;
    }
    
    const auto& emp = employees[index];
    cout << "\n+======================================+\n";
    cout << "|          EMPLOYEE TO DELETE         |\n";
    cout << "+======================================+\n";
    cout << "| ID           : " << setw(23) << emp.employeeId << "|\n";
    cout << "| Name         : " << setw(23) << emp.name << "|\n";
    cout << "+======================================+\n";
    
    if (confirmAction("Are you sure you want to delete this employee?")) {
        employees.erase(employees.begin() + index);
        saveEmployees();
        cout << "\n+======================================+\n";
        cout << "|      Employee deleted successfully! |\n";
        cout << "+======================================+\n";
    } else {
        cout << "\n+======================================+\n";
        cout << "|          Deletion cancelled.       |\n";
        cout << "+======================================+\n";
    }
}

// ==========================================
// ATTENDANCE FUNCTIONS
// ==========================================

void markAttendance() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "         MARK ATTENDANCE\n";
    cout << "========================================\n";
    
    string employeeId = getNonEmptyString("Enter Employee ID: ");
    int index = findEmployeeIndex(employeeId);
    
    if (index == -1) {
        cout << "\n+======================================+\n";
        cout << "|  [WARNING] Employee ID not found!  |\n";
        cout << "+======================================+\n";
        return;
    }
    
    const auto& emp = employees[index];
    
    Attendance att;
    att.employeeId = emp.employeeId;
    att.employeeName = emp.name;
    att.date = getValidDate("Enter Date (DD/MM/YYYY): ");
    att.checkIn = getValidTime("Enter Check In Time (HH:MM): ");
    att.checkOut = getValidTime("Enter Check Out Time (HH:MM): ");
    att.status = getAttendanceStatus("Enter Status (Present/Absent/Late/Leave): ");
    
    attendanceRecords.push_back(att);
    saveAttendance();
    
    cout << "\n+======================================+\n";
    cout << "|    Attendance marked successfully!  |\n";
    cout << "+======================================+\n";
}

void viewTodayAttendance() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "         TODAY'S ATTENDANCE\n";
    cout << "========================================\n";
    
    string today = getValidDate("Enter Date (DD/MM/YYYY): ");
    
    bool found = false;
    for (const auto& att : attendanceRecords) {
        if (att.date == today) {
            if (!found) {
                cout << "\n+-------------+---------------------+------------+-----------+-----------+----------+\n";
                cout << "|    ID      |        Name         |    Date     | Check In  | Check Out |  Status  |\n";
                cout << "+-------------+---------------------+------------+-----------+-----------+----------+\n";
                found = true;
            }
            cout << "| " << left << setw(11) << att.employeeId << " | " 
                 << setw(19) << att.employeeName << " | " 
                 << setw(10) << att.date << " | " 
                 << setw(9) << att.checkIn << " | " 
                 << setw(9) << att.checkOut << " | " 
                 << setw(8) << att.status << " |\n";
        }
    }
    
    if (found) {
        cout << "+-------------+---------------------+------------+-----------+-----------+----------+\n";
    }
    
    if (!found) {
        cout << "\n+======================================+\n";
        cout << "| No attendance records found for this date |\n";
        cout << "+======================================+\n";
    }
}

void viewAttendanceHistory() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "         ATTENDANCE HISTORY\n";
    cout << "========================================\n";
    if (attendanceRecords.empty()) {
        cout << "\n+======================================+\n";
        cout << "|   No attendance records found!    |\n";
        cout << "+======================================+\n";
        return;
    }
    
    cout << "\n+-------------+---------------------+------------+-----------+-----------+----------+\n";
    cout << "|    ID      |        Name         |    Date     | Check In  | Check Out |  Status  |\n";
    cout << "+-------------+---------------------+------------+-----------+-----------+----------+\n";
    
    for (const auto& att : attendanceRecords) {
        cout << "| " << left << setw(11) << att.employeeId << " | " 
             << setw(19) << att.employeeName << " | " 
             << setw(10) << att.date << " | " 
             << setw(9) << att.checkIn << " | " 
             << setw(9) << att.checkOut << " | " 
             << setw(8) << att.status << " |\n";
    }
    
    cout << "+-------------+---------------------+------------+-----------+-----------+----------+\n";
}

void searchAttendance() {
    clearScreen();
    cout << "\n";
    cout << "========================================\n";
    cout << "         SEARCH ATTENDANCE\n";
    cout << "========================================\n";
    cout << "\n+------------------------------------------+\n";
    cout << "|  [1] Search by Employee ID                |\n";
    cout << "|  [2] Search by Date                       |\n";
    cout << "|  [0] Back                                 |\n";
    cout << "+------------------------------------------+\n";
    
    int choice = getMenuChoice("\nEnter choice: ", 0, 2);
    
    if (choice == 0) return;
    
    if (choice == 1) {
        string employeeId = getNonEmptyString("Enter Employee ID: ");
        
        bool found = false;
        for (const auto& att : attendanceRecords) {
            if (att.employeeId == employeeId) {
                if (!found) {
                    cout << "\n+======================================+\n";
                    cout << "|            SEARCH RESULTS          |\n";
                    cout << "+======================================+\n";
                    cout << "\n+-------------+---------------------+------------+-----------+-----------+----------+\n";
                    cout << "|    ID      |        Name         |    Date     | Check In  | Check Out |  Status  |\n";
                    cout << "+-------------+---------------------+------------+-----------+-----------+----------+\n";
                    found = true;
                }
                cout << "| " << left << setw(11) << att.employeeId << " | " 
                     << setw(19) << att.employeeName << " | " 
                     << setw(10) << att.date << " | " 
                     << setw(9) << att.checkIn << " | " 
                     << setw(9) << att.checkOut << " | " 
                     << setw(8) << att.status << " |\n";
            }
        }
        
        if (found) {
            cout << "+-------------+---------------------+------------+-----------+-----------+----------+\n";
        }
        
        if (!found) {
            cout << "\n+======================================+\n";
            cout << "| No attendance records found for this employee |\n";
            cout << "+======================================+\n";
        }
    } else {
        string date = getValidDate("Enter Date (DD/MM/YYYY): ");
        
        bool found = false;
        for (const auto& att : attendanceRecords) {
            if (att.date == date) {
                if (!found) {
                    cout << "\n+======================================+\n";
                    cout << "|            SEARCH RESULTS          |\n";
                    cout << "+======================================+\n";
                    cout << "\n+-------------+---------------------+------------+-----------+-----------+----------+\n";
                    cout << "|    ID      |        Name         |    Date     | Check In  | Check Out |  Status  |\n";
                    cout << "+-------------+---------------------+------------+-----------+-----------+----------+\n";
                    found = true;
                }
                cout << "| " << left << setw(11) << att.employeeId << " | " 
                     << setw(19) << att.employeeName << " | " 
                     << setw(10) << att.date << " | " 
                     << setw(9) << att.checkIn << " | " 
                     << setw(9) << att.checkOut << " | " 
                     << setw(8) << att.status << " |\n";
            }
        }
        
        if (found) {
            cout << "+-------------+---------------------+------------+-----------+-----------+----------+\n";
        }
        
        if (!found) {
            cout << "\n+======================================+\n";
            cout << "| No attendance records found for this date   |\n";
            cout << "+======================================+\n";
        }
    }
}

// ==========================================
// MENU FUNCTIONS
// ==========================================

bool adminLogin() {
    clearScreen();
    const string USERNAME = "admin";
    const string PASSWORD = "admin123";
    
    cout << "\n";
    cout << "========================================\n";
    cout << "           ADMIN LOGIN\n";
    cout << "========================================\n";
    
    string username = getNonEmptyString("Username: ");
    
    cout << "Password: ";
    string password;
    
    // Try to use _getch() for hidden password input (Windows)
    char ch;
    while (true) {
        ch = _getch();
        if (ch == '\r') {
            break;
        } else if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";
            }
        } else if (ch != '\0') {
            password += ch;
            cout << '*';
        }
    }
    cout << "\n";
    
    if (username == USERNAME && password == PASSWORD) {
        cout << "\n+======================================+\n";
        cout << "|          Login successful!          |\n";
        cout << "+======================================+\n";
        return true;
    } else {
        cout << "\n+======================================+\n";
        cout << "| [WARNING] Invalid username or password! |\n";
        cout << "+======================================+\n";
        return false;
    }
}

void employeeManagementMenu() {
    while (true) {
        clearScreen();
        cout << "\n";
        cout << "========================================\n";
        cout << "       EMPLOYEE MANAGEMENT\n";
        cout << "========================================\n";
        cout << "\n";
        cout << "+------------------------------------------+\n";
        cout << "|  [1] Add Employee                        |\n";
        cout << "|  [2] View Employees                       |\n";
        cout << "|  [3] Search Employee                      |\n";
        cout << "|  [4] Update Employee                      |\n";
        cout << "|  [5] Delete Employee                      |\n";
        cout << "|  [0] Back to Main Menu                    |\n";
        cout << "+------------------------------------------+\n";
        
        cout << "\n+------------------------------------------+\n";
        cout << "| Enter your choice: ";
        
        int choice;
        if (!(cin >> choice)) {
            cout << "Invalid input!                       |\n";
            cout << "+------------------------------------------+\n";
            clearInputBuffer();
            pauseScreen();
            continue;
        }
        clearInputBuffer();
        
        cout << "                                        |\n";
        cout << "+------------------------------------------+\n";
        
        if (choice < 0 || choice > 5) {
            cout << "\n+======================================+\n";
            cout << "| [WARNING] Invalid choice!          |\n";
            cout << "| Please enter a number from the menu.|\n";
            cout << "+======================================+\n";
            pauseScreen();
            continue;
        }
        
        switch (choice) {
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
                updateEmployee();
                break;
            case 5:
                deleteEmployee();
                break;
            case 0:
                return;
        }
        
        pauseScreen();
    }
}

void attendanceManagementMenu() {
    while (true) {
        clearScreen();
        cout << "\n";
        cout << "========================================\n";
        cout << "       ATTENDANCE MANAGEMENT\n";
        cout << "========================================\n";
        cout << "\n";
        cout << "+------------------------------------------+\n";
        cout << "|  [1] Mark Attendance                     |\n";
        cout << "|  [2] View Today's Attendance             |\n";
        cout << "|  [3] View Attendance History             |\n";
        cout << "|  [4] Search Attendance                   |\n";
        cout << "|  [0] Back to Main Menu                    |\n";
        cout << "+------------------------------------------+\n";
        
        cout << "\n+------------------------------------------+\n";
        cout << "| Enter your choice: ";
        
        int choice;
        if (!(cin >> choice)) {
            cout << "Invalid input!                       |\n";
            cout << "+------------------------------------------+\n";
            clearInputBuffer();
            pauseScreen();
            continue;
        }
        clearInputBuffer();
        
        cout << "                                        |\n";
        cout << "+------------------------------------------+\n";
        
        if (choice < 0 || choice > 4) {
            cout << "\n+======================================+\n";
            cout << "| [WARNING] Invalid choice!          |\n";
            cout << "| Please enter a number from the menu.|\n";
            cout << "+======================================+\n";
            pauseScreen();
            continue;
        }
        
        switch (choice) {
            case 1:
                markAttendance();
                break;
            case 2:
                viewTodayAttendance();
                break;
            case 3:
                viewAttendanceHistory();
                break;
            case 4:
                searchAttendance();
                break;
            case 0:
                return;
        }
        
        pauseScreen();
    }
}

void searchEmployeeMenu() {
    clearScreen();
    searchEmployee();
    pauseScreen();
}

void mainMenu() {
    clearScreen();
    while (true) {
        cout << "\n";
        cout << "================================================\n";
        cout << "                                                \n";
        cout << "           EMPLOYEE ATTENDANCE MANAGEMENT SYSTEM\n";
        cout << "                                                \n";
        cout << "================================================\n";
        cout << "\n";
        cout << "+----------------------------------------------+\n";
        cout << "|  [1] Employee Management                     |\n";
        cout << "|  [2] Attendance Management                    |\n";
        cout << "|  [3] Search Employee                          |\n";
        cout << "|  [4] Exit                                     |\n";
        cout << "+----------------------------------------------+\n";
        
        cout << "\n+----------------------------------------------+\n";
        cout << "| Enter your choice: ";
        
        int choice;
        if (!(cin >> choice)) {
            cout << "Invalid input!                          |\n";
            cout << "+----------------------------------------------+\n";
            clearInputBuffer();
            pauseScreen();
            continue;
        }
        clearInputBuffer();
        
        cout << "                                              |\n";
        cout << "+----------------------------------------------+\n";
        
        if (choice < 1 || choice > 4) {
            cout << "\n+==============================================+\n";
            cout << "| [WARNING] Invalid choice!                  |\n";
            cout << "| Please enter a number from the menu.       |\n";
            cout << "+==============================================+\n";
            pauseScreen();
            continue;
        }
        
        switch (choice) {
            case 1:
                employeeManagementMenu();
                break;
            case 2:
                attendanceManagementMenu();
                break;
            case 3:
                searchEmployeeMenu();
                break;
            case 4:
                clearScreen();
                cout << "\n";
                cout << "================================================\n";
                cout << "                                                \n";
                cout << "       Thank you for using EAMS!                 \n";
                cout << "               Goodbye!                         \n";
                cout << "                                                \n";
                cout << "================================================\n";
                return;
        }
    }
}

// ==========================================
// MAIN FUNCTION
// ==========================================

int main() {
    // Load saved data
    loadEmployees();
    loadAttendance();
    
    // Admin login
    if (!adminLogin()) {
        cout << "\nAccess denied. Exiting program.\n";
        return 0;
    }
    
    // Show main menu
    mainMenu();
    
    return 0;
}
