#include <iostream>
#include <limits>  // Tambahkan header ini
#include <cstdlib>  // untuk system
#include "fitur.cpp"

int main() {
    char pilihan;

    do {
        clearScreen();  // Clear screen before main menu print

        std::cout << "=== Menu Utama ===" << std::endl;
        std::cout << "1. Masuk sebagai Admin" << std::endl;
        std::cout << "2. Masuk sebagai User" << std::endl;
        std::cout << "3. Keluar" << std::endl;
        std::cout << "Pilih: ";
        std::cin >> pilihan;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Perbaiki baris ini

        switch (pilihan) {
            case '1':
                adminMenu(); // Menu untuk admin
                break;
            case '2':
                userMenu(); // Menu untuk user
                break;
            case '3':
                std::cout << "Terima kasih! Selamat belajar." << std::endl;
                break;
            default:
                std::cout << "Pilihan tidak valid!" << std::endl;
                std::cout << "Tekan Enter untuk melanjutkan...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    } while (pilihan != '3');

    return 0;
}
