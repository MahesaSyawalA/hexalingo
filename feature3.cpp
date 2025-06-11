#include <iostream>
#include <fstream>
#include <iomanip>
#include "json.hpp"
#include <vector>
#include <string>

using json = nlohmann::json;
using namespace std;

json db;

bool loadDatabase(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Gagal membuka file database.\n";
        return false;
    }
    file >> db;
    return true;
}

bool saveDatabase(const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Gagal menyimpan file database.\n";
        return false;
    }
    file << setw(4) << db;
    return true;
}

void printPathById(const json &materiList, const string &targetId, string currentPath = "") {
    for (const auto &materi : materiList) {
        if (!materi.contains("id")) continue;
        string id = materi["id"];
        string title = materi["judul"];
        string newPath = currentPath.empty() ? title : currentPath + " -> " + title;
        if (id == targetId) {
            cout << "Path materi: " << newPath << endl;
            return;
        }
        if (materi.contains("submateri")) {
            printPathById(materi["submateri"], targetId, newPath);
        }
    }
}

bool isValidId(const json &materiList, const string &targetId) {
    for (const auto &materi : materiList) {
        if (!materi.contains("id")) continue;
        if (materi["id"] == targetId) return true;
        if (materi.contains("submateri") && isValidId(materi["submateri"], targetId)) {
            return true;
        }
    }
    return false;
}

void tambahTugas() {
    string idMateri, deadline;
    int jumlahSoal;
    cout << "Masukkan ID materi (contoh: 4.1.1): ";
    cin >> idMateri;

    if (!isValidId(db["daftar_mata_pelajaran"], idMateri)) {
        cout << "ID materi tidak ditemukan.\n";
        return;
    }

    cin.ignore();
    cout << "Masukkan deadline (format: YYYY-MM-DD HH:MM): ";
    getline(cin, deadline);

    cout << "Masukkan jumlah soal: ";
    cin >> jumlahSoal;
    cin.ignore();

    json soalList = json::array();
    for (int i = 0; i < jumlahSoal; ++i) {
        string pertanyaan, kunci, opsi;
        vector<string> opsiList;
        cout << "Soal #" << i + 1 << endl;
        cout << "Pertanyaan: ";
        getline(cin, pertanyaan);

        cout << "Masukkan 4 opsi (pisahkan dengan enter):\n";
        for (int j = 0; j < 4; ++j) {
            cout << "Opsi " << (char)('A' + j) << ": ";
            getline(cin, opsi);
            opsiList.push_back(opsi);
        }

        cout << "Kunci jawaban (A/B/C/D): ";
        getline(cin, kunci);

        json soal = {
            {"pertanyaan", pertanyaan},
            {"opsi", opsiList},
            {"kunci", kunci}
        };
        soalList.push_back(soal);
    }

    json tugas = {
        {"id_materi", idMateri},
        {"deadline", deadline},
        {"soal", soalList}
    };

    db["tugas"].push_back(tugas);
    cout << "Tugas berhasil ditambahkan!\n";
    printPathById(db["daftar_mata_pelajaran"], idMateri);
}

void tampilkanSemuaTugas() {
    if (!db.contains("tugas") || db["tugas"].empty()) {
        cout << "Belum ada tugas.\n";
        return;
    }

    for (size_t i = 0; i < db["tugas"].size(); ++i) {
        cout << "\nTugas #" << i + 1 << endl;
        cout << "ID Materi: " << db["tugas"][i]["id_materi"] << endl;
        printPathById(db["daftar_mata_pelajaran"], db["tugas"][i]["id_materi"]);
        cout << "Deadline: " << db["tugas"][i]["deadline"] << endl;
        cout << "Jumlah Soal: " << db["tugas"][i]["soal"].size() << endl;
    }
}

void tampilkanDetailTugas() {
    if (!db.contains("tugas") || db["tugas"].empty()) {
        cout << "Belum ada tugas.\n";
        return;
    }

    for (size_t i = 0; i < db["tugas"].size(); ++i) {
        cout << "\n=== Detail Tugas #" << i + 1 << " ===\n";
        printPathById(db["daftar_mata_pelajaran"], db["tugas"][i]["id_materi"]);
        cout << "Deadline: " << db["tugas"][i]["deadline"] << "\n";
        const auto &soalList = db["tugas"][i]["soal"];
        for (size_t j = 0; j < soalList.size(); ++j) {
            cout << "\nSoal #" << j + 1 << ": " << soalList[j]["pertanyaan"] << endl;
            const auto &opsi = soalList[j]["opsi"];
            for (size_t k = 0; k < opsi.size(); ++k) {
                cout << (char)('A' + k) << ". " << opsi[k] << endl;
            }
            cout << "Kunci Jawaban: " << soalList[j]["kunci"] << endl;
        }
    }
}

