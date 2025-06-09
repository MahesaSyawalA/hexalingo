#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

struct Materi {
    string judul;
    vector<Materi> subMateri;
};

// Data utama, simpan semua Mata Pelajaran (root nodes)
vector<Materi> rootMateriList;

// -------- Fungsi Utility untuk sistem --------
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

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

// -------- Fungsi Konversi Materi <-> JSON --------

// Materi ke json secara rekursif
json materiToJson(const Materi& materi, int level = 0) {
    json j;
    if(level == 0) {
        // Root Materi (Mata Pelajaran)
        j["judul"] = materi.judul;
        j["materi"] = json::array();
        for (const auto& sub : materi.subMateri) {
            j["materi"].push_back(materiToJson(sub, level + 1));
        }
    } else if(level == 1) {
        // Materi level 1
        j["judul"] = materi.judul;
        j["submateri"] = json::array();
        for (const auto& sub : materi.subMateri) {
            j["submateri"].push_back(materiToJson(sub, level + 1));
        }
    } else {
        // Level 2+ submateri tanpa kunci lebih dalam
        j["judul"] = materi.judul;
        // Bisa diteruskan jika struktur lebih dalem, saat ini kosong
    }
    return j;
}

// JSON ke Materi secara rekursif
Materi jsonToMateri(const json& j, int level = 0) {
    Materi m;
    if(level == 0) {
        m.judul = j.value("judul", "");
        if (j.contains("materi") && j["materi"].is_array()) {
            for (const auto& sub : j["materi"]) {
                m.subMateri.push_back(jsonToMateri(sub, level + 1));
            }
        }
    } else if(level == 1) {
        m.judul = j.value("judul", "");
        if (j.contains("submateri") && j["submateri"].is_array()) {
            for (const auto& sub : j["submateri"]) {
                m.subMateri.push_back(jsonToMateri(sub, level + 1));
            }
        }
    } else {
        m.judul = j.value("judul", "");
        // Jika ada subsubmateri bisa diteruskan di sini
    }
    return m;
}

// -------- Fungsi Load/Save dari file --------

const string DATABASE_FILENAME = "database.json";

void saveToJsonFile() {
    json j;
    j["daftar_mata_pelajaran"] = json::array();
    for (const auto& mp : rootMateriList) {
        j["daftar_mata_pelajaran"].push_back(materiToJson(mp));
    }
    try {
        ofstream file(DATABASE_FILENAME);
        if (!file) {
            cerr << "Gagal membuka file untuk penulisan: " << DATABASE_FILENAME << endl;
            return;
        }
        file << j.dump(4) << endl; // Indentasi 4 spasi
        file.close();
    } catch (const exception& e) {
        cerr << "Kesalahan saat menyimpan file JSON: " << e.what() << endl;
    }
}

void loadFromJsonFile() {
    ifstream file(DATABASE_FILENAME);
    if (!file) {
        // File mungkin belum ada, ignore dan start fresh
        return;
    }
    try {
        json j;
        file >> j;
        file.close();
        if (j.contains("daftar_mata_pelajaran") && j["daftar_mata_pelajaran"].is_array()) {
            rootMateriList.clear();
            for (const auto& mpJson : j["daftar_mata_pelajaran"]) {
                rootMateriList.push_back(jsonToMateri(mpJson));
            }
        }
    } catch (const exception& e) {
        cerr << "Kesalahan saat membaca file JSON: " << e.what() << endl;
        rootMateriList.clear();
    }
}

// -------- Fungsi Input Materi/Submateri --------

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

    saveToJsonFile(); // simpan setiap tambah

    cout << "\nMateri berhasil ditambahkan.\n";
    cout << "Tekan Enter untuk melanjutkan...";
    string dummy;
    getline(cin, dummy);
}

// -------- Fungsi Tampil Data --------

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

void tampilkanSemuaMateri() {
    clearScreen();
    cout << "=== Daftar Semua Mata Pelajaran ===\n";
    if (rootMateriList.empty()) {
        cout << "Belum ada materi yang dimasukkan.\n";
    } else {
        for (const auto& materi : rootMateriList) {
            tampilkanMateriFormatBaru(materi);
            cout << "\n";
        }
    }
    cout << "-----------------------------------\n";
    cout << "Tekan Enter untuk melanjutkan...";
    string dummy;
    getline(cin, dummy);
}

// -------- Fungsi Edit Materi --------

bool editMateriRekursif(Materi& materi, const string& judulLama) {
    if (materi.judul == judulLama) {
        cout << "Judul sekarang: " << materi.judul << endl;
        cout << "Masukkan judul baru (kosongkan untuk batal): ";
        string newJudul;
        getline(cin, newJudul);
        if (!newJudul.empty()) {
            materi.judul = newJudul;
            saveToJsonFile(); // simpan saat edit
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

// -------- Fungsi Hapus Materi --------

bool hapusMateriRekursif(Materi& parent, const string& judul) {
    for (size_t i = 0; i < parent.subMateri.size(); i++) {
        if (parent.subMateri[i].judul == judul) {
            parent.subMateri.erase(parent.subMateri.begin() + i);
            saveToJsonFile(); // simpan saat hapus
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
            saveToJsonFile(); // simpan saat hapus
            cout << "Materi root \"" << judul << "\" berhasil dihapus.\n";
            return true;
        }
    }
    return false;
}

// -------- Menu Admin dan User --------

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
                if (hapusMateriRootList(judul)) {
                    deleted = true;
                } else {
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


