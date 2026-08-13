#include <iostream>
#include <string>
#include <conio.h>
using namespace std;

string hidePassword()
{
    string password;
    char ch;

    while (true)
    {
        ch = _getch();

        if (ch == 13) // Enter
        {
            cout << endl;
            break;
        }
        else if (ch == 8) // Backspace
        {
            if (!password.empty())
            {
                password.pop_back();
                cout << "\b \b";
            }
        }
        else if (ch >= 32 && ch <= 126)
        {
            password += ch;
            cout << '*';
        }
    }

    return password;
}

bool login()
{
    string username, password;

    for (int attempt = 1; attempt <= 3; attempt++)
    {
        cout << "\n===== LOGIN =====\n";

        cout << "Username: ";
        cin >> username;

        cout << "Password: ";
        password = hidePassword();

        if (username == "admin" && password == "1234")
        {
            cout << "\nLogin successful!\n";
            return true;
        }

        cout << "\nWrong username or password!\n";
        cout << "Attempts left: " << 3 - attempt << endl;
    }

    cout << "\nToo many failed attempts. Access denied!\n";
    return false;
}

int main()
{
    if (login())
    {
        cout << "Welcome Admin!\n";
    }

    return 0;
}

int main()
{
    if (login())
    {
        cout << " hello";
    }
    return 0;
}