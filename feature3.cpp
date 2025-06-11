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

vector<int> getMataPelajaranUser(int user_id) {
    for (const auto &user : db["users"]) {
        if (user["id"] == user_id) {
            return user["mata_pelajaran"].get<vector<int>>();
        }
    }
    return {};
}

bool idTerkaitDenganUser(const json &materiList, const string &targetId, const vector<int> &mataUser) {
    for (const auto &materi : materiList) {
        if (!materi.contains("id")) continue;
        string id = materi["id"];
        if (targetId == id && materi.contains("judul")) {
            int idUtama = stoi(id.substr(0, id.find('.')));
            return find(mataUser.begin(), mataUser.end(), idUtama) != mataUser.end();
        }
        if (materi.contains("submateri") && idTerkaitDenganUser(materi["submateri"], targetId, mataUser)) {
            return true;
        }
    }
    return false;
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

void tampilkanDaftarMateri(const json &materiList, int indent = 0) //untuk admin
{
    string spasi(indent * 2, ' ');
    for (const auto &materi : materiList) {
        if (!materi.contains("id") || !materi.contains("judul")) continue;
        cout << spasi << "[" << materi["id"] << "] " << materi["judul"] << "\n";
        if (materi.contains("submateri")) {
            tampilkanDaftarMateri(materi["submateri"], indent + 1);
        }
    }
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

    string parentId = idMateri;
    size_t pos = idMateri.find('.');
    if (pos != string::npos) {
        parentId = idMateri.substr(0, pos);
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
        {"parent_id", parentId},
        {"deadline", deadline},
        {"soal", soalList}
    };

    db["tugas"].push_back(tugas);
    cout << "Tugas berhasil ditambahkan!\n";
    printPathById(db["daftar_mata_pelajaran"], idMateri);
}

void tampilkanSemuaTugasUntukUser(int user_id) {
    vector<int> mataUser = getMataPelajaranUser(user_id);
    bool ditemukan = false;

    for (size_t i = 0; i < db["tugas"].size(); ++i) {
        const auto &tugas = db["tugas"][i];
        string parentId = tugas["parent_id"];
        if (idTerkaitDenganUser(db["daftar_mata_pelajaran"], parentId, mataUser)) {
            cout << "\nTugas #" << i + 1 << endl;
            cout << "ID Materi: " << tugas["id_materi"] << endl;
            printPathById(db["daftar_mata_pelajaran"], tugas["id_materi"]);
            cout << "Deadline: " << tugas["deadline"] << endl;
            cout << "Jumlah Soal: " << tugas["soal"].size() << endl;
            ditemukan = true;
        }
    }

    if (!ditemukan) {
        cout << "Tidak ada tugas yang relevan dengan mata pelajaran Anda.\n";
    }
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

    cout << "\n=== Daftar Tugas ===\n";
    for (size_t i = 0; i < db["tugas"].size(); ++i) {
        string idMateri = db["tugas"][i]["id_materi"];
        cout << i + 1 << ". Materi: ";
        printPathById(db["daftar_mata_pelajaran"], idMateri);
        cout << "   Deadline: " << db["tugas"][i]["deadline"] << endl;
    }

    int index;
    cout << "\nPilih nomor tugas untuk melihat detail: ";
    cin >> index;
    cin.ignore();

    if (index < 1 || index > db["tugas"].size()) {
        cout << "Nomor tugas tidak valid.\n";
        return;
    }

    const auto &tugas = db["tugas"][index - 1];
    const auto &soalList = tugas["soal"];

    cout << "\n=== Detail Tugas ===\n";
    cout << "Materi: ";
    printPathById(db["daftar_mata_pelajaran"], tugas["id_materi"]);
    cout << "Deadline: " << tugas["deadline"] << "\n";
    cout << "Jumlah Soal: " << soalList.size() << "\n";

    for (size_t j = 0; j < soalList.size(); ++j) {
        cout << "\nSoal #" << j + 1 << ": " << soalList[j]["pertanyaan"] << endl;
        const auto &opsi = soalList[j]["opsi"];
        for (size_t k = 0; k < opsi.size(); ++k) {
            cout << (char)('A' + k) << ". " << opsi[k] << endl;
        }
        cout << "Kunci Jawaban: " << soalList[j]["kunci"] << endl;
    }
}

void kerjakanTugas() {
    if (!db.contains("tugas") || db["tugas"].empty()) {
        cout << "Belum ada tugas yang tersedia.\n";
        return;
    }

    int user_id = db["session"]["user_id"];
    vector<int> mataUser = getMataPelajaranUser(user_id);

    // Cari semua tugas relevan
    vector<int> tugasRelevanIndex;
    for (size_t i = 0; i < db["tugas"].size(); ++i) {
        const auto &tugas = db["tugas"][i];
        string parentId = tugas["parent_id"];
        if (idTerkaitDenganUser(db["daftar_mata_pelajaran"], parentId, mataUser)) {
            tugasRelevanIndex.push_back(i);
        }
    }

    // Jika tidak ada tugas yang sesuai
    if (tugasRelevanIndex.empty()) {
        cout << "Tidak ada tugas yang relevan dengan mata pelajaran Anda.\n";
        return;
    }

    // Tampilkan daftar tugas relevan
    cout << "\n=== Daftar Tugas ===\n";
    for (size_t i = 0; i < tugasRelevanIndex.size(); ++i) {
        int idx = tugasRelevanIndex[i];
        const auto &tugas = db["tugas"][idx];
        cout << i + 1 << ". Materi: ";
        printPathById(db["daftar_mata_pelajaran"], tugas["id_materi"]);
        cout << "   Deadline: " << tugas["deadline"] << endl;
    }

    int pilihan;
    cout << "\nPilih nomor tugas yang ingin dikerjakan: ";
    cin >> pilihan;
    cin.ignore();

    if (pilihan < 1 || pilihan > tugasRelevanIndex.size()) {
        cout << "Pilihan tidak valid.\n";
        return;
    }

    int tugasIndex = tugasRelevanIndex[pilihan - 1];
    const auto &tugas = db["tugas"][tugasIndex];
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

    json jawabanData = {
        {"user_id", user_id},
        {"id_tugas", tugasIndex},
        {"jawaban", jawabanUser},
        {"nilai", nilai}
    };

    db["jawaban"].push_back(jawabanData);
    saveDatabase("database.json");
    cout << "Jawaban dan nilai disimpan.\n";
}

void kontrakmapel(const json &materiList, int user_id) {
    vector<int> mataUser  = getMataPelajaranUser (user_id);

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
    tampilkanDaftarMateri(materiList);

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
        for (auto &user : db["users"]) {
            if (user["id"] == user_id) {
                user["mata_pelajaran"] = mataUser ; // Update daftar mata pelajaran
                break;
            }
        }

        // Simpan perubahan ke database
        saveDatabase("database.json");
    } else {
        cout << "ID tidak ditemukan dalam daftar materi.\n";
    }
}

