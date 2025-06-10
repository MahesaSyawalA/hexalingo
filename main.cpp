#include <iostream>
#include <string>
#include <limits>
#include "auth/user.h"
#include "auth/auth.h"
#include "json.hpp"
using json = nlohmann::json;

// Tambahkan include fitur.cpp untuk akses adminMenu() dan userMenu()
#include "fitur.cpp"

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
                adminMenu(); // Call admin menu
            }
            else
            {
                userMenu(); // Call user menu
            }

            // After the menu returns, ask user what to do next
            cout << "\n========================================\n";
            cout << "Apa yang ingin Anda lakukan?\n";
            cout << "1. Kembali ke menu utama (tetap login)\n"; // This option will now take you back to the main login/register menu
            cout << "2. Logout\n";
            cout << "3. Exit (session tetap aktif)\n";
            cout << "========================================\n";
            cout << "Pilihan: ";

            int postMenuChoice;
            cin >> postMenuChoice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            switch (postMenuChoice)
            {
                case 1:
                    // Choosing 1 means "return to main menu (logged out state for this loop)"
                    // To do this, we need to explicitly logout so the main loop
                    // will then show the login/register menu.
                    // If you truly want to "stay logged in" and go back to *your specific role menu*,
                    // you would need a different flow or a sub-loop within adminMenu/userMenu.
                    // For now, let's assume "Kembali ke menu utama" means going back to the login/register screen.
                    auth.logout(); // Force logout to go back to the main (login/register) menu
                    cout << "Anda telah logout untuk kembali ke menu utama. Silakan login lagi jika ingin mengakses fitur.\n";
                    break; 
                case 2:
                    auth.logout();
                    cout << "Anda telah logout. Kembali ke menu utama...\n";
                    break;
                case 3:
                    cout << "Session tetap aktif. Sampai jumpa!\n";
                    return 0; // Exit the program
                default:
                    cout << "Pilihan tidak valid. Logout otomatis...\n";
                    auth.logout();
                    break;
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