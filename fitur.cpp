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
    string id;
    string judul;
    vector<Materi> subMateri;
    string isi;
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
json materiToJson(const Materi& materi) {
    json j;
    j["id"] = materi.id;
    j["judul"] = materi.judul;

    if (!materi.isi.empty()) {
        j["isi"] = materi.isi;
    }
    if (materi.subMateri.size() > 0) {
        j["submateri"] = json::array();
        for (const auto& sub : materi.subMateri) {
            j["submateri"].push_back(materiToJson(sub));
        }
    }

    return j;
}

Materi jsonToMateri(const json& j) {
    Materi m;
    m.id = j.value("id", "");
    m.judul = j.value("judul", "");
    m.isi = j.value("isi", "");

    if (j.contains("submateri")) {
        for (const auto& sub : j["submateri"]) {
            m.subMateri.push_back(jsonToMateri(sub));
        }
    }

    return m;
}

string toLower(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return tolower(c);
    });
    return result;
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

// Fungsi untuk menghasilkan ID baru
string generateNewId(const string& parentId, int urutan) {
    if (parentId.empty()) {
        return to_string(urutan); // Mata Pelajaran
    } else {
        return parentId + "." + to_string(urutan);
    }
}

// ---------- Tambah Materi ----------
void tambahMateri(Materi& materi, const string& parentId, int urutan) {

    materi.id = generateNewId(parentId, urutan);
    cout << "Masukkan Judul Materi (ID: " << materi.id << "): ";
    getline(cin, materi.judul);
    materi.judul = toLower(materi.judul);

    int depth = count(materi.id.begin(), materi.id.end(), '.');

    if (depth == 2) {
    cout << "Masukkan isi materi untuk \"" << materi.judul << "\": ";
    getline(cin, materi.isi);
    materi.isi = toLower(materi.isi);
    if (materi.isi.empty()) {
        cout << "Isi materi tidak boleh kosong. Materi tidak ditambahkan.\n";
        return;
    }
    cout << "Materi \"" << materi.judul << "\" berhasil ditambahkan.\n";
    return;
}

    
    int subUrutan = 1;
    while (inputYaTidak("Tambah sub-materi \"" + materi.judul + "\"?") == 'y' && depth < 2) {
        Materi sub;
        tambahMateri(sub, materi.id, subUrutan++);
        materi.subMateri.push_back(sub);
    }
}

bool isDuplicateTitle(const vector<Materi>& list, const string& judulBaru) {
    string judulBaruLower = toLower(judulBaru);
    for (const auto& m : list) {
        if (toLower(m.judul) == judulBaruLower) {
            return true;
        }
    }
    return false;
}

void tambahMateriKeRoot() {
    loadFromJsonFile(DATABASE_FILENAME);

    Materi baru;
    clearScreen();
    cout << "=== Tambah Mata Pelajaran ===\n";
    int urutan = rootMateriList.size() + 1;
    baru.id = to_string(urutan);
    cout << "Masukkan Judul Mata Pelajaran: ";
    getline(cin, baru.judul);

    while (baru.judul.empty() || isDuplicateTitle(rootMateriList, baru.judul)) {
        if (baru.judul.empty()) {
            cout << "Judul tidak boleh kosong, ulangi: ";
        } else {
            cout << "Judul sudah ada, ulangi: ";
        }
        getline(cin, baru.judul);
    }

    int subUrutan = 1;
    while (inputYaTidak("Tambah materi ke \"" + baru.judul + "\"?") == 'y') {
        Materi sub;
        tambahMateri(sub, baru.id, subUrutan++);
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

    if (!materi.isi.empty()) {
        string isiIndent = prefix + indentUnit; 
        cout << isiIndent << "[Isi] " << materi.isi << endl;
    }
    for (size_t i = 0; i < materi.subMateri.size(); ++i) {
        string newPrefix = prefix + indentUnit;
        tampilkanMateriFormatBaru(materi.subMateri[i], level + 1, newPrefix);
    }
}

void searchMateri(const json& data, const string& keyword) {
    bool ditemukan = false;
    string keyword_lower = toLower(keyword);

    cout << "Hasil pencarian materi untuk judul: \"" << keyword << "\"\n\n";

    for (const auto& pelajaran : data["daftar_mata_pelajaran"]) {
        string mapel_judul = pelajaran["judul"];
        for (const auto& materi : pelajaran["materi"]) {
            string materi_judul = materi["judul"];
            string materi_judul_lower = toLower(materi_judul);

            if (materi_judul_lower == keyword_lower) {
                ditemukan = true;
                cout << "Ditemukan di mata pelajaran: " << mapel_judul << endl;
                cout << "Materi: " << materi_judul << endl;

                if (materi.contains("submateri") && !materi["submateri"].empty()) {
                    cout << "Submateri:\n";
                    for (const auto& sub : materi["submateri"]) {
                        cout << " - " << sub["judul"] << endl;
                    }
                } else {
                    cout << "(Tidak ada submateri)\n";
                }
                cout << "----------------------\n";
            }
        }
    }

    if (!ditemukan) {
        cout << "Materi dengan judul \"" << keyword << "\" tidak ditemukan.\n";
    }
    cout << "Tekan Enter...";
    string dummy;
    getline(cin, dummy);
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
             << "5. Cari Materi\n"
             << "6. Keluar\nPilih: ";
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
            case '5': {
                ifstream file("database.json");
                if (!file.is_open()) {
                    cout << "Failed to open file." << endl;
                    break;
                }

                json data;
                file >> data;

                string keyword;
                cout << "Masukkan judul materi yang ingin dicari: ";
                getline(cin, keyword); // AMAN karena sudah cin.ignore()

                searchMateri(data, keyword);
                break;
            }
        }
    } while (pilihan != '6');
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