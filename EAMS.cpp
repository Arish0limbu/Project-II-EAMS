#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <limits>
#include <cctype>
#include <cstdlib>
#include <cerrno>
#include <map>
#include <set>
#include <chrono>
#include <functional>

#ifdef _WIN32
#include <conio.h>
#include <direct.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#endif

// Platform-specific directory functions
#ifdef _WIN32
#define mkdir _mkdir
#define rmdir _rmdir
#define PATH_SEPARATOR "\\"
#else
#define mkdir(path) mkdir(path, 0755)
#define PATH_SEPARATOR "/"
#endif

// ============================================
// GLOBAL HELPER FUNCTIONS
// ============================================
char getPasswordChar() {
#ifdef _WIN32
    return _getch();
#else
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

// ============================================
// CONSTANTS
// ============================================
const std::string VERSION = "1.0.0";
const std::string DATA_DIR = "data";
const std::string BACKUP_DIR = "backup";
const std::string REPORTS_DIR = "reports";
const std::string LOG_FILE = "data/system.log";

const int MAX_LOGIN_ATTEMPTS = 3;
const int PASSWORD_MIN_LENGTH = 6;

// ============================================
// UTILITY FUNCTIONS
// ============================================
std::string toLowerCase(const std::string& text) {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string toUpperCase(const std::string& text) {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string trim(const std::string& text) {
    size_t start = text.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = text.find_last_not_of(" \t\n\r");
    return text.substr(start, end - start + 1);
}

std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << (tm.tm_year + 1900) << "-"
        << std::setfill('0') << std::setw(2) << (tm.tm_mon + 1) << "-"
        << std::setfill('0') << std::setw(2) << tm.tm_mday;
    return oss.str();
}

std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << tm.tm_hour << ":"
        << std::setfill('0') << std::setw(2) << tm.tm_min << ":"
        << std::setfill('0') << std::setw(2) << tm.tm_sec;
    return oss.str();
}

std::string getTimestamp() {
    return getCurrentDate() + " " + getCurrentTime();
}

int timeToMinutes(const std::string& time) {
    int hours = 0, minutes = 0;
    char sep;
    std::istringstream iss(time);
    iss >> hours >> sep >> minutes;
    return hours * 60 + minutes;
}

std::string minutesToTime(int totalMinutes) {
    int hours = totalMinutes / 60;
    int minutes = totalMinutes % 60;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setfill('0') << std::setw(2) << minutes;
    return oss.str();
}

// ============================================
// FILE SERIALIZATION FUNCTIONS
// ============================================
void writeString(std::ofstream& file, const std::string& value) {
    size_t length = value.length();
    file.write(reinterpret_cast<const char*>(&length), sizeof(length));
    file.write(value.c_str(), length);
}

std::string readString(std::ifstream& file) {
    size_t length;
    file.read(reinterpret_cast<char*>(&length), sizeof(length));
    if (length == 0) return "";
    std::string value(length, '\0');
    file.read(&value[0], length);
    return value;
}

void writeInt(std::ofstream& file, int value) {
    file.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

int readInt(std::ifstream& file) {
    int value;
    file.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

void writeBool(std::ofstream& file, bool value) {
    file.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

bool readBool(std::ifstream& file) {
    bool value;
    file.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

// ============================================
// UI FUNCTIONS
// ============================================
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printLine() {
    std::cout << std::string(50, '=') << std::endl;
}

void printHeader(const std::string& title) {
    clearScreen();
    printLine();
    std::cout << std::setw((50 + title.length()) / 2) << title << std::endl;
    printLine();
}

void printSuccess(const std::string& message) {
    std::cout << "[SUCCESS] " << message << std::endl;
}

void printError(const std::string& message) {
    std::cout << "[ERROR] " << message << std::endl;
}

void printWarning(const std::string& message) {
    std::cout << "[WARNING] " << message << std::endl;
}

void printInfo(const std::string& message) {
    std::cout << "[INFO] " << message << std::endl;
}

void pauseScreen() {
    std::cout << "\nPress ENTER to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// ============================================
// VALIDATION FUNCTIONS
// ============================================
bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInMonth(int month, int year) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year)) return 29;
    return days[month - 1];
}

bool isValidDate(const std::string& date) {
    if (date.length() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;
    
    try {
        int year = std::stoi(date.substr(0, 4));
        int month = std::stoi(date.substr(5, 2));
        int day = std::stoi(date.substr(8, 2));
        
        if (year < 1900 || year > 2100) return false;
        if (month < 1 || month > 12) return false;
        if (day < 1 || day > daysInMonth(month, year)) return false;
        
        return true;
    } catch (...) {
        return false;
    }
}

bool isValidTime(const std::string& time) {
    if (time.length() != 5 && time.length() != 8) return false;
    if (time[2] != ':') return false;
    
    try {
        int hours = std::stoi(time.substr(0, 2));
        int minutes = std::stoi(time.substr(3, 2));
        
        if (hours < 0 || hours > 23) return false;
        if (minutes < 0 || minutes > 59) return false;
        
        return true;
    } catch (...) {
        return false;
    }
}

bool isValidEmail(const std::string& email) {
    size_t atPos = email.find('@');
    size_t dotPos = email.rfind('.');
    
    if (atPos == std::string::npos || dotPos == std::string::npos) return false;
    if (atPos == 0 || atPos >= email.length() - 1) return false;
    if (dotPos <= atPos + 1 || dotPos >= email.length() - 1) return false;
    
    return true;
}

bool isValidPhone(const std::string& phone) {
    if (phone.empty()) return false;
    for (char c : phone) {
        if (!std::isdigit(c) && c != '+' && c != '-' && c != ' ') return false;
    }
    return phone.length() >= 10;
}

int getInt() {
    int value;
    while (true) {
        std::cin >> value;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            printError("Invalid input. Please enter a number.");
        } else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
    }
}

int getIntInRange(int min, int max) {
    while (true) {
        int value = getInt();
        if (value >= min && value <= max) {
            return value;
        }
        printError("Please enter a value between " + std::to_string(min) + " and " + std::to_string(max) + ".");
    }
}

std::string getNonEmptyString(const std::string& prompt) {
    std::string value;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, value);
        value = trim(value);
        if (!value.empty()) {
            return value;
        }
        printError("This field cannot be empty.");
    }
}

bool confirmAction(const std::string& message) {
    std::cout << message << " (Y/N): ";
    char choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return std::toupper(choice) == 'Y';
}

// ============================================
// DATA STRUCTURES
// ============================================
struct Employee {
    std::string employeeID;
    std::string name;
    std::string gender;
    int age;
    std::string phone;
    std::string email;
    std::string address;
    std::string department;
    std::string position;
    std::string joiningDate;
    std::string status;
    
    Employee() : age(0) {}
    
    void serialize(std::ofstream& file) const {
        writeString(file, employeeID);
        writeString(file, name);
        writeString(file, gender);
        writeInt(file, age);
        writeString(file, phone);
        writeString(file, email);
        writeString(file, address);
        writeString(file, department);
        writeString(file, position);
        writeString(file, joiningDate);
        writeString(file, status);
    }
    
    void deserialize(std::ifstream& file) {
        employeeID = readString(file);
        name = readString(file);
        gender = readString(file);
        age = readInt(file);
        phone = readString(file);
        email = readString(file);
        address = readString(file);
        department = readString(file);
        position = readString(file);
        joiningDate = readString(file);
        status = readString(file);
    }
    
    void writeTXT(std::ofstream& file) const {
        file << "EMPLOYEE\n";
        file << "ID=" << employeeID << "\n";
        file << "NAME=" << name << "\n";
        file << "GENDER=" << gender << "\n";
        file << "AGE=" << age << "\n";
        file << "PHONE=" << phone << "\n";
        file << "EMAIL=" << email << "\n";
        file << "ADDRESS=" << address << "\n";
        file << "DEPARTMENT=" << department << "\n";
        file << "POSITION=" << position << "\n";
        file << "JOINING_DATE=" << joiningDate << "\n";
        file << "STATUS=" << status << "\n";
        file << "END_EMPLOYEE\n";
    }
    
    static Employee readTXT(std::ifstream& file) {
        Employee emp;
        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line == "END_EMPLOYEE") break;
            if (line.find('=') != std::string::npos) {
                size_t pos = line.find('=');
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                if (key == "ID") emp.employeeID = value;
                else if (key == "NAME") emp.name = value;
                else if (key == "GENDER") emp.gender = value;
                else if (key == "AGE") emp.age = std::stoi(value);
                else if (key == "PHONE") emp.phone = value;
                else if (key == "EMAIL") emp.email = value;
                else if (key == "ADDRESS") emp.address = value;
                else if (key == "DEPARTMENT") emp.department = value;
                else if (key == "POSITION") emp.position = value;
                else if (key == "JOINING_DATE") emp.joiningDate = value;
                else if (key == "STATUS") emp.status = value;
            }
        }
        return emp;
    }
};

struct Attendance {
    std::string attendanceID;
    std::string employeeID;
    std::string date;
    std::string checkIn;
    std::string checkOut;
    std::string status;
    int lateMinutes;
    std::string remarks;
    
    Attendance() : lateMinutes(0) {}
    
    void serialize(std::ofstream& file) const {
        writeString(file, attendanceID);
        writeString(file, employeeID);
        writeString(file, date);
        writeString(file, checkIn);
        writeString(file, checkOut);
        writeString(file, status);
        writeInt(file, lateMinutes);
        writeString(file, remarks);
    }
    
    void deserialize(std::ifstream& file) {
        attendanceID = readString(file);
        employeeID = readString(file);
        date = readString(file);
        checkIn = readString(file);
        checkOut = readString(file);
        status = readString(file);
        lateMinutes = readInt(file);
        remarks = readString(file);
    }
    
    void writeTXT(std::ofstream& file) const {
        file << "ATTENDANCE\n";
        file << "ID=" << attendanceID << "\n";
        file << "EMPLOYEE_ID=" << employeeID << "\n";
        file << "DATE=" << date << "\n";
        file << "CHECK_IN=" << checkIn << "\n";
        file << "CHECK_OUT=" << checkOut << "\n";
        file << "STATUS=" << status << "\n";
        file << "LATE_MINUTES=" << lateMinutes << "\n";
        file << "REMARKS=" << remarks << "\n";
        file << "END_ATTENDANCE\n";
    }
    
    static Attendance readTXT(std::ifstream& file) {
        Attendance att;
        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line == "END_ATTENDANCE") break;
            if (line.find('=') != std::string::npos) {
                size_t pos = line.find('=');
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                if (key == "ID") att.attendanceID = value;
                else if (key == "EMPLOYEE_ID") att.employeeID = value;
                else if (key == "DATE") att.date = value;
                else if (key == "CHECK_IN") att.checkIn = value;
                else if (key == "CHECK_OUT") att.checkOut = value;
                else if (key == "STATUS") att.status = value;
                else if (key == "LATE_MINUTES") att.lateMinutes = std::stoi(value);
                else if (key == "REMARKS") att.remarks = value;
            }
        }
        return att;
    }
};

struct Admin {
    std::string username;
    std::string passwordHash;
    std::string role;
    std::string status;
    
    void serialize(std::ofstream& file) const {
        writeString(file, username);
        writeString(file, passwordHash);
        writeString(file, role);
        writeString(file, status);
    }
    
    void deserialize(std::ifstream& file) {
        username = readString(file);
        passwordHash = readString(file);
        role = readString(file);
        status = readString(file);
    }
    
    void writeTXT(std::ofstream& file) const {
        file << "ADMIN\n";
        file << "USERNAME=" << username << "\n";
        file << "PASSWORD_HASH=" << passwordHash << "\n";
        file << "ROLE=" << role << "\n";
        file << "STATUS=" << status << "\n";
        file << "END_ADMIN\n";
    }
    
    static Admin readTXT(std::ifstream& file) {
        Admin admin;
        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line == "END_ADMIN") break;
            if (line.find('=') != std::string::npos) {
                size_t pos = line.find('=');
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                if (key == "USERNAME") admin.username = value;
                else if (key == "PASSWORD_HASH") admin.passwordHash = value;
                else if (key == "ROLE") admin.role = value;
                else if (key == "STATUS") admin.status = value;
            }
        }
        return admin;
    }
    
    static std::string hashPassword(const std::string& password) {
        std::hash<std::string> hasher;
        return std::to_string(hasher(password));
    }
};

struct Settings {
    std::string companyName;
    std::string officeStartTime;
    std::string officeEndTime;
    int lateThreshold;
    std::string workingDays;
    
    Settings() : lateThreshold(15) {
        companyName = "My Company";
        officeStartTime = "09:00";
        officeEndTime = "17:00";
        workingDays = "Monday,Tuesday,Wednesday,Thursday,Friday";
    }
    
    void serialize(std::ofstream& file) const {
        writeString(file, companyName);
        writeString(file, officeStartTime);
        writeString(file, officeEndTime);
        writeInt(file, lateThreshold);
        writeString(file, workingDays);
    }
    
    void deserialize(std::ifstream& file) {
        companyName = readString(file);
        officeStartTime = readString(file);
        officeEndTime = readString(file);
        lateThreshold = readInt(file);
        workingDays = readString(file);
    }
    
