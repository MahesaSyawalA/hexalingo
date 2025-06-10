#include <iostream>
#include <string>
#include "auth/user.h"
#include "auth/auth.h"

using namespace std;

void showMainMenu()
{
    system("cls"); // Gunakan "cls" jika di Windows, "clear" jika di Linux/Mac
    cout << "\n========================================\n";
    cout << "              Hexalingo\n";
    cout << "========================================\n";
    cout << "1. Login\n";
    cout << "2. Register\n";
    cout << "3. Exit\n";
    cout << "========================================\n";
    cout << "Pilihan: ";
}

void showUserMenu(User *user)
{
    system("cls"); // Gunakan "cls" jika di Windows, "clear" jika di Linux/Mac
    cout << "\n========================================\n";
    cout << "  MENU USER - " << user->getFullName() << "\n";
    cout << "========================================\n";
    cout << "1. View Profile\n";
    cout << "2. Edit Profile\n";
    cout << "3. Ganti Password\n";
    cout << "4. Book Ticket\n";
    cout << "5. View Routes\n";
    cout << "6. Logout\n";
    cout << "========================================\n";
    cout << "Pilihan: ";
}

int main()
{
    Auth auth;
    int choice;

    while (true)
    {
        if (!auth.isLoggedIn())
        {
            showMainMenu();
            cin >> choice;

            switch (choice)
            {
            case 1:
                auth.loginUser();
                break;
            case 2:
                auth.registerUser();
                break;
            case 3:
                cout << "Terima kasih telah menggunakan sistem kami!\n";
                return 0;
            default:
                cout << "Pilihan tidak valid!\n";
            }
        }
        else
        {
            User *currentUser = auth.getCurrentUser();
            showUserMenu(currentUser);
            cin >> choice;

            switch (choice)
            {
            case 1:
                auth.viewProfile();
                break;
            case 2:
                auth.editProfile();
                break;
            case 3:
                auth.changePassword();
                break;
            case 4:
                cout << "Fitur booking belum diimplementasi\n";
                break;
            case 5:
                cout << "Fitur view routes belum diimplementasi\n";
                break;
            case 6:
                auth.logout();
                break;
            default:
                cout << "Pilihan tidak valid!\n";
            }
        }
    }

    return 0;
}