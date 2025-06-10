#include <iostream>
#include <string>
#include <limits>
#include <stack>       // For history stack
#include "auth/user.h"
#include "auth/auth.h"
#include "json.hpp"
using json = nlohmann::json;

// Tambahkan include fitur.cpp untuk akses adminMenu() dan userMenu()
#include "fitur.cpp"
#include "fitur-jakoy.cpp"

using namespace std;

// ==== MULAI PENAMBAHAN FITUR HISTORI ====
// Stack untuk menyimpan histori navigasi user
stack<string> userHistory;

// Fungsi menambah histori pada stack
void tambahHistori(const string& item) {
    userHistory.push(item);
}

// Fungsi menampilkan histori navigasi user
void lihatHistori() {
    if (userHistory.empty()) {
        cout << "\n=== HISTORI KOSONG ===\n";
        cout << "Belum ada aktivitas yang tercatat.\n";
    } else {
        cout << "\n=== HISTORI AKTIVITAS ===\n";
        // Buat stack sementara untuk menampilkan histori dari yang terlama ke terbaru
        stack<string> tempStack = userHistory;
        // Gunakan vector untuk menyimpan karena stack aksesnya hanya top
        vector<string> tempVec;
        while (!tempStack.empty()) {
            tempVec.push_back(tempStack.top());
            tempStack.pop();
        }
        // Tampilkan dari yang paling awal (indeks 1)
        for (int i = (int)tempVec.size() - 1; i >= 0; i--) {
            cout << (tempVec.size() - i) << ". " << tempVec[i] << "\n";
        }
    }
    cout << "=====================\n";
    cout << "Tekan Enter untuk kembali...";
    cin.ignore();
    cin.get();
}
// ==== AKHIR PENAMBAHAN FITUR HISTORI ====


void showMainMenu()
{
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

    cout << "Memuat sistem...\n";
    if (auth.loadSession())
    {
        cout << "Session ditemukan! Auto-login berhasil.\n";
    }
    else
    {
        cout << "Tidak ada session aktif. Silakan login.\n";
    }

    while (true)
    {
        if (auth.isLoggedIn())
        {
            User* currentUser = auth.getCurrentUser();
            showWelcomeMessage(currentUser);

            if (currentUser->getRole() == "admin")
            {
                cout << "Anda Login Sebagai Admin"<<endl;
                cout << "\n========================================\n";
                cout << "Selamat data di Aplikasi Hexalingo\n";
                cout << "1. Profil \n";
                cout << "2. Matakuliah & Materi Pembelajaran \n";
                cout << "3. Logout\n";
                cout << "4. Exit (session tetap aktif)\n";
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
                        auth.logout();
                        return 0;
                    case 4:
                        return 0;
                    default:
                        cout << "Pilihan tidak valid. Logout otomatis...\n";
                        auth.logout();
                        break;
                }
            }
            else // Role user
            {
                cout << "Anda Login Sebagai User"<<endl;
                cout << "\n========================================\n";
                cout << "Selamat data di Aplikasi Hexalingo\n";
                cout << "1. Profil \n";
                cout << "2. Matakuliah & Materi Pembelajaran \n";
                // ==== TAMBAHAN MENU LIHAT HISTORI ====
                cout << "3. Lihat Histori\n";
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
                        tambahHistori("Melihat Profil");
                        mainProfile();
                        break;
                    case 2:
                        tambahHistori("Melihat Matakuliah & Materi Pembelajaran");
                        userMenu();
                        break;
                    case 3:
                        tambahHistori("Melihat Histori Aktivitas");
                        lihatHistori();
                        break;
                    case 4:
                        auth.logout();
                        return 0;
                    case 5:
                        return 0;
                    default:
                        cout << "Pilihan tidak valid. Logout otomatis...\n";
                        auth.logout();
                        break;
                }
                // userMenu();
            }
        }
        else
        {
            showMainMenu();
            cin >> choice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

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
    }

    return 0;
}