    void writeTXT(std::ofstream& file) const {
        file << "SETTINGS\n";
        file << "COMPANY_NAME=" << companyName << "\n";
        file << "OFFICE_START_TIME=" << officeStartTime << "\n";
        file << "OFFICE_END_TIME=" << officeEndTime << "\n";
        file << "LATE_THRESHOLD=" << lateThreshold << "\n";
        file << "WORKING_DAYS=" << workingDays << "\n";
        file << "END_SETTINGS\n";
    }
    
    static Settings readTXT(std::ifstream& file) {
        Settings settings;
        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line == "END_SETTINGS") break;
            if (line.find('=') != std::string::npos) {
                size_t pos = line.find('=');
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                if (key == "COMPANY_NAME") settings.companyName = value;
                else if (key == "OFFICE_START_TIME") settings.officeStartTime = value;
                else if (key == "OFFICE_END_TIME") settings.officeEndTime = value;
                else if (key == "LATE_THRESHOLD") settings.lateThreshold = std::stoi(value);
                else if (key == "WORKING_DAYS") settings.workingDays = value;
            }
        }
        return settings;
    }
    
    bool isWorkingDay(const std::string& date) const {
        std::tm tm = {};
        std::istringstream iss(date);
        iss >> std::get_time(&tm, "%Y-%m-%d");
        if (iss.fail()) return true;
        
        const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        std::string dayName = days[tm.tm_wday];
        
        return workingDays.find(dayName) != std::string::npos;
    }
};

struct Holiday {
    std::string date;
    std::string name;
    
    void serialize(std::ofstream& file) const {
        writeString(file, date);
        writeString(file, name);
    }
    
    void deserialize(std::ifstream& file) {
        date = readString(file);
        name = readString(file);
    }
    
    void writeTXT(std::ofstream& file) const {
        file << "HOLIDAY\n";
        file << "DATE=" << date << "\n";
        file << "NAME=" << name << "\n";
        file << "END_HOLIDAY\n";
    }
    
    static Holiday readTXT(std::ifstream& file) {
        Holiday holiday;
        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line == "END_HOLIDAY") break;
            if (line.find('=') != std::string::npos) {
                size_t pos = line.find('=');
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                if (key == "DATE") holiday.date = value;
                else if (key == "NAME") holiday.name = value;
            }
        }
        return holiday;
    }
};

// ============================================
// LOGGER
// ============================================
class Logger {
private:
    std::string logFile;
    
public:
    Logger(const std::string& file) : logFile(file) {}
    
    void log(const std::string& message) {
        std::ofstream file(logFile, std::ios::app);
        if (file.is_open()) {
            file << "[" << getTimestamp() << "] " << message << std::endl;
            file.close();
        }
    }
    
    void logLogin(const std::string& username) {
        log("User '" + username + "' logged in.");
    }
    
    void logLogout(const std::string& username) {
        log("User '" + username + "' logged out.");
    }
    
    void logEmployeeAdded(const std::string& empID) {
        log("Employee '" + empID + "' added.");
    }
    
    void logEmployeeUpdated(const std::string& empID) {
        log("Employee '" + empID + "' updated.");
    }
    
    void logEmployeeDeactivated(const std::string& empID) {
        log("Employee '" + empID + "' deactivated.");
    }
    
    void logAttendanceMarked(const std::string& empID, const std::string& date, const std::string& status) {
        log("Attendance marked for '" + empID + "' on " + date + " as " + status + ".");
    }
    
    void logCheckIn(const std::string& empID, const std::string& time) {
        log("Employee '" + empID + "' checked in at " + time + ".");
    }
    
    void logCheckOut(const std::string& empID, const std::string& time) {
        log("Employee '" + empID + "' checked out at " + time + ".");
    }
    
    void logBackup(const std::string& type) {
        log("Backup created: " + type + ".");
    }
    
    void logRestore(const std::string& type) {
        log("Data restored from: " + type + ".");
    }
    
    void logSettingsChanged() {
        log("System settings changed.");
    }
};

// ============================================
// FILE MANAGER
// ============================================
class FileManager {
private:
    std::string dataDir;
    
    bool createDirectory(const std::string& path) {
#ifdef _WIN32
        return _mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
        return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
    }
    
    bool fileExists(const std::string& path) {
#ifdef _WIN32
        DWORD attr = GetFileAttributesA(path.c_str());
        return attr != INVALID_FILE_ATTRIBUTES;
#else
        struct stat buffer;
        return stat(path.c_str(), &buffer) == 0;
#endif
    }
    
    bool copyFile(const std::string& src, const std::string& dest) {
#ifdef _WIN32
        return CopyFileA(src.c_str(), dest.c_str(), FALSE) != 0;
#else
        std::ifstream srcFile(src, std::ios::binary);
        std::ofstream destFile(dest, std::ios::binary);
        if (!srcFile.is_open() || !destFile.is_open()) return false;
        destFile << srcFile.rdbuf();
        srcFile.close();
        destFile.close();
        return true;
#endif
    }
    
    std::vector<std::string> listDirectory(const std::string& path) {
        std::vector<std::string> files;
#ifdef _WIN32
        WIN32_FIND_DATAA findData;
        std::string searchPath = path + "\\*";
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    files.push_back(findData.cFileName);
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }
#else
        DIR* dir = opendir(path.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (entry->d_type == DT_REG) {
                    files.push_back(entry->d_name);
                }
            }
            closedir(dir);
        }
#endif
        return files;
    }
    
public:
    FileManager(const std::string& dir) : dataDir(dir) {
        createDirectory(dataDir);
        createDirectory(BACKUP_DIR);
        createDirectory(REPORTS_DIR);
    }
    
    template<typename T>
    bool saveBinary(const std::string& filename, const std::vector<T>& data) {
        std::string filepath = dataDir + PATH_SEPARATOR + filename;
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) return false;
        
        size_t count = data.size();
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));
        
        for (const auto& item : data) {
            item.serialize(file);
        }
        
        file.close();
        return true;
    }
    
    template<typename T>
    bool loadBinary(const std::string& filename, std::vector<T>& data) {
        std::string filepath = dataDir + PATH_SEPARATOR + filename;
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) return false;
        
        size_t count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        
        data.clear();
        for (size_t i = 0; i < count; i++) {
            T item;
            item.deserialize(file);
            data.push_back(item);
        }
        
        file.close();
        return true;
    }
    
    template<typename T>
    bool saveText(const std::string& filename, const std::vector<T>& data) {
        std::string filepath = dataDir + PATH_SEPARATOR + filename;
        std::ofstream file(filepath);
        if (!file.is_open()) return false;
        
        for (const auto& item : data) {
            item.writeTXT(file);
        }
        
        file.close();
        return true;
    }
    
    template<typename T>
    bool loadText(const std::string& filename, std::vector<T>& data) {
        std::string filepath = dataDir + PATH_SEPARATOR + filename;
        std::ifstream file(filepath);
        if (!file.is_open()) return false;
        
        data.clear();
        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (!line.empty()) {
                file.seekg(-line.length() - 1, std::ios::cur);
                T item = T::readTXT(file);
                data.push_back(item);
            }
        }
        
        file.close();
        return true;
    }
    
    template<typename T>
    bool saveBinarySingle(const std::string& filename, const T& data) {
        std::string filepath = dataDir + PATH_SEPARATOR + filename;
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) return false;
        
        data.serialize(file);
        
        file.close();
        return true;
    }
    
    template<typename T>
    bool loadBinarySingle(const std::string& filename, T& data) {
        std::string filepath = dataDir + PATH_SEPARATOR + filename;
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) return false;
        
        data.deserialize(file);
        
        file.close();
        return true;
    }
    
    template<typename T>
    bool saveTextSingle(const std::string& filename, const T& data) {
        std::string filepath = dataDir + PATH_SEPARATOR + filename;
        std::ofstream file(filepath);
        if (!file.is_open()) return false;
        
        data.writeTXT(file);
        
        file.close();
        return true;
    }
    
    template<typename T>
    bool loadTextSingle(const std::string& filename, T& data) {
        std::string filepath = dataDir + PATH_SEPARATOR + filename;
        std::ifstream file(filepath);
        if (!file.is_open()) return false;
        
        data = T::readTXT(file);
        
        file.close();
        return true;
    }
    
    bool backupFile(const std::string& filename) {
        std::string srcPath = dataDir + PATH_SEPARATOR + filename;
        if (!fileExists(srcPath)) return false;
        
        std::string timestamp = getCurrentDate() + "_" + getCurrentTime();
        timestamp.erase(std::remove(timestamp.begin(), timestamp.end(), ':'), timestamp.end());
        timestamp.erase(std::remove(timestamp.begin(), timestamp.end(), '-'), timestamp.end());
        
        std::string destPath = BACKUP_DIR + PATH_SEPARATOR + filename.substr(0, filename.find_last_of('.')) 
                              + "_" + timestamp + filename.substr(filename.find_last_of('.'));
        
        return copyFile(srcPath, destPath);
    }
    
    bool restoreFile(const std::string& backupFilename, const std::string& targetFilename) {
        std::string srcPath = BACKUP_DIR + PATH_SEPARATOR + backupFilename;
        std::string destPath = dataDir + PATH_SEPARATOR + targetFilename;
        
        if (!fileExists(srcPath)) return false;
        
        return copyFile(srcPath, destPath);
    }
    
    std::vector<std::string> listBackups() {
        return listDirectory(BACKUP_DIR);
    }
};

// ============================================
// EMPLOYEE MANAGER
// ============================================
class EmployeeManager {
private:
    std::vector<Employee> employees;
    FileManager& fileManager;
    Logger& logger;
    
public:
    EmployeeManager(FileManager& fm, Logger& log) : fileManager(fm), logger(log) {}
    
    bool load() {
        if (fileManager.loadBinary<Employee>("employees.dat", employees)) {
            printInfo("Employee database loaded from .dat file.");
            return true;
        }
        
        printWarning("employees.dat could not be loaded.");
        printInfo("Attempting recovery from employees.txt...");
        
        if (fileManager.loadText<Employee>("employees.txt", employees)) {
            printSuccess("Employee database recovered from .txt file.");
            save();
            return true;
        }
        
        printInfo("Starting with empty employee database.");
        return true;
    }
    
    bool save() {
        bool datSuccess = fileManager.saveBinary<Employee>("employees.dat", employees);
        bool txtSuccess = fileManager.saveText<Employee>("employees.txt", employees);
        
        return datSuccess && txtSuccess;
    }
    
    bool addEmployee(const Employee& emp) {
        if (employeeExists(emp.employeeID)) {
            printError("Employee ID already exists.");
            return false;
        }
        
        employees.push_back(emp);
        if (save()) {
            logger.logEmployeeAdded(emp.employeeID);
            printSuccess("Employee added successfully.");
            return true;
        }
        
        printError("Failed to save employee.");
        employees.pop_back();
        return false;
    }
    
    bool employeeExists(const std::string& empID) {
        for (const auto& emp : employees) {
            if (emp.employeeID == empID) return true;
        }
        return false;
    }
    
    Employee* findEmployee(const std::string& empID) {
        for (auto& emp : employees) {
            if (emp.employeeID == empID) return &emp;
        }
        return nullptr;
    }
    
    std::vector<Employee> searchByName(const std::string& name) {
        std::vector<Employee> results;
        std::string searchLower = toLowerCase(name);
        
        for (const auto& emp : employees) {
            if (toLowerCase(emp.name).find(searchLower) != std::string::npos) {
                results.push_back(emp);
            }
        }
        return results;
    }
    
    std::vector<Employee> searchByFirstLetter(char letter) {
        std::vector<Employee> results;
        char searchChar = std::tolower(letter);
        
        for (const auto& emp : employees) {
            if (std::tolower(emp.name[0]) == searchChar) {
                results.push_back(emp);
            }
        }
        return results;
    }
    
    std::vector<Employee> searchByDepartment(const std::string& dept) {
        std::vector<Employee> results;
        std::string searchLower = toLowerCase(dept);
        
        for (const auto& emp : employees) {
            if (toLowerCase(emp.department) == searchLower) {
                results.push_back(emp);
            }
        }
        return results;
    }
    
    std::vector<Employee> searchByPosition(const std::string& position) {
        std::vector<Employee> results;
        std::string searchLower = toLowerCase(position);
        
        for (const auto& emp : employees) {
            if (toLowerCase(emp.position) == searchLower) {
                results.push_back(emp);
            }
        }
        return results;
    }
    
    std::vector<Employee> searchByStatus(const std::string& status) {
        std::vector<Employee> results;
        
        for (const auto& emp : employees) {
            if (emp.status == status) {
                results.push_back(emp);
            }
        }
        return results;
    }
    
    bool updateEmployee(const std::string& empID, const Employee& updated) {
        Employee* emp = findEmployee(empID);
        if (!emp) {
            printError("Employee not found.");
            return false;
        }
        
        *emp = updated;
        emp->employeeID = empID;
        
        if (save()) {
            logger.logEmployeeUpdated(empID);
            printSuccess("Employee updated successfully.");
            return true;
        }
        
        printError("Failed to update employee.");
        return false;
    }
    
    bool deactivateEmployee(const std::string& empID) {
        Employee* emp = findEmployee(empID);
        if (!emp) {
            printError("Employee not found.");
            return false;
        }
        
        emp->status = "Inactive";
        
        if (save()) {
            logger.logEmployeeDeactivated(empID);
            printSuccess("Employee deactivated successfully.");
            return true;
        }
        
        printError("Failed to deactivate employee.");
        return false;
    }
    
