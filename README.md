<div align="center">

```
███████╗ █████╗ ███╗   ███╗███████╗
██╔════╝██╔══██╗████╗ ████║██╔════╝
█████╗  ███████║██╔████╔██║███████╗
██╔══╝  ██╔══██║██║╚██╔╝██║╚════██║
███████╗██║  ██║██║ ╚═╝ ██║███████║
╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝
```

### EMPLOYEE ATTENDANCE MANAGEMENT SYSTEM

**A C++ powered employee management and attendance system — role-based, terminal-driven, file-persisted.**

![C++](https://img.shields.io/badge/Language-C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-2ea44f?style=for-the-badge&logo=gnubash&logoColor=white)
![Interface](https://img.shields.io/badge/Interface-CLI%20Terminal-000000?style=for-the-badge&logo=windowsterminal&logoColor=white)
![Storage](https://img.shields.io/badge/Storage-Flat--File%20(.txt)-orange?style=for-the-badge&logo=files&logoColor=white)
![Status](https://img.shields.io/badge/Status-Active-success?style=for-the-badge)

```
┌──────────────────────────────────────────────┐
│  SYSTEM     : EAMS                            │
│  LANGUAGE   : C++                             │
│  MODE       : EMPLOYEE ATTENDANCE MANAGEMENT  │
│  STORAGE    : FLAT-FILE (.txt)                │
│  STATUS     : ● ONLINE                        │
└──────────────────────────────────────────────┘
```

**[Features](#️-features) · [Preview](#-system-preview) · [Admin](#-admin-control-center) · [Employee](#-employee-portal) · [Attendance](#-attendance-system) · [Leave](#-leave-management) · [Install](#-installation) · [Author](#-author)**

</div>

---

## 📖 About

**EAMS** is a single-binary, terminal-based workforce system written in standalone C++. It runs two distinct sessions from one login screen — an **Admin** who manages people and records, and an **Employee** who checks in and files leave — with every table persisted to aligned, human-readable `.txt` files. No database, no server, no dependencies beyond a C++ compiler.

<div align="center">

| | | | |
|---|---|---|
| **Language** | C++ | **Roles** | Admin · Employee |
| **Interface** | CLI (Terminal) | **Persistence** | Flat-file (`.txt`) |
| **Cross-Platform** | `termios` / `conio.h` | **Footprint** | Single `.cpp` file |

</div>

---

## 🖥️ System Preview

> Terminal UI concept below — illustrative mockup, **not** an actual screenshot.

```
╔════════════════════════════════════════════════════╗
║           EMPLOYEE ATTENDANCE MANAGEMENT            ║
║                      SYSTEM                         ║
╠════════════════════════════════════════════════════╣
║                                                      ║
║   [1] Admin Login                                   ║
║   [2] Employee Login                                ║
║   [0] Exit                                          ║
║                                                      ║
╚════════════════════════════════════════════════════╝
```

```
╔════════════════════════════════════════════════════╗
║                     EAMS LOGIN                      ║
╠════════════════════════════════════════════════════╣
║                                                      ║
║   User ID     : EMP004                              ║
║   Password    : ••••••••                            ║
║                                                      ║
║   [ 2 ATTEMPT(S) LEFT ]                              ║
║   [ SYSTEM READY ]                                   ║
║                                                      ║
╚════════════════════════════════════════════════════╝
```

---

## ⚙️ Features

<table>
<tr>
<td width="50%" valign="top">

**🔐 Dual-Role Authentication**
One login screen resolves to either an Admin or an Employee session — employee IDs are accepted as raw numbers or `EMP###` format.

</td>
<td width="50%" valign="top">

**🕶️ Masked Password Entry**
A custom keystroke reader (`termios` on Linux/macOS, `conio.h` on Windows) echoes `*` per character with backspace support.

</td>
</tr>
<tr>
<td width="50%" valign="top">

**🚫 3-Attempt Login Lockout**
A single shared counter across admin and employee attempts — three misses and you're denied access for that session.

</td>
<td width="50%" valign="top">

**🔁 Forced First-Login Reset**
New employees log in with their Employee ID as the password and must set a new one (confirmed twice, can't reuse the old one) before reaching the dashboard.

</td>
</tr>
<tr>
<td width="50%" valign="top">

**👥 Batch Employee Onboarding**
Add one or many employees in a single pass — pulls Department/Position from existing registries and auto-generates sequential `EMP###` IDs.

</td>
<td width="50%" valign="top">

**🏷️ Department & Position Registries**
Independent add / edit / delete management for both — required to exist before any employee can be created.

</td>
</tr>
<tr>
<td width="50%" valign="top">

**✏️ Employee Record Editing**
Field-by-field updates — name, password, age, department, position, contact — press Enter to keep the current value.

</td>
<td width="50%" valign="top">

**🗑️ Safe Employee Removal**
Confirmation-gated deletion with an optional cascade to also wipe that employee's attendance history.

</td>
</tr>
<tr>
<td width="50%" valign="top">

**🔍 Employee Search**
Look up by exact Employee ID or a partial, case-insensitive name match.

</td>
<td width="50%" valign="top">

**🕒 Self-Service Attendance**
Employees mark **Present / Absent / Leave** for today only — Present auto-stamps time-in (now) and time-out (05:00 PM).

</td>
</tr>
<tr>
<td width="50%" valign="top">

**🛠️ Admin Attendance Override**
Admins can create or correct any employee's record for any date, with a 4th status — **Half Day** — unavailable to employees.

</td>
<td width="50%" valign="top">

**📅 Daily & Monthly Views**
Filter every employee's attendance by a single date or by month (`MM/YYYY`).

</td>
</tr>
<tr>
<td width="50%" valign="top">

**📊 Attendance Reporting**
Per-employee Present / Absent / Leave / Half-Day tallies and attendance % — with optional export to `report.txt`.

</td>
<td width="50%" valign="top">

**🌴 Leave Requests**
Employees file Sick / Casual / Annual / Other leave with a date range and reason — duplicate requests on the same start date are blocked.

</td>
</tr>
<tr>
<td width="50%" valign="top">

**✅ Leave Approval Workflow**
Admins approve or reject pending requests from a numbered queue — approval auto-marks that date's attendance as `Leave`.

</td>
<td width="50%" valign="top">

**💾 Flat-File Persistence**
Every table loads on startup and saves on every change, formatted into aligned, bordered, human-readable `.txt` tables.

</td>
</tr>
<tr>
<td colspan="2" valign="top">

**🧪 Input Validation Suite** — dedicated checks for full names (letters + single spaces, 2+ words), 10-digit contact numbers, basic email shape (`@` + trailing `.`), and non-empty addresses — every field is re-prompted until valid.

</td>
</tr>
</table>

---

## 👑 Admin Control Center

The Admin dashboard branches into three management zones:

**🗂️ Employee Management**
- Add New Employee *(batch, with generated ID + password = ID)*
- Add / Manage Departments *(edit, delete)*
- Add / Manage Positions *(edit, delete)*
- Update Employee Information
- Remove Employee *(with attendance cascade option)*

**📈 View Employees & Attendance**
- View All Employee Records · Search Employee
- View Employee Attendance *(per-person history + %)*
- View Daily Attendance · View Monthly Attendance
- View Overall Attendance Report *(exportable to `report.txt`)*
- View Leave Records *(all statuses)*

**📝 Review Leave & Update Records**
- View Pending Leave Requests
- Approve Leave Request → syncs attendance to `Leave`
- Reject Leave Request
- Update Attendance Records *(manual create/correct, all 4 statuses)*
- Update Employee Information

<details>
<summary>💻 Exact terminal menu structure</summary>

```
ADMIN DASHBOARD
 ├─ [1] Employee Management
 │    ├─ Add New Employee
 │    ├─ Add Department        ├─ Manage Departments
 │    ├─ Add Position          ├─ Manage Positions
 │    ├─ Update Employee Information
 │    └─ Remove Employee
 ├─ [2] View Employees & Attendance
 │    ├─ View All Employee Records
 │    ├─ Search Employee
 │    ├─ View Employee Attendance
 │    ├─ View Daily Attendance
 │    ├─ View Monthly Attendance
 │    ├─ View Overall Attendance Report
 │    └─ View Leave Records
 ├─ [3] Review Leave & Update Records
 │    ├─ View Leave Requests
 │    ├─ Approve Leave Request
 │    ├─ Reject Leave Request
 │    ├─ Update Attendance Records
 │    └─ Update Employee Information
 └─ [4] Logout
```

</details>

---

## 🧑‍💼 Employee Portal

Each employee gets a personal dashboard scoped to their own records only:

| # | Action | Detail |
|---|--------|--------|
| 1 | **Mark Today's Attendance** | Present / Absent / Leave — one entry per day |
| 2 | **View Attendance** | Full personal history + Present/Absent/Leave summary and % |
| 3 | **Apply for Leave** | Sick, Casual, Annual, or Other — with date range + reason |
| 4 | **View Leave Requests** | Every request they've filed, with live status |
| 5 | **View Profile** | ID, name, age, department, position, contact |
| 6 | **Logout** | Ends the session, returns to the login screen |

---

## 🕒 Attendance System

<div align="center">

| Status | Marked By | Meaning |
|:---:|:---:|---|
| ● **PRESENT** | Employee / Admin | Full day — auto time-in/out on self-mark |
| ● **ABSENT** | Employee / Admin | No attendance recorded for the day |
| ● **LEAVE** | Employee / Admin / Auto | Set directly, or automatically on leave approval |
| ● **HALF DAY** | Admin only | Manual override — not selectable by employees |

</div>

Reports tally each status per employee and compute an **Attendance Percentage**, where a Half Day counts as `0.5` toward the present total:

```
Present: 18   Absent: 2   Leave: 3   Half Day: 1
Attendance Percentage: 84.62%
```

---

## 🌴 Leave Management

```
                    EMPLOYEE
                       │
                       ▼
            Submit Leave Request
        (Type · From/To Date · Reason)
                       │
                       ▼
                    PENDING
                       │
                       ▼
                 ADMIN REVIEW
                  ┌────┴────┐
                  ▼         ▼
             APPROVED   REJECTED
                  │
                  ▼
     Attendance auto-marked "Leave"
          for the request's date
```

Leave types: `Sick Leave` · `Casual Leave` · `Annual Leave` · `Other` — each request gets a generated ID (`L001`, `L002`, …) and duplicate requests for the same start date are rejected outright.

---

## 💾 Data Storage

Every entity round-trips through its own bordered, fixed-width `.txt` table — no external database.

```
EAMS/
├── EAMS.cpp                # single-file application
├── employees.txt           # employee master records          (generated)
├── attendance.txt          # daily attendance log              (generated)
├── leave_requests.txt      # leave applications + status       (generated)
├── departments.txt         # department registry               (generated)
├── positions.txt           # position registry                 (generated)
├── config.txt              # optional admin credential override (generated)
└── report.txt              # optional exported attendance report (generated)
```

<details>
<summary>📄 Sample row format — <code>employees.txt</code></summary>

```
========================================================================================================================
                                             EAMS - EMPLOYEE RECORDS
========================================================================================================================
ID        NAME                DEPARTMENT        POSITION          CONTACT       EMAIL               STATUS    FIRST LOGIN
------------------------------------------------------------------------------------------------------------------------
EMP001    Sarah Johnson       Engineering       Software Engineer 9812345678    sarah@example.com   Active    No
========================================================================================================================
```

</details>

---

## 🧠 C++ Concepts

| Concept | Where it shows up |
|---|---|
| **Classes** | `EAMS` — the core engine encapsulating all data + behavior |
| **Structs** | `Employee`, `AttendanceRecord`, `LeaveRequest`, `Department`, `Position`, `LoginResult` |
| **STL Containers** | `vector<Employee>`, `vector<AttendanceRecord>`, `vector<LeaveRequest>` … |
| **File I/O** | `ifstream` / `ofstream` load & save for every entity |
| **String Streams** | `stringstream` / `ostringstream` for parsing and ID formatting (`EMP001`, `L001`) |
| **Pointers** | `Employee*`, `LeaveRequest*` returned from lookup helpers |
| **Lambdas** | `remove_if` predicates for deleting employees/attendance records |
| **Exception Handling** | `try/catch` guarding every `stoi()` ID conversion |
| **Conditional Compilation** | `#ifdef _WIN32` branches for `conio.h` vs. `termios.h` |
| **Function Overloading** | `changePassword()`, `fitWidth()` (string vs. int overloads) |
| **Input Validation** | Dedicated validators for names, emails, contacts, addresses |
| **const Correctness** | `const string EMP_FILE = "employees.txt"`, `const string&` params throughout |

---

## 🧩 System Architecture

```
                    ┌─────────┐
                    │  EAMS   │
                    └────┬────┘
                         ▼
                    ┌─────────┐
                    │  LOGIN  │
                    └────┬────┘
                         ▼
              ┌──────────┴──────────┐
              ▼                     ▼
          ┌───────┐             ┌──────────┐
          │ ADMIN │             │ EMPLOYEE │
          └───┬───┘             └────┬─────┘
              ▼                      ▼
        ┌──────────┐          ┌──────────┐
        │ DASHBOARD│          │ DASHBOARD│
        └────┬─────┘          └────┬─────┘
              └──────────┬─────────┘
                         ▼
     ┌───────────────────────────────────────┐
     │  MANAGEMENT · ATTENDANCE · LEAVE · REPORTS │
     └────────────────────┬────────────────────┘
                          ▼
                  ┌────────────────┐
                  │  TXT FILE STORE │
                  └────────────────┘
```

---

## 🚀 Installation

```bash
# 1. Clone
git clone https://github.com/Arish0limbu/EAMS.git
cd EAMS

# 2. Compile
g++ -std=c++11 -O2 -o eams EAMS.cpp

# 3. Run
./eams
```

**Windows:** `g++ -std=c++11 -O2 -o eams.exe EAMS.cpp` then run `eams.exe`.

---

## 🔑 Default Login *(Development Only)*

> ⚠️ Plaintext credentials in flat files — **not** production-grade security.

<div align="center">

| Role | User ID | Default Password | Notes |
|---|---|---|---|
| **Admin** | `admin` | `1234` | Overridable via optional `config.txt` (username on line 1, password on line 2) |
| **Employee** | `EMP###` (assigned) | Same as Employee ID | Forced password change on first login |

</div>

---

## 🗂️ Project Structure

```
EAMS/
└── EAMS.cpp     # everything lives here — models, persistence, menus, main()
```

Data files (`employees.txt`, `attendance.txt`, `leave_requests.txt`, `departments.txt`, `positions.txt`, `config.txt`, `report.txt`) are created and maintained automatically at runtime — see [Data Storage](#-data-storage).

---

## 👨‍💻 Author

<div align="center">

**Arish Limbu**

[![GitHub](https://img.shields.io/badge/GitHub-Arish0limbu-181717?style=for-the-badge&logo=github&logoColor=white)](https://github.com/Arish0limbu)

</div>

---

<div align="center">

────────────────────────────────────────
&nbsp;&nbsp;&nbsp;&nbsp;EAMS // SYSTEM TERMINATED
&nbsp;&nbsp;&nbsp;&nbsp;BUILT WITH C++ ⚡
────────────────────────────────────────

</div>
