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

vector<Materi> rootMateriList;

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
        if (line.empty()) continue;
        char ch = tolower(line[0]);
        if (ch == 'y' || ch == 'n') return ch;
        cout << "Input tidak valid.\n";
    }
}

// ---------- Konversi JSON ----------
json materiToJson(const Materi& materi, int level = 0) {
    json j;
    j["judul"] = materi.judul;

    if (level == 0)
        j["materi"] = json::array();
    else if (level == 1)
        j["submateri"] = json::array();

    for (const auto& sub : materi.subMateri) {
        if (level == 0)
            j["materi"].push_back(materiToJson(sub, level + 1));
        else if (level == 1)
            j["submateri"].push_back(materiToJson(sub, level + 1));
    }

    return j;
}

Materi jsonToMateri(const json& j, int level = 0) {
    Materi m;
    m.judul = j.value("judul", "");

    if (level == 0 && j.contains("materi")) {
        for (const auto& sub : j["materi"])
            m.subMateri.push_back(jsonToMateri(sub, level + 1));
    } else if (level == 1 && j.contains("submateri")) {
        for (const auto& sub : j["submateri"])
            m.subMateri.push_back(jsonToMateri(sub, level + 1));
    }

    return m;
}

// ---------- Load / Save ----------
const string DATABASE_FILENAME = "database.json";

void saveToJsonFile(const string& filePath) {
    // Pertama load semua data yang ada
    json root;
    ifstream inFile(filePath);
    if (inFile.is_open()) {
        inFile >> root;
        inFile.close();
    }

    // Update hanya bagian daftar_mata_pelajaran
    json daftar_mata_pelajaran = json::array();
    for (const auto& materi : rootMateriList)
        daftar_mata_pelajaran.push_back(materiToJson(materi));
    
    root["daftar_mata_pelajaran"] = daftar_mata_pelajaran;

    // Simpan seluruh data kembali
    ofstream outFile(filePath);
    outFile << setw(4) << root << endl;
    outFile.close();
}

void loadFromJsonFile(const string& filePath) {
    ifstream inFile(filePath);
    if (!inFile.is_open()) return;

    json data;
    inFile >> data;
    inFile.close();

    rootMateriList.clear();
    if (data.contains("daftar_mata_pelajaran")) {
        for (const auto& item : data["daftar_mata_pelajaran"])
            rootMateriList.push_back(jsonToMateri(item));
    }
}

// ---------- Tambah Materi ----------
void tambahMateri(Materi& materi, int level = 1) {
    string indent(level * 2, ' ');
    cout << indent << "Masukkan Judul Materi (Level " << level << "): ";
    getline(cin, materi.judul);
    while (materi.judul.empty()) {
        cout << indent << "Judul tidak boleh kosong, ulangi: ";
        getline(cin, materi.judul);
    }

    while (inputYaTidak(indent + "Tambah sub-materi \"" + materi.judul + "\"?") == 'y') {
        Materi sub;
        tambahMateri(sub, level + 1);
        materi.subMateri.push_back(sub);
    }
}

void tambahMateriKeRoot() {
    loadFromJsonFile(DATABASE_FILENAME);

    Materi baru;
    clearScreen();
    cout << "=== Tambah Mata Pelajaran ===\n";
    cout << "Masukkan Judul: ";
    getline(cin, baru.judul);

    while (baru.judul.empty()) {
        cout << "Judul tidak boleh kosong, ulangi: ";
        getline(cin, baru.judul);
    }

    while (inputYaTidak("Tambah materi ke \"" + baru.judul + "\"?") == 'y') {
        Materi sub;
        tambahMateri(sub, 1);
        baru.subMateri.push_back(sub);
    }

    rootMateriList.push_back(baru);
    saveToJsonFile(DATABASE_FILENAME);
    cout << "Materi berhasil ditambahkan.\nTekan Enter...";
    string dummy;
    getline(cin, dummy);
}

// ---------- Tampilkan Materi ----------
void tampilkanMateriFormatBaru(const Materi& materi, int level = 0, const string& prefix = "") {
    string indentUnit = "    ";
    string batangVertikal = "|";
    string batangCabang = "+--";
    string batangSubCabang = "|--";

    if (level == 0)
        cout << "[Mata Pelajaran] " << materi.judul << endl;
    else if (level == 1)
        cout << prefix << batangCabang << " [Materi] " << materi.judul << endl;
    else
        cout << prefix << batangVertikal << batangSubCabang << " [Submateri] " << materi.judul << endl;

    for (size_t i = 0; i < materi.subMateri.size(); ++i) {
        string newPrefix = prefix + indentUnit;
        tampilkanMateriFormatBaru(materi.subMateri[i], level + 1, newPrefix);
    }
}