    bool activateEmployee(const std::string& empID) {
        Employee* emp = findEmployee(empID);
        if (!emp) {
            printError("Employee not found.");
            return false;
        }
        
        emp->status = "Active";
        
        if (save()) {
            printSuccess("Employee activated successfully.");
            return true;
        }
        
        printError("Failed to activate employee.");
        return false;
    }
    
    void viewAllEmployees() {
        printHeader("ALL EMPLOYEES");
        
        if (employees.empty()) {
            printInfo("No employees found.");
            pauseScreen();
            return;
        }
        
        std::cout << std::left << std::setw(12) << "ID" 
                  << std::setw(25) << "Name"
                  << std::setw(15) << "Department"
                  << std::setw(15) << "Position"
                  << std::setw(10) << "Status" << std::endl;
        printLine();
        
        for (const auto& emp : employees) {
            std::cout << std::left << std::setw(12) << emp.employeeID
                      << std::setw(25) << emp.name
                      << std::setw(15) << emp.department
                      << std::setw(15) << emp.position
                      << std::setw(10) << emp.status << std::endl;
        }
        
        pauseScreen();
    }
    
    void viewEmployeeProfile(const std::string& empID) {
        Employee* emp = findEmployee(empID);
        if (!emp) {
            printError("Employee not found.");
            pauseScreen();
            return;
        }
        
        printHeader("EMPLOYEE PROFILE");
        
        std::cout << "Employee ID : " << emp->employeeID << std::endl;
        std::cout << "Name        : " << emp->name << std::endl;
        std::cout << "Gender      : " << emp->gender << std::endl;
        std::cout << "Age         : " << emp->age << std::endl;
        std::cout << "Phone       : " << emp->phone << std::endl;
        std::cout << "Email       : " << emp->email << std::endl;
        std::cout << "Address     : " << emp->address << std::endl;
        std::cout << "Department  : " << emp->department << std::endl;
        std::cout << "Position    : " << emp->position << std::endl;
        std::cout << "Joining Date: " << emp->joiningDate << std::endl;
        std::cout << "Status      : " << emp->status << std::endl;
        
        pauseScreen();
    }
    
    void sortEmployees(int criteria) {
        switch (criteria) {
            case 1: // By ID
                std::sort(employees.begin(), employees.end(),
                    [](const Employee& a, const Employee& b) { return a.employeeID < b.employeeID; });
                break;
            case 2: // By Name
                std::sort(employees.begin(), employees.end(),
                    [](const Employee& a, const Employee& b) { return a.name < b.name; });
                break;
            case 3: // By Department
                std::sort(employees.begin(), employees.end(),
                    [](const Employee& a, const Employee& b) { return a.department < b.department; });
                break;
            case 4: // By Joining Date
                std::sort(employees.begin(), employees.end(),
                    [](const Employee& a, const Employee& b) { return a.joiningDate < b.joiningDate; });
                break;
        }
        printSuccess("Employees sorted.");
    }
    
    std::vector<Employee> getActiveEmployees() {
        std::vector<Employee> active;
        for (const auto& emp : employees) {
            if (emp.status == "Active") {
                active.push_back(emp);
            }
        }
        return active;
    }
    
    int getTotalCount() { return employees.size(); }
    
    int getActiveCount() {
        int count = 0;
        for (const auto& emp : employees) {
            if (emp.status == "Active") count++;
        }
        return count;
    }
    
    int getInactiveCount() {
        int count = 0;
        for (const auto& emp : employees) {
            if (emp.status == "Inactive") count++;
        }
        return count;
    }
};

// ============================================
// ATTENDANCE MANAGER
// ============================================
class AttendanceManager {
private:
    std::vector<Attendance> attendance;
    FileManager& fileManager;
    Logger& logger;
    EmployeeManager& empManager;
    
    std::string generateAttendanceID() {
        int id = attendance.size() + 1;
        return "ATT" + std::string(4 - std::to_string(id).length(), '0') + std::to_string(id);
    }
    
public:
    AttendanceManager(FileManager& fm, Logger& log, EmployeeManager& em)
        : fileManager(fm), logger(log), empManager(em) {}
    
    bool load() {
        if (fileManager.loadBinary<Attendance>("attendance.dat", attendance)) {
            printInfo("Attendance database loaded from .dat file.");
            return true;
        }
        
        printWarning("attendance.dat could not be loaded.");
        printInfo("Attempting recovery from attendance.txt...");
        
        if (fileManager.loadText<Attendance>("attendance.txt", attendance)) {
            printSuccess("Attendance database recovered from .txt file.");
            save();
            return true;
        }
        
        printInfo("Starting with empty attendance database.");
        return true;
    }
    
    bool save() {
        bool datSuccess = fileManager.saveBinary<Attendance>("attendance.dat", attendance);
        bool txtSuccess = fileManager.saveText<Attendance>("attendance.txt", attendance);
        
        return datSuccess && txtSuccess;
    }
    
    bool attendanceExists(const std::string& empID, const std::string& date) {
        for (const auto& att : attendance) {
            if (att.employeeID == empID && att.date == date) {
                return true;
            }
        }
        return false;
    }
    
    Attendance* findAttendance(const std::string& empID, const std::string& date) {
        for (auto& att : attendance) {
            if (att.employeeID == empID && att.date == date) {
                return &att;
            }
        }
        return nullptr;
    }
    
    bool markAttendance(const Attendance& att, const Settings& settings) {
        if (!empManager.employeeExists(att.employeeID)) {
            printError("Employee does not exist.");
            return false;
        }
        
        if (attendanceExists(att.employeeID, att.date)) {
            printError("Attendance already exists for this employee on this date.");
            return false;
        }
        
        Attendance newAtt = att;
        newAtt.attendanceID = generateAttendanceID();
        
        // Calculate late minutes if check-in time is provided
        if (!newAtt.checkIn.empty() && (newAtt.status == "Present" || newAtt.status == "Late")) {
            int checkInMinutes = timeToMinutes(newAtt.checkIn);
            int officeStartMinutes = timeToMinutes(settings.officeStartTime);
            
            if (checkInMinutes > officeStartMinutes) {
                newAtt.lateMinutes = checkInMinutes - officeStartMinutes;
                if (newAtt.lateMinutes > 0) {
                    newAtt.status = "Late";
                }
            }
        }
        
        attendance.push_back(newAtt);
        
        if (save()) {
            logger.logAttendanceMarked(newAtt.employeeID, newAtt.date, newAtt.status);
            printSuccess("Attendance marked successfully.");
            return true;
        }
        
        printError("Failed to save attendance.");
        attendance.pop_back();
        return false;
    }
    
    bool checkIn(const std::string& empID, const Settings& settings) {
        if (!empManager.employeeExists(empID)) {
            printError("Employee does not exist.");
            return false;
        }
        
        std::string today = getCurrentDate();
        
        if (attendanceExists(empID, today)) {
            printError("Attendance already exists for this employee today.");
            return false;
        }
        
        Attendance att;
        att.attendanceID = generateAttendanceID();
        att.employeeID = empID;
        att.date = today;
        att.checkIn = getCurrentTime();
        att.checkOut = "";
        att.status = "Present";
        att.lateMinutes = 0;
        att.remarks = "";
        
        // Calculate late minutes
        int checkInMinutes = timeToMinutes(att.checkIn);
        int officeStartMinutes = timeToMinutes(settings.officeStartTime);
        
        if (checkInMinutes > officeStartMinutes) {
            att.lateMinutes = checkInMinutes - officeStartMinutes;
            if (att.lateMinutes > 0) {
                att.status = "Late";
            }
        }
        
        attendance.push_back(att);
        
        if (save()) {
            logger.logCheckIn(empID, att.checkIn);
            printSuccess("Check-in successful!");
            std::cout << "Date: " << att.date << std::endl;
            std::cout << "Time: " << att.checkIn << std::endl;
            std::cout << "Status: " << att.status;
            if (att.status == "Late") {
                std::cout << " (Late: " << att.lateMinutes << " minutes)";
            }
            std::cout << std::endl;
            return true;
        }
        
        printError("Failed to save check-in.");
        attendance.pop_back();
        return false;
    }
    
    bool checkOut(const std::string& empID) {
        std::string today = getCurrentDate();
        Attendance* att = findAttendance(empID, today);
        
        if (!att) {
            printError("No check-in found for this employee today.");
            return false;
        }
        
        if (!att->checkOut.empty()) {
            printError("Employee has already checked out today.");
            return false;
        }
        
        att->checkOut = getCurrentTime();
        
        if (save()) {
            logger.logCheckOut(empID, att->checkOut);
            printSuccess("Check-out successful!");
            std::cout << "Check-out time: " << att->checkOut << std::endl;
            return true;
        }
        
        printError("Failed to save check-out.");
        att->checkOut = "";
        return false;
    }
    
    bool editAttendance(const std::string& empID, const std::string& date, const Attendance& updated) {
        Attendance* att = findAttendance(empID, date);
        if (!att) {
            printError("Attendance record not found.");
            return false;
        }
        
        att->checkIn = updated.checkIn;
        att->checkOut = updated.checkOut;
        att->status = updated.status;
        att->lateMinutes = updated.lateMinutes;
        att->remarks = updated.remarks;
        
        if (save()) {
            printSuccess("Attendance updated successfully.");
            return true;
        }
        
        printError("Failed to update attendance.");
        return false;
    }
    
    bool deleteAttendance(const std::string& empID, const std::string& date) {
        auto it = std::remove_if(attendance.begin(), attendance.end(),
            [&](const Attendance& att) { return att.employeeID == empID && att.date == date; });
        
        if (it == attendance.end()) {
            printError("Attendance record not found.");
            return false;
        }
        
        attendance.erase(it, attendance.end());
        
        if (save()) {
            printSuccess("Attendance deleted successfully.");
            return true;
        }
        
        printError("Failed to delete attendance.");
        return false;
    }
    
    void viewTodayAttendance(const Settings& settings, const std::vector<Holiday>& holidays) {
        printHeader("TODAY'S ATTENDANCE");
        
        std::string today = getCurrentDate();
        
        // Check if today is a holiday
        bool isHoliday = false;
        std::string holidayName;
        for (const auto& h : holidays) {
            if (h.date == today) {
                isHoliday = true;
                holidayName = h.name;
                break;
            }
        }
        
        if (isHoliday) {
            printInfo("Today is a holiday: " + holidayName);
            pauseScreen();
            return;
        }
        
        if (!settings.isWorkingDay(today)) {
            printInfo("Today is not a working day.");
            pauseScreen();
            return;
        }
        
        std::cout << "Date: " << today << std::endl << std::endl;
        
        std::cout << std::left << std::setw(12) << "ID" 
                  << std::setw(25) << "Name"
                  << std::setw(12) << "Check In"
                  << std::setw(12) << "Check Out"
                  << std::setw(10) << "Status" << std::endl;
        printLine();
        
        std::vector<Employee> activeEmployees = empManager.getActiveEmployees();
        
        for (const auto& emp : activeEmployees) {
            Attendance* att = findAttendance(emp.employeeID, today);
            
            std::cout << std::left << std::setw(12) << emp.employeeID
                      << std::setw(25) << emp.name;
            
            if (att) {
                std::cout << std::setw(12) << (att->checkIn.empty() ? "--" : att->checkIn)
                          << std::setw(12) << (att->checkOut.empty() ? "--" : att->checkOut)
                          << std::setw(10) << att->status;
            } else {
                std::cout << std::setw(12) << "--"
                          << std::setw(12) << "--"
                          << std::setw(10) << "Absent";
            }
            std::cout << std::endl;
        }
        
        pauseScreen();
    }
    
    void viewAttendanceHistory(const std::string& empID) {
        printHeader("ATTENDANCE HISTORY - " + empID);
        
        std::cout << std::left << std::setw(12) << "Date"
                  << std::setw(12) << "Check In"
                  << std::setw(12) << "Check Out"
                  << std::setw(10) << "Status"
                  << std::setw(10) << "Late" << std::endl;
        printLine();
        
        for (const auto& att : attendance) {
            if (att.employeeID == empID) {
                std::cout << std::left << std::setw(12) << att.date
                          << std::setw(12) << (att.checkIn.empty() ? "--" : att.checkIn)
                          << std::setw(12) << (att.checkOut.empty() ? "--" : att.checkOut)
                          << std::setw(10) << att.status
                          << std::setw(10) << (att.lateMinutes > 0 ? std::to_string(att.lateMinutes) + " min" : "")
                          << std::endl;
            }
        }
        
        pauseScreen();
    }
    
    std::vector<Attendance> getAttendanceByDate(const std::string& date) {
        std::vector<Attendance> result;
        for (const auto& att : attendance) {
            if (att.date == date) {
                result.push_back(att);
            }
        }
        return result;
    }
    
    std::vector<Attendance> getAttendanceByEmployee(const std::string& empID) {
        std::vector<Attendance> result;
        for (const auto& att : attendance) {
            if (att.employeeID == empID) {
                result.push_back(att);
            }
        }
        return result;
    }
    
    std::vector<Attendance> getAttendanceByDateRange(const std::string& startDate, const std::string& endDate) {
        std::vector<Attendance> result;
        for (const auto& att : attendance) {
            if (att.date >= startDate && att.date <= endDate) {
                result.push_back(att);
            }
        }
        return result;
    }
    
