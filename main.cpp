#include <iostream>
#include <string>
#include <limits>
#include "auth/user.h"
#include "auth/auth.h"
#include "json.hpp"
using json = nlohmann::json;

// Tambahkan include fitur.cpp untuk akses adminMenu() dan userMenu()
#include "fitur.cpp"
#include "feature3.cpp"
#include "fitur-jakoy.cpp"

using namespace std;

void showMainMenu()
{
    // #ifdef _WIN32
    // system("cls");
    // #else
    // system("clear");
    // #endif

    cout << "\n========================================\n";
    cout << "              Hexalingo\n";
    cout << "========================================\n";
    cout << "1. Login\n";
    cout << "2. Register\n";
    cout << "3. Exit\n";
    cout << "========================================\n";
    cout << "Pilihan: ";
}

void showWelcomeMessage(User* user)
{
    // #ifdef _WIN32
    // system("cls");
    // #else
    // system("clear");
    // #endif

    cout << "\n========================================\n";
    cout << "          Selamat Datang!\n";
    cout << "========================================\n";
    cout << "User: " << user->getFullName() << "\n";
    cout << "Role: " << user->getRole() << "\n";
    cout << "========================================\n";
    cout << "Tekan Enter untuk melanjutkan...";
    cin.ignore();
    cin.get();
}

int main()
{
    Auth auth;
    int choice;

    // Try to load session at program start
    cout << "Memuat sistem...\n";
    if (auth.loadSession())
    {
        cout << "Session ditemukan! Auto-login berhasil.\n";
        // Directly proceed to the logged-in state in the loop below
        // No need for a separate showWelcomeMessage here, it will be called in the loop
    }
    else
    {
        cout << "Tidak ada session aktif. Silakan login.\n";
    }

    while (true) // Main application loop
    {
        if (auth.isLoggedIn()) // User is logged in
        {
            User* currentUser = auth.getCurrentUser();
            showWelcomeMessage(currentUser); // Show welcome only once per login

            if (currentUser->getRole() == "admin")
            {
                cout << "Anda Login Sebagai Admin"<<endl;
                // After the menu returns, ask user what to do next
                cout << "\n========================================\n";
                cout << "Selamat data di Aplikasi Hexalingo\n";
                cout << "1. Profil \n";
                cout << "2. Matakuliah & Materi Pembelajaran \n";
                cout << "3. Tugas Menu\n";
                cout << "4. Logout\n";
                cout << "5. Exit (session tetap aktif)\n";
                cout << "========================================\n";
                cout << "Pilihan: ";

                int postMenuChoice;
                cin >> postMenuChoice;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                switch (postMenuChoice)
                {
                    case 1:
                        mainProfile();
                        break; 
                    case 2:
                        adminMenu();
                        break;
                    case 3:
                        mainKuis();
                        return 0; 
                    case 4:
                        auth.logout();
                        return 0; // Exit the program
                    case 5:
                        return 0; // Exit the program
                    default:
                        cout << "Pilihan tidak valid. Logout otomatis...\n";
                        auth.logout();
                        break;
                }
            }
            else
            {
                cout << "Anda Login Sebagai User"<<endl;
                cout << "\n========================================\n";
                cout << "Selamat data di Aplikasi Hexalingo\n";
                cout << "1. Profil \n";
                cout << "2. Matakuliah & Materi Pembelajaran \n";
                cout << "3. Logout\n";
                cout << "4. Tugas Menu\n";
                cout << "5. Exit (session tetap aktif)\n";
                cout << "========================================\n";
                cout << "Pilihan: ";

                int postMenuChoice;
                cin >> postMenuChoice;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                switch (postMenuChoice)
                {
                    case 1:
                        mainProfile();
                        break; 
                    case 2:
                        userMenu();
                        break;
                    case 3:
                        auth.logout();
                        return 0; // Exit the program
                    case 4:
                        mainKuis();
                        return 0;
                    case 5:
                        return 0; // Exit the program
                    default:
                        cout << "Pilihan tidak valid. Logout otomatis...\n";
                        auth.logout();
                        break;
                }
                // userMenu(); // Call user menu
            }

            
        }
        else // User is NOT logged in
        {
            showMainMenu(); // Show Login/Register/Exit
            cin >> choice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            switch (choice)
            {
                case 1:
                    auth.loginUser();
                    // If login is successful, the next loop iteration will hit isLoggedIn() == true
                    break;
                case 2:
                    auth.registerUser();
                    // If registration is successful, it auto-logs in, so next loop will hit isLoggedIn() == true
                    break;
                case 3:
                    cout << "Terima kasih telah menggunakan sistem kami!\n";
                    return 0; // Exit the program
                default:
                    cout << "Pilihan tidak valid!\n";
                    break;
            }
        }
    } // End of while(true) loop

    return 0; // Should ideally be unreachable
}