void tampilkanDaftarMateri(const json &materiList, int indent = 0) {
    string spasi(indent * 2, ' ');
    for (const auto &materi : materiList) {
        if (!materi.contains("id") || !materi.contains("judul")) continue;
        cout << spasi << "[" << materi["id"] << "] " << materi["judul"] << "\n";
        if (materi.contains("submateri")) {
            tampilkanDaftarMateri(materi["submateri"], indent + 1);
        }
    }
}

void kerjakanTugas() {
    if (!db.contains("tugas") || db["tugas"].empty()) {
        cout << "Belum ada tugas yang tersedia.\n";
        return;
    }

    tampilkanSemuaTugas();  // Tampilkan daftar tugas agar user bisa memilih

    int index;
    cout << "\nPilih nomor tugas yang ingin dikerjakan: ";
    cin >> index;
    cin.ignore();

    if (index < 1 || index > db["tugas"].size()) {
        cout << "Nomor tugas tidak valid.\n";
        return;
    }

    const auto &tugas = db["tugas"][index - 1];
    const auto &soalList = tugas["soal"];

    int benar = 0;
    vector<string> jawabanUser;

    for (size_t i = 0; i < soalList.size(); ++i) {
        cout << "\nSoal #" << i + 1 << ": " << soalList[i]["pertanyaan"] << endl;
        const auto &opsi = soalList[i]["opsi"];
        for (size_t j = 0; j < opsi.size(); ++j) {
            cout << (char)('A' + j) << ". " << opsi[j] << endl;
        }

        string jawaban;
        cout << "Jawaban Anda (A/B/C/D): ";
        getline(cin, jawaban);

        if (!jawaban.empty()) {
            jawabanUser.push_back(string(1, toupper(jawaban[0])));
            if (toupper(jawaban[0]) == toupper(soalList[i]["kunci"].get<string>()[0])) {
                benar++;
            }
        } else {
            jawabanUser.push_back(" ");
        }
    }

    double nilai = (double)benar / soalList.size() * 100.0;
    cout << "\nSelesai! Nilai akhir Anda: " << fixed << setprecision(2) << nilai << " dari " << soalList.size() << " soal.\n";

    // Simpan ke database jawaban
    int user_id = db["session"]["user_id"];

    json jawabanData = {
        {"user_id", user_id},
        {"id_tugas", index - 1},  // index tugas ke berapa
        {"jawaban", jawabanUser},
        {"nilai", nilai}
    };

    if (!db.contains("jawaban")) {
        db["jawaban"] = json::array();
    }

    db["jawaban"].push_back(jawabanData);
    saveDatabase("database.json");
    cout << "Jawaban dan nilai disimpan.\n";
}

int main() {
    const string filename = "database.json";
    if (!loadDatabase(filename)) return 1;

    string role = db["session"]["role"];
    int pilihan;

    do {
        cout << "\n=== DASHBOARD " << (role == "admin" ? "ADMIN" : "USER") << " ===\n";

        if (role == "admin") {
            cout << "1. Tambah Tugas\n";
            cout << "2. Lihat Semua Tugas\n";
            cout << "3. Lihat Detail Tugas\n";
            cout << "4. Tampilkan Semua Materi\n";
            cout << "5. Simpan dan Keluar\n";
        } else {
            cout << "1. Lihat Semua Tugas\n";
            cout << "2. Tampilkan Semua Materi\n";
            cout << "3. Kerjakan Tugas\n";
            cout << "4. Keluar\n";
        }

        cout << "Pilih: ";
        cin >> pilihan;

        if (role == "admin") {
            switch (pilihan) {
                case 1:
                    tampilkanDaftarMateri(db["daftar_mata_pelajaran"]);
                    tambahTugas(); 
                    saveDatabase(filename);
                    break;
                case 2: tampilkanSemuaTugas(); break;
                case 3: tampilkanDetailTugas(); break;
                case 4: tampilkanDaftarMateri(db["daftar_mata_pelajaran"]); break;
                case 5: saveDatabase(filename); cout << "Data disimpan.\n"; break;
                default: cout << "Pilihan tidak valid.\n";
            }
        } else {
            switch (pilihan) {
                case 1: tampilkanSemuaTugas(); break;
                case 2: tampilkanDaftarMateri(db["daftar_mata_pelajaran"]); break;
                case 3: kerjakanTugas(); break;
                case 4: cout << "Keluar...\n"; break;
                default: cout << "Pilihan tidak valid.\n";
        }
}

    } while ((role == "admin" && pilihan != 5) || (role != "admin" && pilihan != 5));

    return 0;
}