void hapusKontrakMapel(const json &materiList, int user_id) {
    vector<int> mataUser = getMataPelajaranUser(user_id);

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
    for (auto &user : db["users"]) {
        if (user["id"] == user_id) {
            user["mata_pelajaran"] = mataUser;
            break;
        }
    }

    // Simpan perubahan ke database
    if (saveDatabase("database.json")) {
        cout << "Mata pelajaran dengan ID " << idTerhapus << " berhasil dihapus dari kontrak Anda.\n";
    } else {
        cout << "Gagal menyimpan perubahan.\n";
    }
}

int mainTugas() {
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
            cout << "3. Kontrak Mata Pelajaran\n";
            cout << "4. Hapus Kontrak Mata Pelajaran\n";
            cout << "5. Kerjakan Tugas\n";
            cout << "6. Keluar\n";
        }

        cout << "Pilih: ";
        cin >> pilihan;

        if (role == "admin") {
            switch (pilihan) {
                case 1: tampilkanDaftarMateri(db["daftar_mata_pelajaran"]); tambahTugas(); saveDatabase(filename); break;
                case 2: tampilkanSemuaTugas(); break;
                case 3: tampilkanDetailTugas(); break;
                case 4: tampilkanDaftarMateri(db["daftar_mata_pelajaran"]); break;
                case 5: saveDatabase(filename); cout << "Data disimpan.\n"; break;
                default: cout << "Pilihan tidak valid.\n";
            }
        } else {
            switch (pilihan) {
                case 1: tampilkanSemuaTugasUntukUser(db["session"]["user_id"]); break;
                case 2: tampilkanDaftarMateri(db["daftar_mata_pelajaran"]); break;
                case 3: kontrakmapel(db["daftar_mata_pelajaran"], db["session"]["user_id"]);break;
                case 4: hapusKontrakMapel(db["daftar_mata_pelajaran"], db["session"]["user_id"]);break; 
                case 5: kerjakanTugas(); break;
                case 6: cout << "Keluar...\n"; break;
                default: cout << "Pilihan tidak valid.\n";
            }
        }

    } while ((role == "admin" && pilihan != 5) || (role != "admin" && pilihan != 4));

    return 0;
}