void tampilkanSemuaMateri() {
    loadFromJsonFile(DATABASE_FILENAME);
    clearScreen();
    cout << "=== Daftar Semua Materi ===\n";
    if (rootMateriList.empty()) {
        cout << "Belum ada materi.\n";
    } else {
        for (const auto& materi : rootMateriList) {
            tampilkanMateriFormatBaru(materi);
            cout << "\n";
        }
    }
    cout << "Tekan Enter...";
    string dummy;
    getline(cin, dummy);
}

// ---------- Edit Materi ----------
bool editMateriRekursif(Materi& materi, const string& judulLama) {
    if (materi.judul == judulLama) {
        cout << "Judul sekarang: " << materi.judul << "\nMasukkan judul baru: ";
        string baru;
        getline(cin, baru);
        if (!baru.empty()) {
            materi.judul = baru;
            saveToJsonFile(DATABASE_FILENAME);
            cout << "Judul diubah.\n";
        }
        return true;
    }
    for (auto& sub : materi.subMateri)
        if (editMateriRekursif(sub, judulLama)) return true;

    return false;
}

// ---------- Hapus Materi ----------
bool hapusMateriRekursif(Materi& parent, const string& judul) {
    for (size_t i = 0; i < parent.subMateri.size(); i++) {
        if (parent.subMateri[i].judul == judul) {
            parent.subMateri.erase(parent.subMateri.begin() + i);
            saveToJsonFile(DATABASE_FILENAME);
            return true;
        }
        if (hapusMateriRekursif(parent.subMateri[i], judul)) return true;
    }
    return false;
}

bool hapusMateriRootList(const string& judul) {
    for (size_t i = 0; i < rootMateriList.size(); i++) {
        if (rootMateriList[i].judul == judul) {
            rootMateriList.erase(rootMateriList.begin() + i);
            saveToJsonFile(DATABASE_FILENAME);
            return true;
        }
    }
    return false;
}

// ---------- Menu Utama ----------
void adminMenu() {
    char pilihan;
    do {
        clearScreen();
        cout << "=== Menu Admin ===\n"
             << "1. Tambah Materi\n"
             << "2. Tampilkan Materi\n"
             << "3. Edit Materi\n"
             << "4. Hapus Materi\n"
             << "5. Keluar\nPilih: ";
        string input;
        getline(cin, input);
        pilihan = input.empty() ? ' ' : input[0];

        switch (pilihan) {
            case '1': tambahMateriKeRoot(); break;
            case '2': tampilkanSemuaMateri(); break;
            case '3': {
                tampilkanSemuaMateri();
                cout << "Judul materi yang ingin diedit: ";
                string target;
                getline(cin, target);
                bool found = false;
                for (auto& m : rootMateriList)
                    if (editMateriRekursif(m, target)) { found = true; break; }
                if (!found) cout << "Materi tidak ditemukan.\n";
                cout << "Tekan Enter..."; getline(cin, input);
                break;
            }
            case '4': {
                tampilkanSemuaMateri();
                cout << "Judul materi yang ingin dihapus: ";
                string target;
                getline(cin, target);
                bool deleted = hapusMateriRootList(target);
                if (!deleted)
                    for (auto& m : rootMateriList)
                        if (hapusMateriRekursif(m, target)) { deleted = true; break; }
                if (!deleted) cout << "Materi tidak ditemukan.\n";
                cout << "Tekan Enter..."; getline(cin, input);
                break;
            }
        }
    } while (pilihan != '5');
}

void userMenu() {
    clearScreen();
    cout << "=== Menu User ===\n";
    tampilkanSemuaMateri();
}

int mainMateri() {
    loadFromJsonFile(DATABASE_FILENAME);
    char mode;
    do {
        clearScreen();
        cout << "=== Menu Utama ===\n1. Admin\n2. User\n3. Keluar\nPilih: ";
        string pilih;
        getline(cin, pilih);
        mode = pilih.empty() ? ' ' : pilih[0];

        if (mode == '1') adminMenu();
        else if (mode == '2') userMenu();
    } while (mode != '3');

    return 0;
}