#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>

using namespace std;

struct Materi {
    string judul;
    vector<Materi> subMateri;
};

// Menyimpan semua Mata Pelajaran sebagai root nodes
vector<Materi> rootMateriList;

// Membersihkan layar
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Input validasi 'y' atau 'n'
char inputYaTidak(const string& prompt) {
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

// Fungsi rekursif untuk menambah materi/submateri
void tambahMateri(Materi& materi, int level = 1) {
    string indent(level * 2, ' ');
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
            tambahMateri(sub, level + 1);
            materi.subMateri.push_back(sub);
        } else {
            break;
        }
    }
}

// Fungsi menambah Materi baru yang adalah Mata Pelajaran (level 0)
void tambahMateriKeRoot() {
    Materi mataPelajaranBaru;
    clearScreen();
    cout << "=== Input Materi Baru ===" << endl;
    cout << "Masukkan Judul Mata Pelajaran (Level 0): ";
    getline(cin, mataPelajaranBaru.judul);
    while (mataPelajaranBaru.judul.empty()) {
        cout << "Judul tidak boleh kosong, harap masukkan ulang: ";
        getline(cin, mataPelajaranBaru.judul);
    }
    // Tambahkan submateri secara rekursif mulai level 1
    while (true) {
        char jawaban = inputYaTidak("Apakah Anda ingin menambahkan materi di bawah \"" + mataPelajaranBaru.judul + "\"?");
        if (jawaban == 'y') {
            Materi sub;
            tambahMateri(sub, 1);
            mataPelajaranBaru.subMateri.push_back(sub);
        } else {
            break;
        }
    }
    rootMateriList.push_back(mataPelajaranBaru);
    cout << "\nMateri berhasil ditambahkan.\n";
    cout << "Tekan Enter untuk melanjutkan...";
    string dummy;
    getline(cin, dummy);
}

// Fungsi tampilkan dengan format tree modern
void tampilkanMateriFormatBaru(const Materi& materi, int level = 0, const string& prefix = "") {
    string indentUnit = "    ";
    string batangVertikal = "|";
    string batangCabang = "+--";
    string batangSubCabang = "|--";

    string label;
    if (level == 0) {
        label = "[Mata Pelajaran] ";
        cout << label << materi.judul << endl;
    } else if (level == 1) {
        label = "[Materi] ";
        cout << prefix << batangCabang << " " << label << materi.judul << endl;
    } else if (level == 2) {
        label = "[Submateri] ";
        cout << prefix << batangVertikal << batangSubCabang << " " << label << materi.judul << endl;
    } else {
        // Level lebih dalam bisa pakai indent biasa
        label = "";
        cout << prefix << indentUnit << label << materi.judul << endl;
    }

    for (size_t i = 0; i < materi.subMateri.size(); ++i) {
        const Materi& sub = materi.subMateri[i];
        string newPrefix = prefix;
        if (level == 0) {
            if (i != materi.subMateri.size() - 1)
                newPrefix = batangVertikal + indentUnit.substr(1);
            else
                newPrefix = indentUnit;
        } else if (level == 1) {
            if (i != materi.subMateri.size() - 1)
                newPrefix = prefix + batangVertikal + indentUnit.substr(1);
            else
                newPrefix = prefix + indentUnit;
        } else {
            newPrefix = prefix + indentUnit;
        }
        tampilkanMateriFormatBaru(sub, level + 1, newPrefix);
    }
}

// Tampilkan Semua Materi (menampilkan semua Mata Pelajaran)
void tampilkanSemuaMateri() {
    clearScreen();
    cout << "=== Daftar Semua Mata Pelajaran ===\n";
    if (rootMateriList.empty()) {
        cout << "Belum ada materi yang dimasukkan.\n";
    } else {
        for (size_t i = 0; i < rootMateriList.size(); ++i) {
            tampilkanMateriFormatBaru(rootMateriList[i]);
            cout << "\n";
        }
    }
    cout << "-----------------------------------\n";
    cout << "Tekan Enter untuk melanjutkan...";
    string dummy;
    getline(cin, dummy);
}

// Fungsi rekursif edit materi tiap level
bool editMateriRekursif(Materi& materi, const string& judulLama) {
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
    for (auto& sub : materi.subMateri) {
        if (editMateriRekursif(sub, judulLama)) {
            return true;
        }
    }
    return false;
}

// Fungsi hapus materi rekursif
bool hapusMateriRekursif(Materi& parent, const string& judul) {
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

bool hapusMateriRootList(const string& judul) {
    for (size_t i = 0; i < rootMateriList.size(); i++) {
        if (rootMateriList[i].judul == judul) {
            rootMateriList.erase(rootMateriList.begin() + i);
            cout << "Materi root \"" << judul << "\" berhasil dihapus.\n";
            return true;
        }
    }
    return false;
}

// Menu Admin
void adminMenu() {
    char pilihan;
    do {
        clearScreen();
        cout << "\n=== Menu Admin ===\n";
        cout << "1. Tambah Materi\n";
        cout << "2. Tampilkan Materi\n";
        cout << "3. Edit Materi\n";
        cout << "4. Hapus Materi\n";
        cout << "5. Keluar\n";
        cout << "Pilih: ";
        string line;
        getline(cin, line);
        if (line.empty()) continue;
        pilihan = line[0];

        switch (pilihan) {
            case '1':
                tambahMateriKeRoot();
                break;
            case '2':
                tampilkanSemuaMateri();
                break;
            case '3': {
                tampilkanSemuaMateri();
                cout << "Masukkan judul materi yang ingin diedit: ";
                string judulLama;
                getline(cin, judulLama);
                bool found = false;
                for (auto& materi : rootMateriList) {
                    if (editMateriRekursif(materi, judulLama)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    cout << "Materi tidak ditemukan.\n";
                }
                cout << "Tekan Enter untuk kembali...";
                getline(cin, line);
                break;
            }
            case '4': {
                tampilkanSemuaMateri();
                cout << "Masukkan judul materi yang ingin dihapus: ";
                string judul;
                getline(cin, judul);
                bool deleted = false;
                // Cek hapus di root list dulu
                if (hapusMateriRootList(judul)) {
                    deleted = true;
                } else {
                    // Hapus rekursif dari semua subtree
                    for (auto& materi : rootMateriList) {
                        if (hapusMateriRekursif(materi, judul)) {
                            deleted = true;
                            break;
                        }
                    }
                }
                if (!deleted) {
                    cout << "Materi tidak ditemukan.\n";
                }
                cout << "Tekan Enter untuk kembali...";
                getline(cin, line);
                break;
            }
            case '5':
                cout << "Keluar dari menu admin.\n";
                break;
            default:
                cout << "Pilihan tidak valid! Silakan coba lagi.\n";
                cout << "Tekan Enter untuk melanjutkan...";
                getline(cin, line);
        }
    } while (pilihan != '5');
}

// Menu User
void userMenu() {
    clearScreen();
    cout << "\n=== Menu User ===\n";
    if (rootMateriList.empty()) {
        cout << "Belum ada materi untuk ditampilkan.\n";
    } else {
        cout << "Daftar Materi:\n";
        for (const auto& materi : rootMateriList) {
            tampilkanMateriFormatBaru(materi);
            cout << "\n";
        }
    }
    cout << "Tekan Enter untuk kembali...";
    string dummy;
    getline(cin, dummy);
}


