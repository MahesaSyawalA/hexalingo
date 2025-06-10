#include <iostream>
#include <string>
#include "auth/user.h"
#include "auth/auth.h"

// Tambahkan include fitur.cpp untuk akses adminMenu() dan userMenu()
#include "fitur.cpp"

using namespace std;

void showMainMenu()
{
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif

    cout << "\n========================================\n";
    cout << "              Hexalingo\n";
    cout << "========================================\n";
    cout << "1. Login\n";
    cout << "2. Register\n";
    cout << "3. Exit\n";
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
            cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Hapus sisa input agar getline bisa bekerja nanti

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
                break;
            }
        }
        else
        {
            // Setelah login berhasil, cek role user
            User* currentUser = auth.getCurrentUser();

            if (currentUser->getRole() == "admin")
            {
                adminMenu(); // Panggil menu admin dari fitur.cpp
            }
            else
            {
                userMenu(); // Panggil menu user dari fitur.cpp
            }

            // Setelah keluar dari menu, logout user supaya bisa login ulang atau register baru
            auth.logout();
        }
    }

    return 0;
}