    std::vector<Attendance> getAttendanceByMonth(const std::string& yearMonth) {
        std::vector<Attendance> result;
        for (const auto& att : attendance) {
            if (att.date.substr(0, 7) == yearMonth) {
                result.push_back(att);
            }
        }
        return result;
    }
    
    std::vector<Attendance> getAttendanceByDepartment(const std::string& department, 
                                                       const std::string& date,
                                                       EmployeeManager& empManager) {
        std::vector<Attendance> result;
        std::vector<Employee> deptEmployees = empManager.searchByDepartment(department);
        
        for (const auto& emp : deptEmployees) {
            for (const auto& att : attendance) {
                if (att.employeeID == emp.employeeID && att.date == date) {
                    result.push_back(att);
                }
            }
        }
        return result;
    }
    
    std::vector<Attendance> getAbsentEmployees(const std::string& date, 
                                              EmployeeManager& empManager,
                                              const Settings& settings,
                                              const std::vector<Holiday>& holidays) {
        std::vector<Attendance> result;
        
        // Check if date is a holiday
        for (const auto& h : holidays) {
            if (h.date == date) return result;
        }
        
        if (!settings.isWorkingDay(date)) return result;
        
        std::vector<Employee> activeEmployees = empManager.getActiveEmployees();
        
        for (const auto& emp : activeEmployees) {
            bool found = false;
            for (const auto& att : attendance) {
                if (att.employeeID == emp.employeeID && att.date == date) {
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                Attendance att;
                att.employeeID = emp.employeeID;
                att.date = date;
                att.status = "Absent";
                result.push_back(att);
            }
        }
        
        return result;
    }
    
    std::vector<Attendance> getLateEmployees(const std::string& date) {
        std::vector<Attendance> result;
        for (const auto& att : attendance) {
            if (att.date == date && att.status == "Late") {
                result.push_back(att);
            }
        }
        return result;
    }
    
    int calculateAttendancePercentage(const std::string& empID, 
                                       const std::string& startDate,
                                       const std::string& endDate,
                                       const Settings& settings,
                                       const std::vector<Holiday>& holidays) {
        int present = 0, late = 0, absent = 0, leave = 0;
        int workingDays = 0;
        
        for (const auto& att : attendance) {
            if (att.employeeID == empID && att.date >= startDate && att.date <= endDate) {
                if (att.status == "Present") present++;
                else if (att.status == "Late") late++;
                else if (att.status == "Absent") absent++;
                else if (att.status == "Leave") leave++;
            }
        }
        
        workingDays = present + late + absent;
        
        if (workingDays == 0) return 0;
        
        return ((present + late) * 100) / workingDays;
    }
    
    int getTodayPresentCount(const std::string& date) {
        int count = 0;
        for (const auto& att : attendance) {
            if (att.date == date && (att.status == "Present" || att.status == "Late")) {
                count++;
            }
        }
        return count;
    }
    
    int getTodayLateCount(const std::string& date) {
        int count = 0;
        for (const auto& att : attendance) {
            if (att.date == date && att.status == "Late") {
                count++;
            }
        }
        return count;
    }
    
    int getTodayAbsentCount(const std::string& date, EmployeeManager& empManager,
                           const Settings& settings, const std::vector<Holiday>& holidays) {
        return getAbsentEmployees(date, empManager, settings, holidays).size();
    }
    
    int getTodayLeaveCount(const std::string& date) {
        int count = 0;
        for (const auto& att : attendance) {
            if (att.date == date && att.status == "Leave") {
                count++;
            }
        }
        return count;
    }
    
    int getTodayHolidayCount(const std::string& date, const std::vector<Holiday>& holidays) {
        for (const auto& h : holidays) {
            if (h.date == date) return 1;
        }
        return 0;
    }
};

// ============================================
// AUTH MANAGER
// ============================================
class AuthManager {
private:
    std::vector<Admin> admins;
    FileManager& fileManager;
    Logger& logger;
    Admin* currentAdmin = nullptr;
    
public:
    AuthManager(FileManager& fm, Logger& log) : fileManager(fm), logger(log) {}
    
    bool load() {
        if (fileManager.loadBinary<Admin>("admins.dat", admins)) {
            printInfo("Admin database loaded from .dat file.");
            return true;
        }
        
        printWarning("admins.dat could not be loaded.");
        printInfo("Attempting recovery from admins.txt...");
        
        if (fileManager.loadText<Admin>("admins.txt", admins)) {
            printSuccess("Admin database recovered from .txt file.");
            save();
            return true;
        }
        
        printInfo("Starting with empty admin database.");
        createDefaultAdmin();
        return true;
    }
    
    bool save() {
        bool datSuccess = fileManager.saveBinary<Admin>("admins.dat", admins);
        bool txtSuccess = fileManager.saveText<Admin>("admins.txt", admins);
        
        return datSuccess && txtSuccess;
    }
    
    void createDefaultAdmin() {
        if (admins.empty()) {
            Admin admin;
            admin.username = "admin";
            admin.passwordHash = Admin::hashPassword("admin123");
            admin.role = "Administrator";
            admin.status = "Active";
            admins.push_back(admin);
            save();
            printInfo("Default admin account created (admin/admin123)");
        }
    }
    
    bool login() {
        printHeader("EMPLOYEE ATTENDANCE MANAGEMENT");
        
        int attempts = 0;
        
        while (attempts < MAX_LOGIN_ATTEMPTS) {
            std::cout << "\nUsername: ";
            std::string username;
            std::getline(std::cin, username);
            
            std::cout << "Password: ";
            std::string password;
            // Simple password hiding (cross-platform)
            char c;
            while ((c = getPasswordChar()) != '\r' && c != '\n') {
                if (c == '\b' || c == 127) {
                    if (!password.empty()) {
                        password.pop_back();
                        std::cout << "\b \b";
                    }
                } else {
                    password += c;
                    std::cout << '*';
                }
            }
            std::cout << std::endl;
            
            for (auto& admin : admins) {
                if (admin.username == username && 
                    admin.passwordHash == Admin::hashPassword(password) &&
                    admin.status == "Active") {
                    
                    currentAdmin = &admin;
                    logger.logLogin(username);
                    
                    // Force password change for default admin
                    if (username == "admin" && password == "admin123") {
                        printWarning("Default password detected. Please change your password.");
                        changePassword();
                    }
                    
                    return true;
                }
            }
            
            attempts++;
            printError("Invalid username or password. Attempts remaining: " + 
                      std::to_string(MAX_LOGIN_ATTEMPTS - attempts));
        }
        
        return false;
    }
    
    void logout() {
        if (currentAdmin) {
            logger.logLogout(currentAdmin->username);
            currentAdmin = nullptr;
        }
    }
    
    bool changePassword() {
        if (!currentAdmin) return false;
        
        std::cout << "Current password: ";
        std::string currentPassword;
        char c;
        while ((c = ::getch()) != '\r' && c != '\n') {
            if (c == '\b' || c == 127) {
                if (!currentPassword.empty()) {
                    currentPassword.pop_back();
                    std::cout << "\b \b";
                }
            } else {
                currentPassword += c;
                std::cout << '*';
            }
        }
        std::cout << std::endl;
        
        if (Admin::hashPassword(currentPassword) != currentAdmin->passwordHash) {
            printError("Current password is incorrect.");
            return false;
        }
        
        std::string newPassword, confirmPassword;
        
        do {
            std::cout << "New password: ";
            newPassword.clear();
            while ((c = getPasswordChar()) != '\r' && c != '\n') {
                if (c == '\b' || c == 127) {
                    if (!newPassword.empty()) {
                        newPassword.pop_back();
                        std::cout << "\b \b";
                    }
                } else {
                    newPassword += c;
                    std::cout << '*';
                }
            }
            std::cout << std::endl;
            
            if (newPassword.length() < PASSWORD_MIN_LENGTH) {
                printError("Password must be at least " + std::to_string(PASSWORD_MIN_LENGTH) + " characters.");
                continue;
            }
            
            std::cout << "Confirm new password: ";
            confirmPassword.clear();
            while ((c = getPasswordChar()) != '\r' && c != '\n') {
                if (c == '\b' || c == 127) {
                    if (!confirmPassword.empty()) {
                        confirmPassword.pop_back();
                        std::cout << "\b \b";
                    }
                } else {
                    confirmPassword += c;
                    std::cout << '*';
                }
            }
            std::cout << std::endl;
            
            if (newPassword != confirmPassword) {
                printError("Passwords do not match.");
            } else {
                break;
            }
        } while (true);
        
        currentAdmin->passwordHash = Admin::hashPassword(newPassword);
        
        if (save()) {
            printSuccess("Password changed successfully.");
            return true;
        }
        
        printError("Failed to change password.");
        return false;
    }
    
    bool addAdmin(const std::string& username, const std::string& password, 
                  const std::string& role) {
        for (const auto& admin : admins) {
            if (admin.username == username) {
                printError("Username already exists.");
                return false;
            }
        }
        
        if (password.length() < PASSWORD_MIN_LENGTH) {
            printError("Password must be at least " + std::to_string(PASSWORD_MIN_LENGTH) + " characters.");
            return false;
        }
        
        Admin admin;
        admin.username = username;
        admin.passwordHash = Admin::hashPassword(password);
        admin.role = role;
        admin.status = "Active";
        
        admins.push_back(admin);
        
        if (save()) {
            printSuccess("Admin added successfully.");
            return true;
        }
        
        printError("Failed to add admin.");
        admins.pop_back();
        return false;
    }
    
    bool deleteAdmin(const std::string& username) {
        if (admins.size() <= 1) {
            printError("Cannot delete the last administrator.");
            return false;
        }
        
        auto it = std::remove_if(admins.begin(), admins.end(),
            [&](const Admin& admin) { return admin.username == username; });
        
        if (it == admins.end()) {
            printError("Admin not found.");
            return false;
        }
        
        admins.erase(it, admins.end());
        
        if (save()) {
            printSuccess("Admin deleted successfully.");
            return true;
        }
        
        printError("Failed to delete admin.");
        return false;
    }
    
    bool changeAdminRole(const std::string& username, const std::string& newRole) {
        for (auto& admin : admins) {
            if (admin.username == username) {
                admin.role = newRole;
                if (save()) {
                    printSuccess("Role changed successfully.");
                    return true;
                }
                printError("Failed to change role.");
                return false;
            }
        }
        
        printError("Admin not found.");
        return false;
    }
    
    void viewAllAdmins() {
        printHeader("ALL ADMINS");
        
        std::cout << std::left << std::setw(20) << "Username"
                  << std::setw(15) << "Role"
                  << std::setw(10) << "Status" << std::endl;
        printLine();
        
        for (const auto& admin : admins) {
            std::cout << std::left << std::setw(20) << admin.username
                      << std::setw(15) << admin.role
                      << std::setw(10) << admin.status << std::endl;
        }
        
        pauseScreen();
    }
    
    bool isAdmin() const {
        return currentAdmin && currentAdmin->role == "Administrator";
    }
    
    bool isManager() const {
        return currentAdmin && currentAdmin->role == "Manager";
    }
    
    std::string getCurrentUsername() const {
        return currentAdmin ? currentAdmin->username : "";
    }
    
    std::string getCurrentRole() const {
        return currentAdmin ? currentAdmin->role : "";
    }
};

// ============================================
// SETTINGS MANAGER
// ============================================
class SettingsManager {
private:
    Settings settings;
    FileManager& fileManager;
    Logger& logger;
    
public:
    SettingsManager(FileManager& fm, Logger& log) : fileManager(fm), logger(log) {}
    
    bool load() {
        if (fileManager.loadBinarySingle<Settings>("settings.dat", settings)) {
            printInfo("Settings loaded from .dat file.");
            return true;
        }
        
        printWarning("settings.dat could not be loaded.");
        printInfo("Attempting recovery from settings.txt...");
        
        if (fileManager.loadTextSingle<Settings>("settings.txt", settings)) {
            printSuccess("Settings recovered from .txt file.");
            save();
            return true;
        }
        
        printInfo("Using default settings.");
        return true;
    }
    
    bool save() {
        bool datSuccess = fileManager.saveBinarySingle<Settings>("settings.dat", settings);
        bool txtSuccess = fileManager.saveTextSingle<Settings>("settings.txt", settings);
        
        return datSuccess && txtSuccess;
    }
    
    Settings& getSettings() { return settings; }
    
    bool updateCompanyName(const std::string& name) {
        settings.companyName = name;
        if (save()) {
            logger.logSettingsChanged();
            printSuccess("Company name updated.");
            return true;
        }
        return false;
    }
    
    bool updateOfficeStartTime(const std::string& time) {
        if (!isValidTime(time)) {
            printError("Invalid time format.");
            return false;
        }
        settings.officeStartTime = time;
        if (save()) {
            logger.logSettingsChanged();
            printSuccess("Office start time updated.");
            return true;
        }
        return false;
    }
    
    bool updateOfficeEndTime(const std::string& time) {
        if (!isValidTime(time)) {
            printError("Invalid time format.");
            return false;
        }
        settings.officeEndTime = time;
        if (save()) {
            logger.logSettingsChanged();
            printSuccess("Office end time updated.");
            return true;
        }
        return false;
    }
    
    bool updateLateThreshold(int minutes) {
        settings.lateThreshold = minutes;
        if (save()) {
            logger.logSettingsChanged();
            printSuccess("Late threshold updated.");
            return true;
        }
        return false;
    }
    
