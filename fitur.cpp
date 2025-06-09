#include <iostream>
#include <string>
#include <vector>
#include <cctype> // untuk fungsi tolower
#include <cstdlib> // untuk fungsi system

using namespace std;

struct Materi {
    string judul;
    vector<Materi> subMateri; // Menyimpan sub-materi
};

// Global root materi yang dapat diakses oleh admin dan user
Materi rootMateri;

// Fungsi untuk membersihkan layar
void clearScreen() {
#ifdef _WIN32
    system("cls"); // Untuk Windows
#else
    system("clear"); // Untuk Linux/Mac
#endif
}

// Fungsi untuk membaca input 'y' atau 'n' dengan validasi
char inputYaTidak(const string &prompt) {
    string line;
    while (true) {
        cout << prompt << " (y/n): ";
        getline(cin, line);
        if (line.empty()) {
            cout << "Input kosong, silakan masukkan 'y' atau 'n'." << endl;
            continue;
        }
        char ch = tolower(line[0]);
        if (ch == 'y' || ch == 'n') {
            return ch;
        }
        cout << "Input tidak valid, silakan masukkan 'y' atau 'n'." << endl;
    }
}

// Fungsi untuk menambahkan materi dengan input yang jelas
void tambahMateri(Materi &materi, int level = 0) {
    if (level == 0) {
        clearScreen(); // Hanya sekali di level 0
    }
    string indent(level * 2, ' '); // spasi indentasi sesuai level
    cout << indent << "Masukkan Judul Materi (Level " << level << "): ";
    getline(cin, materi.judul);
    while (materi.judul.empty()) {
        cout << indent << "Judul tidak boleh kosong, harap masukkan ulang: ";
        getline(cin, materi.judul);
    }
    while (true) {
        char jawaban = inputYaTidak(indent + "Apakah Anda ingin menambahkan sub-materi untuk \"" + materi.judul + "\"?");
        if (jawaban == 'y') {
            Materi sub;
            tambahMateri(sub, level + 1); // rekursif tanpa clear screen lagi
            materi.subMateri.push_back(sub);
        } else {
            break; // keluar dari loop jika tidak mau menambahkan sub materi
        }
    }
}

// Fungsi untuk menampilkan materi
void tampilkanMateri(const Materi &materi, int level = 0) {
    for (int i = 0; i < level; i++) {
        cout << "  "; // Indentasi untuk level
    }
    cout << "- " << materi.judul << endl;

    for (const auto &sub : materi.subMateri) {
        tampilkanMateri(sub, level + 1); // Rekursif untuk menampilkan sub-materi
    }
}

// Fungsi untuk menampilkan materi lengkap sebelum edit/hapus
void tampilkanMateriUntukAdmin(const Materi &materi) {
    clearScreen();
    cout << "=== Daftar Materi Saat Ini ===" << endl;
    if (materi.judul.empty()) {
        cout << "Belum ada materi yang dimasukkan." << endl;
    } else {
        tampilkanMateri(materi);
    }
    cout << "Tekan Enter untuk melanjutkan...";
    string dummy;
    getline(cin, dummy);
}


// Fungsi untuk mengedit materi secara rekursif
bool editMateriRekursif(Materi &materi, const string &judulLama) {
    if (materi.judul == judulLama) {
        cout << "Judul sekarang: " << materi.judul << endl;
        cout << "Masukkan judul baru (kosongkan untuk batal): ";
        string newJudul;
        getline(cin, newJudul);
        if (!newJudul.empty()) {
            materi.judul = newJudul;
            cout << "Judul berhasil diubah.\n";
        } else {
            cout << "Edit dibatalkan.\n";
        }
        return true;
    }
    for (auto &sub : materi.subMateri) {
        if (editMateriRekursif(sub, judulLama)) {
            return true;
        }
    }
    return false;
}

// Fungsi untuk menghapus materi secara rekursif
bool hapusMateriRekursif(Materi &parent, const string &judul) {
    for (size_t i = 0; i < parent.subMateri.size(); i++) {
        if (parent.subMateri[i].judul == judul) {
            parent.subMateri.erase(parent.subMateri.begin() + i);
            cout << "Materi \"" << judul << "\" berhasil dihapus.\n";
            return true;
        }
        if (hapusMateriRekursif(parent.subMateri[i], judul)) {
            return true;
        }
    }
    return false;
}

// Fungsi untuk menu admin
void adminMenu() {
    char pilihan;
    do {
        clearScreen();
        cout << "\n=== Menu Admin ===" << endl;
        cout << "1. Tambah Materi" << endl;
        cout << "2. Tampilkan Materi" << endl;
        cout << "3. Edit Materi" << endl;
        cout << "4. Hapus Materi" << endl;
        cout << "5. Keluar" << endl;
        cout << "Pilih: ";
        string line;
        getline(cin, line);
        if (line.empty()) {
            cout << "Pilihan tidak boleh kosong, silakan coba lagi." << endl;
            continue;
        }
        pilihan = line[0];
        switch (pilihan) {
            case '1':
                tambahMateri(rootMateri);
                break;
            case '2':
                cout << "\nDaftar Materi:" << endl;
                if (rootMateri.judul.empty()) {
                    cout << "Belum ada materi yang dimasukkan." << endl;
                } else {
                    tampilkanMateri(rootMateri);
                }
                cout << "Tekan Enter untuk kembali...";
                getline(cin, line);
                break;
            case '3': {
                tampilkanMateriUntukAdmin(rootMateri); // Menampilkan materi dulu sebelum edit
                cout << "Masukkan judul materi yang ingin diedit: ";
                string judulLama;
                getline(cin, judulLama);
                if (!editMateriRekursif(rootMateri, judulLama)) {
                    cout << "Materi tidak ditemukan.\n";
                }
                cout << "Tekan Enter untuk kembali...";
                getline(cin, line);
                break;
            }
            case '4': {
                tampilkanMateriUntukAdmin(rootMateri); // Menampilkan materi dulu sebelum hapus
                cout << "Masukkan judul materi yang ingin dihapus: ";
                string judul;
                getline(cin, judul);
                if (!hapusMateriRekursif(rootMateri, judul)) {
                    cout << "Materi tidak ditemukan.\n";
                }
                cout << "Tekan Enter untuk kembali...";
                getline(cin, line);
                break;
            }
            case '5':
                cout << "Keluar dari menu admin." << endl;
                break;
            default:
                cout << "Pilihan tidak valid! Silakan coba lagi." << endl;
        }
    } while (pilihan != '5');
}

// Fungsi untuk menu user
void userMenu() {
    clearScreen();
    cout << "\n=== Menu User ===" << endl;
    if (rootMateri.judul.empty()) {
        cout << "Belum ada materi untuk ditampilkan." << endl;
    } else {
        cout << "Daftar Materi:" << endl;
        tampilkanMateri(rootMateri);
    }
    cout << "Tekan Enter untuk kembali...";
    string dummy;
    getline(cin, dummy);
}
