#include "fitur.cpp"

int main() {
    Materi rootMateri; // Root materi untuk admin
    char pilihan;

    do {
        cout << "=== Menu Utama ===" << endl;
        cout << "1. Masuk sebagai Admin" << endl;
        cout << "2. Masuk sebagai User" << endl;
        cout << "3. Keluar" << endl;
        cout << "Pilih: ";
        cin >> pilihan;

        switch (pilihan) {
            case '1':
                adminMenu(); // Menu untuk admin
                break;
            case '2':
                userMenu(); // Menu untuk user
                break;
            case '3':
                cout << "Terima kasih! Selamat belajar." << endl;
                break;
            default:
                cout << "Pilihan tidak valid!" << endl;
        }
    } while (pilihan != '3');

    return 0;
}