    bool updateWorkingDays(const std::string& days) {
        settings.workingDays = days;
        if (save()) {
            logger.logSettingsChanged();
            printSuccess("Working days updated.");
            return true;
        }
        return false;
    }
    
    void viewSettings() {
        printHeader("SYSTEM SETTINGS");
        
        std::cout << "Company Name      : " << settings.companyName << std::endl;
        std::cout << "Office Start Time : " << settings.officeStartTime << std::endl;
        std::cout << "Office End Time   : " << settings.officeEndTime << std::endl;
        std::cout << "Late Threshold    : " << settings.lateThreshold << " minutes" << std::endl;
        std::cout << "Working Days      : " << settings.workingDays << std::endl;
        
        pauseScreen();
    }
};

// ============================================
// HOLIDAY MANAGER
// ============================================
class HolidayManager {
private:
    std::vector<Holiday> holidays;
    FileManager& fileManager;
    Logger& logger;
    
public:
    HolidayManager(FileManager& fm, Logger& log) : fileManager(fm), logger(log) {}
    
    bool load() {
        if (fileManager.loadBinary<Holiday>("holidays.dat", holidays)) {
            printInfo("Holidays loaded from .dat file.");
            return true;
        }
        
        printWarning("holidays.dat could not be loaded.");
        printInfo("Attempting recovery from holidays.txt...");
        
        if (fileManager.loadText<Holiday>("holidays.txt", holidays)) {
            printSuccess("Holidays recovered from .txt file.");
            save();
            return true;
        }
        
        printInfo("Starting with empty holidays list.");
        return true;
    }
    
    bool save() {
        bool datSuccess = fileManager.saveBinary<Holiday>("holidays.dat", holidays);
        bool txtSuccess = fileManager.saveText<Holiday>("holidays.txt", holidays);
        
        return datSuccess && txtSuccess;
    }
    
    bool addHoliday(const std::string& date, const std::string& name) {
        if (!isValidDate(date)) {
            printError("Invalid date format.");
            return false;
        }
        
        for (const auto& h : holidays) {
            if (h.date == date) {
                printError("Holiday already exists for this date.");
                return false;
            }
        }
        
        Holiday holiday;
        holiday.date = date;
        holiday.name = name;
        
        holidays.push_back(holiday);
        
        if (save()) {
            printSuccess("Holiday added successfully.");
            return true;
        }
        
        printError("Failed to add holiday.");
        holidays.pop_back();
        return false;
    }
    
    bool deleteHoliday(const std::string& date) {
        auto it = std::remove_if(holidays.begin(), holidays.end(),
            [&](const Holiday& h) { return h.date == date; });
        
        if (it == holidays.end()) {
            printError("Holiday not found.");
            return false;
        }
        
        holidays.erase(it, holidays.end());
        
        if (save()) {
            printSuccess("Holiday deleted successfully.");
            return true;
        }
        
        printError("Failed to delete holiday.");
        return false;
    }
    
    void viewHolidays() {
        printHeader("HOLIDAYS");
        
        if (holidays.empty()) {
            printInfo("No holidays found.");
            pauseScreen();
            return;
        }
        
        std::cout << std::left << std::setw(15) << "Date"
                  << std::setw(30) << "Name" << std::endl;
        printLine();
        
        for (const auto& h : holidays) {
            std::cout << std::left << std::setw(15) << h.date
                      << std::setw(30) << h.name << std::endl;
        }
        
        pauseScreen();
    }
    
    bool isHoliday(const std::string& date) {
        for (const auto& h : holidays) {
            if (h.date == date) return true;
        }
        return false;
    }
    
    std::vector<Holiday> getHolidays() { return holidays; }
};

// ============================================
// REPORT MANAGER
// ============================================
class ReportManager {
private:
    FileManager& fileManager;
    Logger& logger;
    
public:
    ReportManager(FileManager& fm, Logger& log) : fileManager(fm), logger(log) {}
    
    void generateDailyReport(const std::string& date, AttendanceManager& attManager,
                             EmployeeManager& empManager, const Settings& settings,
                             const std::vector<Holiday>& holidays) {
        printHeader("DAILY REPORT - " + date);
        
        int totalEmployees = empManager.getActiveCount();
        int present = attManager.getTodayPresentCount(date);
        int late = attManager.getTodayLateCount(date);
        int absent = attManager.getTodayAbsentCount(date, empManager, settings, holidays);
        int leave = attManager.getTodayLeaveCount(date);
        int holiday = attManager.getTodayHolidayCount(date, holidays);
        
        std::cout << "Date: " << date << std::endl << std::endl;
        std::cout << "Total Employees : " << totalEmployees << std::endl;
        std::cout << "Present         : " << present << std::endl;
        std::cout << "Late            : " << late << std::endl;
        std::cout << "Absent          : " << absent << std::endl;
        std::cout << "Leave           : " << leave << std::endl;
        std::cout << "Holiday         : " << holiday << std::endl;
        
        pauseScreen();
    }
    
    void generateWeeklyReport(const std::string& startDate, AttendanceManager& attManager,
                             EmployeeManager& empManager, const Settings& settings,
                             const std::vector<Holiday>& holidays) {
        printHeader("WEEKLY REPORT");
        
        std::cout << std::left << std::setw(12) << "Date"
                  << std::setw(10) << "Present"
                  << std::setw(8) << "Late"
                  << std::setw(8) << "Absent"
                  << std::setw(8) << "Leave" << std::endl;
        printLine();
        
        // Calculate end date (7 days)
        std::tm tm = {};
        std::istringstream iss(startDate);
        iss >> std::get_time(&tm, "%Y-%m-%d");
        
        for (int i = 0; i < 7; i++) {
            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%d");
            std::string date = oss.str();
            
            int present = attManager.getTodayPresentCount(date);
            int late = attManager.getTodayLateCount(date);
            int absent = attManager.getTodayAbsentCount(date, empManager, settings, holidays);
            int leave = attManager.getTodayLeaveCount(date);
            
            std::cout << std::left << std::setw(12) << date
                      << std::setw(10) << present
                      << std::setw(8) << late
                      << std::setw(8) << absent
                      << std::setw(8) << leave << std::endl;
            
            tm.tm_mday++;
            std::mktime(&tm);
        }
        
        pauseScreen();
    }
    
    void generateMonthlyReport(const std::string& empID, const std::string& yearMonth,
                               AttendanceManager& attManager, EmployeeManager& empManager,
                               const Settings& settings, const std::vector<Holiday>& holidays) {
        Employee* emp = empManager.findEmployee(empID);
        if (!emp) {
            printError("Employee not found.");
            pauseScreen();
            return;
        }
        
        printHeader("MONTHLY ATTENDANCE REPORT");
        
        std::cout << "Employee: " << empID << " - " << emp->name << std::endl;
        std::cout << "Month   : " << yearMonth << std::endl << std::endl;
        
        std::vector<Attendance> empAttendance = attManager.getAttendanceByMonth(yearMonth);
        
        int present = 0, late = 0, absent = 0, leave = 0;
        
        for (const auto& att : empAttendance) {
            if (att.employeeID == empID) {
                if (att.status == "Present") present++;
                else if (att.status == "Late") late++;
                else if (att.status == "Absent") absent++;
                else if (att.status == "Leave") leave++;
            }
        }
        
        int workingDays = present + late + absent;
        int attendancePercentage = workingDays > 0 ? ((present + late) * 100) / workingDays : 0;
        
        std::cout << "Working Days : " << workingDays << std::endl;
        std::cout << "Present      : " << present << std::endl;
        std::cout << "Late         : " << late << std::endl;
        std::cout << "Absent       : " << absent << std::endl;
        std::cout << "Leave        : " << leave << std::endl;
        std::cout << std::endl;
        std::cout << "Attendance: " << attendancePercentage << "%" << std::endl;
        
        pauseScreen();
    }
    
    void generateEmployeeReport(const std::string& empID, AttendanceManager& attManager,
                                 EmployeeManager& empManager) {
        Employee* emp = empManager.findEmployee(empID);
        if (!emp) {
            printError("Employee not found.");
            pauseScreen();
            return;
        }
        
        printHeader("EMPLOYEE REPORT - " + empID);
        
        std::vector<Attendance> empAttendance = attManager.getAttendanceByEmployee(empID);
        
        std::cout << std::left << std::setw(12) << "Date"
                  << std::setw(12) << "Check In"
                  << std::setw(12) << "Check Out"
                  << std::setw(10) << "Status" << std::endl;
        printLine();
        
        for (const auto& att : empAttendance) {
            std::cout << std::left << std::setw(12) << att.date
                      << std::setw(12) << (att.checkIn.empty() ? "--" : att.checkIn)
                      << std::setw(12) << (att.checkOut.empty() ? "--" : att.checkOut)
                      << std::setw(10) << att.status << std::endl;
        }
        
        pauseScreen();
    }
    
    void generateDepartmentReport(const std::string& department, const std::string& date,
                                   AttendanceManager& attManager, EmployeeManager& empManager,
                                   const Settings& settings, const std::vector<Holiday>& holidays) {
        printHeader("DEPARTMENT REPORT - " + department);
        
        std::vector<Employee> deptEmployees = empManager.searchByDepartment(department);
        std::vector<Attendance> deptAttendance = attManager.getAttendanceByDepartment(department, date, empManager);
        
        int totalEmployees = deptEmployees.size();
        int present = 0, late = 0, absent = 0, leave = 0;
        
        for (const auto& att : deptAttendance) {
            if (att.status == "Present") present++;
            else if (att.status == "Late") { present++; late++; }
            else if (att.status == "Absent") absent++;
            else if (att.status == "Leave") leave++;
        }
        
        absent = totalEmployees - present - leave;
        
        int workingDays = present + absent;
        int avgAttendance = workingDays > 0 ? (present * 100) / workingDays : 0;
        
        std::cout << "Department: " << department << std::endl;
        std::cout << "Date: " << date << std::endl << std::endl;
        std::cout << "Total Employees : " << totalEmployees << std::endl;
        std::cout << "Present Today   : " << present << std::endl;
        std::cout << "Late Today      : " << late << std::endl;
        std::cout << "Absent Today    : " << absent << std::endl;
        std::cout << "On Leave        : " << leave << std::endl;
        std::cout << std::endl;
        std::cout << "Average Attendance: " << avgAttendance << "%" << std::endl;
        
        pauseScreen();
    }
    
    void generateAbsentReport(const std::string& date, AttendanceManager& attManager,
                              EmployeeManager& empManager, const Settings& settings,
                              const std::vector<Holiday>& holidays) {
        printHeader("ABSENT EMPLOYEES - " + date);
        
        std::vector<Attendance> absentEmployees = attManager.getAbsentEmployees(date, empManager, settings, holidays);
        
        if (absentEmployees.empty()) {
            printInfo("No absent employees found.");
            pauseScreen();
            return;
        }
        
        for (const auto& att : absentEmployees) {
            Employee* emp = empManager.findEmployee(att.employeeID);
            if (emp) {
                std::cout << att.employeeID << " - " << emp->name << std::endl;
            }
        }
        
        pauseScreen();
    }
    
    void generateLateReport(const std::string& date, AttendanceManager& attManager,
                            EmployeeManager& empManager) {
        printHeader("LATE EMPLOYEES - " + date);
        
        std::vector<Attendance> lateEmployees = attManager.getLateEmployees(date);
        
        if (lateEmployees.empty()) {
            printInfo("No late employees found.");
            pauseScreen();
            return;
        }
        
        std::cout << std::left << std::setw(12) << "ID"
                  << std::setw(20) << "Name"
                  << std::setw(12) << "Check In"
                  << std::setw(10) << "Late" << std::endl;
        printLine();
        
        for (const auto& att : lateEmployees) {
            Employee* emp = empManager.findEmployee(att.employeeID);
            if (emp) {
                std::cout << std::left << std::setw(12) << att.employeeID
                          << std::setw(20) << emp->name
                          << std::setw(12) << att.checkIn
                          << std::setw(10) << att.lateMinutes << " min" << std::endl;
            }
        }
        
        pauseScreen();
    }
    
    void generateAttendancePercentage(const std::string& empID, const std::string& startDate,
                                      const std::string& endDate, AttendanceManager& attManager,
                                      EmployeeManager& empManager, const Settings& settings,
                                      const std::vector<Holiday>& holidays) {
        Employee* emp = empManager.findEmployee(empID);
        if (!emp) {
            printError("Employee not found.");
            pauseScreen();
            return;
        }
        
        printHeader("ATTENDANCE PERCENTAGE - " + empID);
        
        int percentage = attManager.calculateAttendancePercentage(empID, startDate, endDate, settings, holidays);
        
        std::cout << "Employee: " << empID << " - " << emp->name << std::endl;
        std::cout << "Period: " << startDate << " to " << endDate << std::endl << std::endl;
        std::cout << "Attendance Rate: " << percentage << "%" << std::endl;
        
        pauseScreen();
    }
    
