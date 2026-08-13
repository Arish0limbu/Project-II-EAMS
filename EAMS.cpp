#include <iostream>
#include <string.h>

using namespace std;

bool login()
{
    string username, password;

    for (int attempt = 1; attempt <= 3; attempt++)
    {
        cout << "\n===== LOGIN =====\n";

        cout << "Username: ";
        cin >> username;

        cout << "Password: ";
        cin >> password;

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

    return 0;
}