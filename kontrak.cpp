#include <iostream>
#include <fstream>
#include <iomanip>
#include "json.hpp"
#include <vector>
#include <string>

using json = nlohmann::json;
using namespace std;

json dbK;

bool loadDatabaseK(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Gagal membuka file database.\n";
        return false;
    }
    file >> dbK;
    return true;
}

bool saveDatabaseK(const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Gagal menyimpan file database.\n";
        return false;
    }
    file << setw(4) << dbK;
    return true;
}

vector<int> getMataPelajaranUserK(int user_id) {
    for (const auto &user : dbK["users"]) {
        if (user["id"] == user_id) {
            return user["mata_pelajaran"].get<vector<int>>();
        }
    }
    return {};
}

void tampilkanDaftarMateriK(const json &materiList, int indent = 0) //untuk admin
{
    string spasi(indent * 2, ' ');
    for (const auto &materi : materiList) {
        if (!materi.contains("id") || !materi.contains("judul")) continue;
        cout << spasi << "[" << materi["id"] << "] " << materi["judul"] << "\n";
        if (materi.contains("submateri")) {
            tampilkanDaftarMateriK(materi["submateri"], indent + 1);
        }
    }
}

void kontrakmapel(const json &materiList, int user_id) {
    vector<int> mataUser  = getMataPelajaranUserK(user_id);

    cout << "\nMata Pelajaran yang Sudah Anda Miliki:\n";
    if (mataUser .empty()) {
        cout << "- Tidak ada mata pelajaran terkontrak.\n";
    } else {
        for (int idMapel : mataUser ) {
            // Cari judul mapel utama yang cocok dengan idMapel
            string judulMapel = "Judul tidak ditemukan";

            for (const auto &materi : materiList) {
                if (materi.contains("id") && materi.contains("judul")) {
                    string idStr = materi["id"];
                    size_t pos = idStr.find('.');
                    int idUtama = stoi(idStr.substr(0, pos));
                    if (idUtama == idMapel) {
                        judulMapel = materi["judul"];
                        break;
                    }
                }
            }
            cout << "- ID Mapel: " << idMapel << " | Judul: " << judulMapel << "\n";
        }
    }

    cout << "\nDaftar Mata Pelajaran:\n";
    tampilkanDaftarMateriK(materiList);

    string idDipilih;
    cout << "\nMasukkan ID mapel yang ingin Anda kontrak: ";
    cin >> idDipilih;

    // Cek apakah ID tersebut ada di materiList (rekursif)
    function<json(const json&)> cariMateri = [&](const json &list) -> json {
        for (const auto &materi : list) {
            if (materi.contains("id") && materi["id"] == idDipilih)
                return materi;
            if (materi.contains("submateri")) {
                json hasil = cariMateri(materi["submateri"]);
                if (!hasil.is_null()) return hasil;
            }
        }
        return nullptr;
    };

    json materiDipilih = cariMateri(materiList);

    if (!materiDipilih.is_null()) {
        cout << "Anda berhasil mengontrak mata pelajaran: " << materiDipilih["judul"] << "\n";

        // Ambil ID utama dari ID yang dipilih
        int idUtama = stoi(idDipilih.substr(0, idDipilih.find('.'))); // Ambil ID utama

        // Tambahkan ID mapel ke daftar mata pelajaran pengguna jika belum ada
        if (find(mataUser .begin(), mataUser .end(), idUtama) == mataUser .end()) {
            mataUser .push_back(idUtama); // Tambahkan ID mapel jika belum ada
        }

        // Update database
        for (auto &user : dbK["users"]) {
            if (user["id"] == user_id) {
                user["mata_pelajaran"] = mataUser ; // Update daftar mata pelajaran
                break;
            }
        }

        // Simpan perubahan ke database
        saveDatabaseK("database.json");
    } else {
        cout << "ID tidak ditemukan dalam daftar materi.\n";
    }
}

void hapusKontrakMapel(const json &materiList, int user_id) {
    vector<int> mataUser = getMataPelajaranUserK(user_id);

    if (mataUser.empty()) {
        cout << "\nAnda tidak memiliki mata pelajaran yang dikontrak.\n";
        return;
    }

    cout << "\nMata Pelajaran yang Sudah Anda Miliki:\n";
    for (size_t i = 0; i < mataUser.size(); ++i) {
        int idMapel = mataUser[i];
        string judulMapel = "Judul tidak ditemukan";

        for (const auto &materi : materiList) {
            if (materi.contains("id") && materi.contains("judul")) {
                string idStr = materi["id"];
                size_t pos = idStr.find('.');
                int idUtama = stoi(idStr.substr(0, pos));
                if (idUtama == idMapel) {
                    judulMapel = materi["judul"];
                    break;
                }
            }
        }
        cout << (i + 1) << ". ID Mapel: " << idMapel << " | Judul: " << judulMapel << "\n";
    }

    cout << "\nMasukkan nomor mata pelajaran yang ingin dihapus kontraknya: ";
    int pilihan;
    cin >> pilihan;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (pilihan < 1 || pilihan > (int)mataUser.size()) {
        cout << "Pilihan tidak valid.\n";
        return;
    }

    // Hapus mata pelajaran dipilih
    int idTerhapus = mataUser[pilihan - 1];
    mataUser.erase(mataUser.begin() + (pilihan - 1));

    // Perbarui database
    for (auto &user : dbK["users"]) {
        if (user["id"] == user_id) {
            user["mata_pelajaran"] = mataUser;
            break;
        }
    }

    // Simpan perubahan ke database
    if (saveDatabaseK("database.json")) {
        cout << "Mata pelajaran dengan ID " << idTerhapus << " berhasil dihapus dari kontrak Anda.\n";
    } else {
        cout << "Gagal menyimpan perubahan.\n";
    }
}

int mainKontrak() {
    const string filename = "database.json";
    if (!loadDatabaseK(filename)) return 1;

    string role = dbK["session"]["role"];
    int pilihan;

    do {
        cout << "\n=== DASHBOARD " << (role == "admin" ? "ADMIN" : "USER") << " ===\n";
            cout << "1. Tampilkan Semua Materi\n";
            cout << "2. Kontrak Mata Pelajaran\n";
            cout << "3. Hapus Kontrak Mata Pelajaran\n";
            cout << "4. Keluar\n";
        

        cout << "Pilih: ";
        cin >> pilihan;

            switch (pilihan) {
                case 1: tampilkanDaftarMateriK(dbK["daftar_mata_pelajaran"]); break;
                case 2: kontrakmapel(dbK["daftar_mata_pelajaran"], dbK["session"]["user_id"]);break;
                case 3: hapusKontrakMapel(dbK["daftar_mata_pelajaran"], dbK["session"]["user_id"]);break; 
                case 4: cout << "Keluar...\n"; break;
                default: cout << "Pilihan tidak valid.\n";
            }
        

    } while ((role == "admin" && pilihan != 5) || (role != "admin" && pilihan != 4));

    return 0;
}