    bool exportReport(const std::string& filename, const std::string& content) {
        std::string filepath = REPORTS_DIR + PATH_SEPARATOR + filename;
        std::ofstream file(filepath);
        
        if (!file.is_open()) {
            printError("Failed to create report file.");
            return false;
        }
        
        file << content;
        file.close();
        
        printSuccess("Report exported to " + filepath);
        logger.logBackup("Report export: " + filename);
        return true;
    }
};

// ============================================
// STATISTICS MANAGER
// ============================================
class StatisticsManager {
private:
    EmployeeManager& empManager;
    AttendanceManager& attManager;
    Settings& settings;
    HolidayManager& holidayManager;
    
public:
    StatisticsManager(EmployeeManager& em, AttendanceManager& am, Settings& s, HolidayManager& hm)
        : empManager(em), attManager(am), settings(s), holidayManager(hm) {}
    
    void displayStatistics() {
        printHeader("STATISTICS");
        
        std::string today = getCurrentDate();
        std::vector<Holiday> holidays = holidayManager.getHolidays();
        
        int totalEmployees = empManager.getTotalCount();
        int activeEmployees = empManager.getActiveCount();
        int inactiveEmployees = empManager.getInactiveCount();
        
        int presentToday = attManager.getTodayPresentCount(today);
        int lateToday = attManager.getTodayLateCount(today);
        int absentToday = attManager.getTodayAbsentCount(today, empManager, settings, holidays);
        int onLeaveToday = attManager.getTodayLeaveCount(today);
        
        int totalActive = activeEmployees;
        int avgAttendance = totalActive > 0 ? ((presentToday + lateToday) * 100) / totalActive : 0;
        
        std::cout << "Total Employees    : " << totalEmployees << std::endl;
        std::cout << "Active Employees   : " << activeEmployees << std::endl;
        std::cout << "Inactive Employees : " << inactiveEmployees << std::endl;
        std::cout << std::endl;
        std::cout << "Present Today      : " << presentToday << std::endl;
        std::cout << "Late Today         : " << lateToday << std::endl;
        std::cout << "Absent Today       : " << absentToday << std::endl;
        std::cout << "On Leave           : " << onLeaveToday << std::endl;
        std::cout << std::endl;
        std::cout << "Average Attendance : " << avgAttendance << "%" << std::endl;
        
        pauseScreen();
    }
};

// ============================================
// BACKUP MANAGER
// ============================================
class BackupManager {
private:
    FileManager& fileManager;
    Logger& logger;
    
public:
    BackupManager(FileManager& fm, Logger& log) : fileManager(fm), logger(log) {}
    
    bool backupAll() {
        bool success = true;
        success &= fileManager.backupFile("employees.dat");
        success &= fileManager.backupFile("employees.txt");
        success &= fileManager.backupFile("attendance.dat");
        success &= fileManager.backupFile("attendance.txt");
        success &= fileManager.backupFile("admins.dat");
        success &= fileManager.backupFile("admins.txt");
        success &= fileManager.backupFile("settings.dat");
        success &= fileManager.backupFile("settings.txt");
        success &= fileManager.backupFile("holidays.dat");
        success &= fileManager.backupFile("holidays.txt");
        
        if (success) {
            printSuccess("All data backed up successfully.");
            logger.logBackup("All data");
        } else {
            printError("Some files failed to backup.");
        }
        
        return success;
    }
    
    bool backupEmployees() {
        bool success = fileManager.backupFile("employees.dat") && 
                       fileManager.backupFile("employees.txt");
        if (success) {
            printSuccess("Employee data backed up successfully.");
            logger.logBackup("Employees");
        } else {
            printError("Failed to backup employee data.");
        }
        return success;
    }
    
    bool backupAttendance() {
        bool success = fileManager.backupFile("attendance.dat") && 
                       fileManager.backupFile("attendance.txt");
        if (success) {
            printSuccess("Attendance data backed up successfully.");
            logger.logBackup("Attendance");
        } else {
            printError("Failed to backup attendance data.");
        }
        return success;
    }
    
    bool backupAdmins() {
        bool success = fileManager.backupFile("admins.dat") && 
                       fileManager.backupFile("admins.txt");
        if (success) {
            printSuccess("Admin data backed up successfully.");
            logger.logBackup("Admins");
        } else {
            printError("Failed to backup admin data.");
        }
        return success;
    }
    
    bool backupSettings() {
        bool success = fileManager.backupFile("settings.dat") && 
                       fileManager.backupFile("settings.txt");
        if (success) {
            printSuccess("Settings backed up successfully.");
            logger.logBackup("Settings");
        } else {
            printError("Failed to backup settings.");
        }
        return success;
    }
    
    void viewBackups() {
        printHeader("AVAILABLE BACKUPS");
        
        std::vector<std::string> backups = fileManager.listBackups();
        
        if (backups.empty()) {
            printInfo("No backups found.");
        } else {
            for (const auto& backup : backups) {
                std::cout << backup << std::endl;
            }
        }
        
        pauseScreen();
    }
    
    bool restoreData(const std::string& backupFile) {
        std::cout << "\nWARNING!" << std::endl;
        std::cout << "Current data may be replaced." << std::endl;
        
        if (!confirmAction("Continue?")) {
            printInfo("Restore cancelled.");
            return false;
        }
        
        std::string targetFile;
        if (backupFile.find("employees") != std::string::npos) {
            targetFile = backupFile.substr(0, backupFile.find_last_of('_')) + 
                         backupFile.substr(backupFile.find_last_of('.'));
        } else {
            targetFile = backupFile;
        }
        
        bool success = fileManager.restoreFile(backupFile, targetFile);
        
        if (success) {
            printSuccess("Data restored successfully.");
            logger.logRestore(backupFile);
        } else {
            printError("Failed to restore data.");
        }
        
        return success;
    }
};

// ============================================
// MAIN EAMS CLASS
// ============================================
class EAMS {
private:
    FileManager fileManager;
    Logger logger;
    EmployeeManager empManager;
    AttendanceManager attManager;
    AuthManager authManager;
    SettingsManager settingsManager;
    HolidayManager holidayManager;
    ReportManager reportManager;
    StatisticsManager statsManager;
    BackupManager backupManager;
    
    bool running;
    
public:
    EAMS() 
        : fileManager(DATA_DIR),
          logger(LOG_FILE),
          empManager(fileManager, logger),
          attManager(fileManager, logger, empManager),
          authManager(fileManager, logger),
          settingsManager(fileManager, logger),
          holidayManager(fileManager, logger),
          reportManager(fileManager, logger),
          statsManager(empManager, attManager, settingsManager.getSettings(), holidayManager),
          backupManager(fileManager, logger),
          running(true) {}
    
    void initialize() {
        printHeader("EMPLOYEE ATTENDANCE MANAGEMENT");
        
        std::cout << "\nInitializing EAMS..." << std::endl;
        std::cout << "Creating directories..." << std::endl;
        
        std::cout << "Loading employees..." << std::endl;
        empManager.load();
        
        std::cout << "Loading attendance..." << std::endl;
        attManager.load();
        
        std::cout << "Loading administrators..." << std::endl;
        authManager.load();
        
        std::cout << "Loading settings..." << std::endl;
        settingsManager.load();
        
        std::cout << "Loading holidays..." << std::endl;
        holidayManager.load();
        
        std::cout << "\nSystem Ready.\n" << std::endl;
        pauseScreen();
    }
    
    void run() {
        if (!authManager.login()) {
            printError("Login failed. Exiting...");
            return;
        }
        
        while (running) {
            showDashboard();
        }
    }
    
    void showDashboard() {
        printHeader("EAMS DASHBOARD");
        
        std::cout << "\n1. Employee Management\n";
        std::cout << "2. Attendance Management\n";
        std::cout << "3. Attendance Reports\n";
        std::cout << "4. Employee Search\n";
        std::cout << "5. Statistics\n";
        
        if (authManager.isAdmin()) {
            std::cout << "6. Admin Management\n";
            std::cout << "7. System Settings\n";
        }
        
        std::cout << "8. Backup & Restore\n";
        std::cout << "9. Holiday Management\n";
        std::cout << "0. Logout\n";
        
        printLine();
        std::cout << "Enter choice: ";
        
        int choice = getIntInRange(0, authManager.isAdmin() ? 9 : 8);
        
        switch (choice) {
            case 1: employeeManagementMenu(); break;
            case 2: attendanceManagementMenu(); break;
            case 3: reportsMenu(); break;
            case 4: employeeSearchMenu(); break;
            case 5: statsManager.displayStatistics(); break;
            case 6: 
                if (authManager.isAdmin()) adminManagementMenu(); 
                else backupRestoreMenu();
                break;
            case 7:
                if (authManager.isAdmin()) settingsMenu();
                else holidayManagementMenu();
                break;
            case 8:
                if (authManager.isAdmin()) backupRestoreMenu();
                else holidayManagementMenu();
                break;
            case 9:
                if (authManager.isAdmin()) holidayManagementMenu();
                else logout();
                break;
            case 0: logout(); break;
        }
    }
    
    void employeeManagementMenu() {
        while (true) {
            printHeader("EMPLOYEE MANAGEMENT");
            
            std::cout << "\n1. Add Employee\n";
            std::cout << "2. View All Employees\n";
            std::cout << "3. Search Employee\n";
            std::cout << "4. Update Employee\n";
            std::cout << "5. Deactivate Employee\n";
            std::cout << "6. Activate Employee\n";
            std::cout << "7. View Employee Profile\n";
            std::cout << "8. Sort Employees\n";
            std::cout << "0. Back\n";
            
            printLine();
            std::cout << "Enter choice: ";
            
            int choice = getIntInRange(0, 8);
            
            switch (choice) {
                case 1: addEmployee(); break;
                case 2: empManager.viewAllEmployees(); break;
                case 3: employeeSearchMenu(); break;
                case 4: updateEmployee(); break;
                case 5: deactivateEmployee(); break;
                case 6: activateEmployee(); break;
                case 7: viewEmployeeProfile(); break;
                case 8: sortEmployees(); break;
                case 0: return;
            }
        }
    }
    
    void addEmployee() {
        printHeader("ADD EMPLOYEE");
        
        Employee emp;
        
        emp.employeeID = getNonEmptyString("Employee ID: ");
        
        if (empManager.employeeExists(emp.employeeID)) {
            printError("Employee ID already exists.");
            pauseScreen();
            return;
        }
        
        emp.name = getNonEmptyString("Full Name: ");
        emp.gender = getNonEmptyString("Gender: ");
        
        std::cout << "Age: ";
        emp.age = getIntInRange(18, 70);
        
        emp.phone = getNonEmptyString("Phone: ");
        if (!isValidPhone(emp.phone)) {
            printError("Invalid phone number.");
            pauseScreen();
            return;
        }
        
        emp.email = getNonEmptyString("Email: ");
        if (!isValidEmail(emp.email)) {
            printError("Invalid email address.");
            pauseScreen();
            return;
        }
        
        emp.address = getNonEmptyString("Address: ");
        emp.department = getNonEmptyString("Department: ");
        emp.position = getNonEmptyString("Position: ");
        
        do {
            emp.joiningDate = getNonEmptyString("Joining Date (YYYY-MM-DD): ");
            if (!isValidDate(emp.joiningDate)) {
                printError("Invalid date format. Use YYYY-MM-DD.");
            }
        } while (!isValidDate(emp.joiningDate));
        
        emp.status = "Active";
        
        std::cout << "\nSave employee? (Y/N): ";
        if (confirmAction("")) {
            empManager.addEmployee(emp);
        } else {
            printInfo("Employee not saved.");
        }
        
        pauseScreen();
    }
    
    void updateEmployee() {
        printHeader("UPDATE EMPLOYEE");
        
        std::string empID = getNonEmptyString("Enter Employee ID: ");
        Employee* emp = empManager.findEmployee(empID);
        
        if (!emp) {
            printError("Employee not found.");
            pauseScreen();
            return;
        }
        
        // Show current data
        std::cout << "\nCurrent Data:\n";
        std::cout << "Name: " << emp->name << std::endl;
        std::cout << "Gender: " << emp->gender << std::endl;
        std::cout << "Age: " << emp->age << std::endl;
        std::cout << "Phone: " << emp->phone << std::endl;
        std::cout << "Email: " << emp->email << std::endl;
        std::cout << "Address: " << emp->address << std::endl;
        std::cout << "Department: " << emp->department << std::endl;
        std::cout << "Position: " << emp->position << std::endl;
        std::cout << "Joining Date: " << emp->joiningDate << std::endl;
        std::cout << "Status: " << emp->status << std::endl;
        
        Employee updated = *emp;
        
        std::cout << "\nEnter new values (leave blank to keep current):\n";
        
        std::string input;
        
        std::cout << "Name [" << updated.name << "]: ";
        std::getline(std::cin, input);
        if (!trim(input).empty()) updated.name = input;
        
        std::cout << "Gender [" << updated.gender << "]: ";
        std::getline(std::cin, input);
        if (!trim(input).empty()) updated.gender = input;
        
        std::cout << "Age [" << updated.age << "]: ";
        std::getline(std::cin, input);
        if (!trim(input).empty()) updated.age = std::stoi(input);
        
        std::cout << "Phone [" << updated.phone << "]: ";
        std::getline(std::cin, input);
        if (!trim(input).empty()) {
            if (!isValidPhone(input)) {
                printError("Invalid phone number.");
                pauseScreen();
                return;
            }
            updated.phone = input;
        }
        
        std::cout << "Email [" << updated.email << "]: ";
        std::getline(std::cin, input);
        if (!trim(input).empty()) {
            if (!isValidEmail(input)) {
                printError("Invalid email address.");
                pauseScreen();
                return;
            }
            updated.email = input;
        }
        
        std::cout << "Address [" << updated.address << "]: ";
        std::getline(std::cin, input);
        if (!trim(input).empty()) updated.address = input;
        
        std::cout << "Department [" << updated.department << "]: ";
        std::getline(std::cin, input);
        if (!trim(input).empty()) updated.department = input;
        
        std::cout << "Position [" << updated.position << "]: ";
        std::getline(std::cin, input);
        if (!trim(input).empty()) updated.position = input;
        
        std::cout << "Joining Date [" << updated.joiningDate << "]: ";
        std::getline(std::cin, input);
        if (!trim(input).empty()) {
            if (!isValidDate(input)) {
                printError("Invalid date format.");
                pauseScreen();
                return;
            }
            updated.joiningDate = input;
        }
        
        std::cout << "Status [" << updated.status << "]: ";
        std::getline(std::cin, input);
        if (!trim(input).empty()) updated.status = input;
        
        if (confirmAction("Update employee?")) {
            empManager.updateEmployee(empID, updated);
        } else {
            printInfo("Employee not updated.");
        }
        
        pauseScreen();
    }
    
    void deactivateEmployee() {
        printHeader("DEACTIVATE EMPLOYEE");
        
        std::string empID = getNonEmptyString("Enter Employee ID: ");
        
        if (!empManager.employeeExists(empID)) {
            printError("Employee not found.");
            pauseScreen();
            return;
        }
        
        if (confirmAction("Are you sure you want to deactivate this employee?")) {
            empManager.deactivateEmployee(empID);
        } else {
            printInfo("Operation cancelled.");
        }
        
        pauseScreen();
    }
    
    void activateEmployee() {
        printHeader("ACTIVATE EMPLOYEE");
        
        std::string empID = getNonEmptyString("Enter Employee ID: ");
        
        if (!empManager.employeeExists(empID)) {
            printError("Employee not found.");
            pauseScreen();
            return;
        }
        
        empManager.activateEmployee(empID);
        pauseScreen();
    }
    
    void viewEmployeeProfile() {
        printHeader("VIEW EMPLOYEE PROFILE");
        
        std::string empID = getNonEmptyString("Enter Employee ID: ");
        empManager.viewEmployeeProfile(empID);
    }
    
    void sortEmployees() {
        printHeader("SORT EMPLOYEES");
        
        std::cout << "\n1. Sort by ID\n";
        std::cout << "2. Sort by Name\n";
        std::cout << "3. Sort by Department\n";
        std::cout << "4. Sort by Joining Date\n";
        std::cout << "0. Back\n";
        
        printLine();
        std::cout << "Enter choice: ";
        
        int choice = getIntInRange(0, 4);
        
        if (choice > 0) {
            empManager.sortEmployees(choice);
            empManager.viewAllEmployees();
        }
    }
    
    void employeeSearchMenu() {
        printHeader("EMPLOYEE SEARCH");
        
        std::cout << "\n1. Search by Employee ID\n";
        std::cout << "2. Search by Full Name\n";
        std::cout << "3. Search by First Letter\n";
        std::cout << "4. Search by Department\n";
        std::cout << "5. Search by Position\n";
        std::cout << "6. Search by Status\n";
        std::cout << "0. Back\n";
        
        printLine();
        std::cout << "Enter choice: ";
        
        int choice = getIntInRange(0, 6);
        
        switch (choice) {
            case 1: searchByID(); break;
            case 2: searchByName(); break;
            case 3: searchByFirstLetter(); break;
            case 4: searchByDepartment(); break;
            case 5: searchByPosition(); break;
            case 6: searchByStatus(); break;
            case 0: return;
        }
    }
    
    void searchByID() {
        std::string empID = getNonEmptyString("Enter Employee ID: ");
        Employee* emp = empManager.findEmployee(empID);
        
        if (emp) {
            empManager.viewEmployeeProfile(empID);
        } else {
            printError("Employee not found.");
            pauseScreen();
        }
    }
    
    void searchByName() {
        std::string name = getNonEmptyString("Enter Name: ");
        std::vector<Employee> results = empManager.searchByName(name);
        
        displaySearchResults(results);
    }
    
    void searchByFirstLetter() {
        std::cout << "Enter First Letter: ";
        char letter;
        std::cin >> letter;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        std::vector<Employee> results = empManager.searchByFirstLetter(letter);
        displaySearchResults(results);
    }
    
    void searchByDepartment() {
        std::string dept = getNonEmptyString("Enter Department: ");
        std::vector<Employee> results = empManager.searchByDepartment(dept);
        
        displaySearchResults(results);
    }
    
    void searchByPosition() {
        std::string position = getNonEmptyString("Enter Position: ");
        std::vector<Employee> results = empManager.searchByPosition(position);
        
        displaySearchResults(results);
    }
    
    void searchByStatus() {
        std::cout << "\n1. Active\n";
        std::cout << "2. Inactive\n";
        std::cout << "Enter choice: ";
        
        int choice = getIntInRange(1, 2);
        std::string status = (choice == 1) ? "Active" : "Inactive";
        
        std::vector<Employee> results = empManager.searchByStatus(status);
        displaySearchResults(results);
    }
    
    void displaySearchResults(const std::vector<Employee>& results) {
        if (results.empty()) {
            printInfo("No employees found.");
            pauseScreen();
            return;
        }
        
        std::cout << "\nFound " << results.size() << " employee(s):\n\n";
        
        std::cout << std::left << std::setw(12) << "ID" 
                  << std::setw(25) << "Name"
                  << std::setw(15) << "Department"
                  << std::setw(15) << "Position"
                  << std::setw(10) << "Status" << std::endl;
        printLine();
        
        for (const auto& emp : results) {
            std::cout << std::left << std::setw(12) << emp.employeeID
                      << std::setw(25) << emp.name
                      << std::setw(15) << emp.department
                      << std::setw(15) << emp.position
                      << std::setw(10) << emp.status << std::endl;
        }
        
        pauseScreen();
    }
    
    void attendanceManagementMenu() {
        while (true) {
            printHeader("ATTENDANCE MANAGEMENT");
            
            std::cout << "\n1. Mark Attendance\n";
            std::cout << "2. Check In\n";
            std::cout << "3. Check Out\n";
            std::cout << "4. Mark Absent\n";
            std::cout << "5. Mark Leave\n";
            std::cout << "6. Mark Holiday\n";
            std::cout << "7. View Today's Attendance\n";
            std::cout << "8. Attendance History\n";
            std::cout << "9. Edit Attendance\n";
            std::cout << "10. Delete Attendance\n";
            std::cout << "0. Back\n";
            
            printLine();
            std::cout << "Enter choice: ";
            
            int choice = getIntInRange(0, 10);
            
            switch (choice) {
                case 1: markAttendance(); break;
                case 2: checkIn(); break;
                case 3: checkOut(); break;
                case 4: markAbsent(); break;
                case 5: markLeave(); break;
                case 6: markHolidayStatus(); break;
                case 7: viewTodayAttendance(); break;
                case 8: viewAttendanceHistory(); break;
                case 9: editAttendance(); break;
                case 10: deleteAttendance(); break;
                case 0: return;
            }
        }
    }
    
    void markAttendance() {
        printHeader("MARK ATTENDANCE");
        
        Attendance att;
        att.employeeID = getNonEmptyString("Employee ID: ");
        
        if (!empManager.employeeExists(att.employeeID)) {
            printError("Employee does not exist.");
            pauseScreen();
            return;
        }
        
        do {
            att.date = getNonEmptyString("Date (YYYY-MM-DD): ");
            if (!isValidDate(att.date)) {
                printError("Invalid date format.");
            }
        } while (!isValidDate(att.date));
        
        std::cout << "Check In (HH:MM, leave blank if none): ";
        std::getline(std::cin, att.checkIn);
        att.checkIn = trim(att.checkIn);
        
        std::cout << "Check Out (HH:MM, leave blank if none): ";
        std::getline(std::cin, att.checkOut);
        att.checkOut = trim(att.checkOut);
        
        std::cout << "Status (Present/Absent/Late/Leave/Holiday): ";
        std::getline(std::cin, att.status);
        att.status = trim(att.status);
        
        std::cout << "Remarks: ";
        std::getline(std::cin, att.remarks);
        
        if (attManager.markAttendance(att, settingsManager.getSettings())) {
            printSuccess("Attendance marked successfully.");
        }
        
        pauseScreen();
    }
    
    void checkIn() {
        printHeader("CHECK IN");
        
        std::string empID = getNonEmptyString("Employee ID: ");
        
        if (attManager.checkIn(empID, settingsManager.getSettings())) {
            printSuccess("Check-in successful!");
        }
        
        pauseScreen();
    }
    
    void checkOut() {
        printHeader("CHECK OUT");
        
        std::string empID = getNonEmptyString("Employee ID: ");
        
        if (attManager.checkOut(empID)) {
            printSuccess("Check-out successful!");
        }
        
        pauseScreen();
    }
    
    void markAbsent() {
        printHeader("MARK ABSENT");
        
        Attendance att;
        att.employeeID = getNonEmptyString("Employee ID: ");
        att.date = getCurrentDate();
        att.status = "Absent";
        att.remarks = "Marked as absent";
        
        if (attManager.markAttendance(att, settingsManager.getSettings())) {
            printSuccess("Marked as absent.");
        }
        
        pauseScreen();
    }
    
    void markLeave() {
        printHeader("MARK LEAVE");
        
        Attendance att;
        att.employeeID = getNonEmptyString("Employee ID: ");
        
        do {
            att.date = getNonEmptyString("Date (YYYY-MM-DD): ");
            if (!isValidDate(att.date)) {
                printError("Invalid date format.");
            }
        } while (!isValidDate(att.date));
        
        att.status = "Leave";
        att.remarks = "On leave";
        
        if (attManager.markAttendance(att, settingsManager.getSettings())) {
            printSuccess("Marked as on leave.");
        }
        
        pauseScreen();
    }
    
    void markHolidayStatus() {
        printHeader("MARK HOLIDAY");
        
        Attendance att;
        att.employeeID = getNonEmptyString("Employee ID: ");
        
        do {
            att.date = getNonEmptyString("Date (YYYY-MM-DD): ");
            if (!isValidDate(att.date)) {
                printError("Invalid date format.");
            }
        } while (!isValidDate(att.date));
        
        att.status = "Holiday";
        att.remarks = "Holiday";
        
        if (attManager.markAttendance(att, settingsManager.getSettings())) {
            printSuccess("Marked as holiday.");
        }
        
        pauseScreen();
    }
    
    void viewTodayAttendance() {
        attManager.viewTodayAttendance(settingsManager.getSettings(), holidayManager.getHolidays());
    }
    
    void viewAttendanceHistory() {
        printHeader("ATTENDANCE HISTORY");
        
        std::string empID = getNonEmptyString("Enter Employee ID: ");
        attManager.viewAttendanceHistory(empID);
    }
    
    void editAttendance() {
        printHeader("EDIT ATTENDANCE");
        
        std::string empID = getNonEmptyString("Employee ID: ");
        
        do {
            std::string date = getNonEmptyString("Date (YYYY-MM-DD): ");
            if (!isValidDate(date)) {
                printError("Invalid date format.");
                continue;
            }
            
            Attendance* att = attManager.findAttendance(empID, date);
            if (!att) {
                printError("Attendance record not found.");
                pauseScreen();
                return;
            }
            
            std::cout << "\nCurrent Data:\n";
            std::cout << "Check In: " << att->checkIn << std::endl;
            std::cout << "Check Out: " << att->checkOut << std::endl;
            std::cout << "Status: " << att->status << std::endl;
            std::cout << "Remarks: " << att->remarks << std::endl;
            
            Attendance updated = *att;
            
            std::string input;
            std::cout << "\nCheck In [" << updated.checkIn << "]: ";
            std::getline(std::cin, input);
            if (!trim(input).empty()) updated.checkIn = input;
            
            std::cout << "Check Out [" << updated.checkOut << "]: ";
            std::getline(std::cin, input);
            if (!trim(input).empty()) updated.checkOut = input;
            
            std::cout << "Status [" << updated.status << "]: ";
            std::getline(std::cin, input);
            if (!trim(input).empty()) updated.status = input;
            
            std::cout << "Remarks [" << updated.remarks << "]: ";
            std::getline(std::cin, input);
            if (!trim(input).empty()) updated.remarks = input;
            
            if (confirmAction("Update attendance?")) {
                attManager.editAttendance(empID, date, updated);
            } else {
                printInfo("Attendance not updated.");
            }
            
            pauseScreen();
            return;
        } while (true);
    }
    
    void deleteAttendance() {
        printHeader("DELETE ATTENDANCE");
        
        std::string empID = getNonEmptyString("Employee ID: ");
        
        do {
            std::string date = getNonEmptyString("Date (YYYY-MM-DD): ");
            if (!isValidDate(date)) {
                printError("Invalid date format.");
                continue;
            }
            
            if (confirmAction("Are you sure you want to delete this attendance record?")) {
                attManager.deleteAttendance(empID, date);
            } else {
                printInfo("Operation cancelled.");
            }
            
            pauseScreen();
            return;
        } while (true);
    }
    
    void reportsMenu() {
        while (true) {
            printHeader("ATTENDANCE REPORTS");
            
            std::cout << "\n1. Daily Report\n";
            std::cout << "2. Weekly Report\n";
            std::cout << "3. Monthly Report\n";
            std::cout << "4. Employee Report\n";
            std::cout << "5. Department Report\n";
            std::cout << "6. Absent Report\n";
            std::cout << "7. Late Report\n";
            std::cout << "8. Attendance Percentage\n";
            std::cout << "9. Export Report\n";
            std::cout << "0. Back\n";
            
            printLine();
            std::cout << "Enter choice: ";
            
            int choice = getIntInRange(0, 9);
            
            switch (choice) {
                case 1: dailyReport(); break;
                case 2: weeklyReport(); break;
                case 3: monthlyReport(); break;
                case 4: employeeReport(); break;
                case 5: departmentReport(); break;
                case 6: absentReport(); break;
                case 7: lateReport(); break;
                case 8: attendancePercentage(); break;
                case 9: exportReport(); break;
                case 0: return;
            }
        }
    }
    
    void dailyReport() {
        std::string date = getNonEmptyString("Enter Date (YYYY-MM-DD, leave blank for today): ");
        if (date.empty()) date = getCurrentDate();
        
        if (!isValidDate(date)) {
            printError("Invalid date format.");
            pauseScreen();
            return;
        }
        
        reportManager.generateDailyReport(date, attManager, empManager, 
                                          settingsManager.getSettings(), holidayManager.getHolidays());
    }
    
    void weeklyReport() {
        std::string startDate = getNonEmptyString("Enter Start Date (YYYY-MM-DD): ");
        
        if (!isValidDate(startDate)) {
            printError("Invalid date format.");
            pauseScreen();
            return;
        }
        
        reportManager.generateWeeklyReport(startDate, attManager, empManager,
                                          settingsManager.getSettings(), holidayManager.getHolidays());
    }
    
    void monthlyReport() {
        std::string empID = getNonEmptyString("Enter Employee ID: ");
        std::string yearMonth = getNonEmptyString("Enter Year-Month (YYYY-MM): ");
        
        reportManager.generateMonthlyReport(empID, yearMonth, attManager, empManager,
                                           settingsManager.getSettings(), holidayManager.getHolidays());
    }
    
    void employeeReport() {
        std::string empID = getNonEmptyString("Enter Employee ID: ");
        reportManager.generateEmployeeReport(empID, attManager, empManager);
    }
    
    void departmentReport() {
        std::string department = getNonEmptyString("Enter Department: ");
        std::string date = getNonEmptyString("Enter Date (YYYY-MM-DD, leave blank for today): ");
        if (date.empty()) date = getCurrentDate();
        
        reportManager.generateDepartmentReport(department, date, attManager, empManager,
                                             settingsManager.getSettings(), holidayManager.getHolidays());
    }
    
    void absentReport() {
        std::string date = getNonEmptyString("Enter Date (YYYY-MM-DD, leave blank for today): ");
        if (date.empty()) date = getCurrentDate();
        
        reportManager.generateAbsentReport(date, attManager, empManager,
                                         settingsManager.getSettings(), holidayManager.getHolidays());
    }
    
    void lateReport() {
        std::string date = getNonEmptyString("Enter Date (YYYY-MM-DD, leave blank for today): ");
        if (date.empty()) date = getCurrentDate();
        
        reportManager.generateLateReport(date, attManager, empManager);
    }
    
    void attendancePercentage() {
        std::string empID = getNonEmptyString("Enter Employee ID: ");
        std::string startDate = getNonEmptyString("Start Date (YYYY-MM-DD): ");
        std::string endDate = getNonEmptyString("End Date (YYYY-MM-DD): ");
        
        reportManager.generateAttendancePercentage(empID, startDate, endDate, attManager, empManager,
                                                 settingsManager.getSettings(), holidayManager.getHolidays());
    }
    
    void exportReport() {
        printHeader("EXPORT REPORT");
        
        std::string filename = getNonEmptyString("Enter filename: ");
        std::cout << "Enter report content:\n";
        
        std::string content;
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line == ".") break;
            content += line + "\n";
        }
        
        reportManager.exportReport(filename, content);
        pauseScreen();
    }
    
    void adminManagementMenu() {
        while (true) {
            printHeader("ADMIN MANAGEMENT");
            
            std::cout << "\n1. Add Admin\n";
            std::cout << "2. View Admins\n";
            std::cout << "3. Change Password\n";
            std::cout << "4. Delete Admin\n";
            std::cout << "5. Change Role\n";
            std::cout << "0. Back\n";
            
            printLine();
            std::cout << "Enter choice: ";
            
            int choice = getIntInRange(0, 5);
            
            switch (choice) {
                case 1: addAdmin(); break;
                case 2: authManager.viewAllAdmins(); break;
                case 3: authManager.changePassword(); break;
                case 4: deleteAdmin(); break;
                case 5: changeAdminRole(); break;
                case 0: return;
            }
        }
    }
    
    void addAdmin() {
        printHeader("ADD ADMIN");
        
        std::string username = getNonEmptyString("Username: ");
        std::string password;
        std::string confirmPassword;
        
        do {
            std::cout << "Password: ";
            char c;
            password.clear();
            while ((c = getPasswordChar()) != '\r' && c != '\n') {
                if (c == '\b' || c == 127) {
                    if (!password.empty()) {
                        password.pop_back();
                        std::cout << "\b \b";
                    }
                } else {
                    password += c;
                    std::cout << '*';
                }
            }
            std::cout << std::endl;
            
            if (password.length() < PASSWORD_MIN_LENGTH) {
                printError("Password must be at least " + std::to_string(PASSWORD_MIN_LENGTH) + " characters.");
                continue;
            }
            
            std::cout << "Confirm Password: ";
            confirmPassword.clear();
            while ((c = getPasswordChar()) != '\r' && c != '\n') {
                if (c == '\b' || c == 127) {
                    if (!confirmPassword.empty()) {
                        confirmPassword.pop_back();
                        std::cout << "\b \b";
                    }
                } else {
                    confirmPassword += c;
                    std::cout << '*';
                }
            }
            std::cout << std::endl;
            
            if (password != confirmPassword) {
                printError("Passwords do not match.");
            } else {
                break;
            }
        } while (true);
        
        std::cout << "\n1. Administrator\n";
        std::cout << "2. Manager\n";
        std::cout << "Role: ";
        int roleChoice = getIntInRange(1, 2);
        std::string role = (roleChoice == 1) ? "Administrator" : "Manager";
        
        authManager.addAdmin(username, password, role);
        pauseScreen();
    }
    
    void deleteAdmin() {
        printHeader("DELETE ADMIN");
        
        std::string username = getNonEmptyString("Enter Username: ");
        
        if (confirmAction("Are you sure you want to delete this admin?")) {
            authManager.deleteAdmin(username);
        } else {
            printInfo("Operation cancelled.");
        }
        
        pauseScreen();
    }
    
    void changeAdminRole() {
        printHeader("CHANGE ADMIN ROLE");
        
        std::string username = getNonEmptyString("Enter Username: ");
        
        std::cout << "\n1. Administrator\n";
        std::cout << "2. Manager\n";
        std::cout << "New Role: ";
        int roleChoice = getIntInRange(1, 2);
        std::string newRole = (roleChoice == 1) ? "Administrator" : "Manager";
        
        authManager.changeAdminRole(username, newRole);
        pauseScreen();
    }
    
    void settingsMenu() {
        while (true) {
            printHeader("SYSTEM SETTINGS");
            
            std::cout << "\n1. Company Name\n";
            std::cout << "2. Office Start Time\n";
            std::cout << "3. Office End Time\n";
            std::cout << "4. Late Threshold\n";
            std::cout << "5. Working Days\n";
            std::cout << "6. View Settings\n";
            std::cout << "0. Back\n";
            
            printLine();
            std::cout << "Enter choice: ";
            
            int choice = getIntInRange(0, 6);
            
            switch (choice) {
                case 1: updateCompanyName(); break;
                case 2: updateOfficeStartTime(); break;
                case 3: updateOfficeEndTime(); break;
                case 4: updateLateThreshold(); break;
                case 5: updateWorkingDays(); break;
                case 6: settingsManager.viewSettings(); break;
                case 0: return;
            }
        }
    }
    
    void updateCompanyName() {
        std::string name = getNonEmptyString("Enter Company Name: ");
        settingsManager.updateCompanyName(name);
        pauseScreen();
    }
    
    void updateOfficeStartTime() {
        std::string time = getNonEmptyString("Enter Office Start Time (HH:MM): ");
        settingsManager.updateOfficeStartTime(time);
        pauseScreen();
    }
    
    void updateOfficeEndTime() {
        std::string time = getNonEmptyString("Enter Office End Time (HH:MM): ");
        settingsManager.updateOfficeEndTime(time);
        pauseScreen();
    }
    
    void updateLateThreshold() {
        std::cout << "Enter Late Threshold (minutes): ";
        int minutes = getInt();
        settingsManager.updateLateThreshold(minutes);
        pauseScreen();
    }
    
    void updateWorkingDays() {
        std::cout << "Enter Working Days (comma-separated, e.g., Monday,Tuesday,Wednesday): ";
        std::string days;
        std::getline(std::cin, days);
        settingsManager.updateWorkingDays(days);
        pauseScreen();
    }
    
    void backupRestoreMenu() {
        while (true) {
            printHeader("BACKUP & RESTORE");
            
            std::cout << "\n1. Backup All Data\n";
            std::cout << "2. Backup Employees\n";
            std::cout << "3. Backup Attendance\n";
            std::cout << "4. Backup Admins\n";
            std::cout << "5. Backup Settings\n";
            std::cout << "6. Restore Data\n";
            std::cout << "7. View Backups\n";
            std::cout << "0. Back\n";
            
            printLine();
            std::cout << "Enter choice: ";
            
            int choice = getIntInRange(0, 7);
            
            switch (choice) {
                case 1: backupManager.backupAll(); break;
                case 2: backupManager.backupEmployees(); break;
                case 3: backupManager.backupAttendance(); break;
                case 4: backupManager.backupAdmins(); break;
                case 5: backupManager.backupSettings(); break;
                case 6: restoreData(); break;
                case 7: backupManager.viewBackups(); break;
                case 0: return;
            }
            
            pauseScreen();
        }
    }
    
    void restoreData() {
        printHeader("RESTORE DATA");
        
        backupManager.viewBackups();
        
        std::string filename = getNonEmptyString("Enter backup filename to restore: ");
        backupManager.restoreData(filename);
    }
    
    void holidayManagementMenu() {
        while (true) {
            printHeader("HOLIDAY MANAGEMENT");
            
            std::cout << "\n1. Add Holiday\n";
            std::cout << "2. View Holidays\n";
            std::cout << "3. Delete Holiday\n";
            std::cout << "0. Back\n";
            
            printLine();
            std::cout << "Enter choice: ";
            
            int choice = getIntInRange(0, 3);
            
            switch (choice) {
                case 1: addHoliday(); break;
                case 2: holidayManager.viewHolidays(); break;
                case 3: deleteHoliday(); break;
                case 0: return;
            }
        }
    }
    
    void addHoliday() {
        printHeader("ADD HOLIDAY");
        
        std::string date, name;
        
        do {
            date = getNonEmptyString("Date (YYYY-MM-DD): ");
            if (!isValidDate(date)) {
                printError("Invalid date format.");
            }
        } while (!isValidDate(date));
        
        name = getNonEmptyString("Holiday Name: ");
        
        holidayManager.addHoliday(date, name);
        pauseScreen();
    }
    
    void deleteHoliday() {
        printHeader("DELETE HOLIDAY");
        
        std::string date = getNonEmptyString("Enter Date (YYYY-MM-DD): ");
        
        if (confirmAction("Are you sure you want to delete this holiday?")) {
            holidayManager.deleteHoliday(date);
        } else {
            printInfo("Operation cancelled.");
        }
        
        pauseScreen();
    }
    
    void logout() {
        printHeader("LOGOUT");
        
        std::cout << "\nSaving data...\n" << std::endl;
        
        if (empManager.save()) printSuccess("Employees saved.");
        else printError("Failed to save employees.");
        
        if (attManager.save()) printSuccess("Attendance saved.");
        else printError("Failed to save attendance.");
        
        if (authManager.save()) printSuccess("Admins saved.");
        else printError("Failed to save admins.");
        
        if (settingsManager.save()) printSuccess("Settings saved.");
        else printError("Failed to save settings.");
        
        if (holidayManager.save()) printSuccess("Holidays saved.");
        else printError("Failed to save holidays.");
        
        authManager.logout();
        
        printSuccess("Logout successful.");
        running = false;
        
        pauseScreen();
    }
    
    void shutdown() {
        printHeader("EXIT");
        
        std::cout << "\nSaving all data...\n" << std::endl;
        
        if (empManager.save()) printSuccess("Employees saved.");
        if (attManager.save()) printSuccess("Attendance saved.");
        if (authManager.save()) printSuccess("Admins saved.");
        if (settingsManager.save()) printSuccess("Settings saved.");
        if (holidayManager.save()) printSuccess("Holidays saved.");
        
        std::cout << "\nThank you for using EAMS." << std::endl;
    }
};

// ============================================
// MAIN FUNCTION
// ============================================
int main() {
    EAMS eams;
    
    try {
        eams.initialize();
        eams.run();
        eams.shutdown();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
