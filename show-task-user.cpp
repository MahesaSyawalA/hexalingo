#include <iostream>
#include <fstream>
#include <iomanip>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

json db2;

/* ----------------------------------------------------------------
   Memuat database dari file JSON
---------------------------------------------------------------- */
bool loadDatabase2(const string& filename) {
    ifstream in(filename);
    if (!in.is_open()) {
        cerr << "Gagal membuka " << filename << '\n';
        return false;
    }
    try {
        in >> db2;
    } catch (const exception& e) {
        cerr << "Error parse JSON: " << e.what() << '\n';
        return false;
    }
    return true;
}

/* ----------------------------------------------------------------
   Mencari judul materi by id secara rekursif
---------------------------------------------------------------- */
string cariJudulMateri(const json& list, const string& id) {
    for (const auto& m : list) {
        if (m["id"] == id && m.contains("judul"))
            return m["judul"];
        if (m.contains("submateri") && !m["submateri"].empty()) {
            string hasil = cariJudulMateri(m["submateri"], id);
            if (!hasil.empty()) return hasil;
        }
    }
    return "";
}

/* ----------------------------------------------------------------
   Menampilkan daftar tugas yang dikerjakan oleh user
---------------------------------------------------------------- */
vector<int> tampilkanTugasUser(int userId) {
    vector<int> daftarTugas;

    cout << "\n=== DAFTAR TUGAS YANG TELAH DIKERJAKAN (User ID: " << userId << ") ===\n";
    for (const auto& rec : db2["jawaban"]) {
        if (rec["user_id"] != userId) continue;

        int idTugas = rec["id_tugas"];
        if (idTugas < 0 || idTugas >= db2["tugas"].size()) continue;

        const auto& tugas = db2["tugas"][idTugas];
        string judul = cariJudulMateri(db2["daftar_mata_pelajaran"], tugas["id_materi"]);

        cout << "- ID Tugas: " << idTugas
             << " | Materi: " << tugas["id_materi"] << " - " << judul
             << " | Nilai: " << rec["nilai"] << "\n";

        daftarTugas.push_back(idTugas);
    }

    return daftarTugas;
}

/* ----------------------------------------------------------------
   Menampilkan detail soal dari tugas tertentu
---------------------------------------------------------------- */
void tampilkanDetailTugas(int userId, int idTugas) {
    const json& tugas = db2["tugas"][idTugas];
    string judul = cariJudulMateri(db2["daftar_mata_pelajaran"], tugas["id_materi"]);

    // cari data jawaban dari user yang cocok
    json jawabanUser;
    bool ditemukan = false;
    for (const auto& rec : db2["jawaban"]) {
        if (rec["user_id"] == userId && rec["id_tugas"] == idTugas) {
            jawabanUser = rec;
            ditemukan = true;
            break;
        }
    }

    if (!ditemukan) {
        cout << "Data jawaban tidak ditemukan.\n";
        return;
    }

    cout << "\n===== DETAIL TUGAS ID: " << idTugas << " =====\n";
    cout << "Materi     : " << tugas["id_materi"] << " - " << judul << "\n";
    cout << "Deadline   : " << tugas["deadline"] << "\n";
    cout << "Nilai      : " << jawabanUser["nilai"] << "\n";

    const auto& soalArr = tugas["soal"];
    const auto& jwbArr = jawabanUser["jawaban"];

    for (size_t i = 0; i < soalArr.size(); ++i) {
        const auto& s = soalArr[i];
        cout << "\nPertanyaan #" << (i + 1) << ": " << s["pertanyaan"] << '\n';

        string jenis = s.contains("jenis") ? string(s["jenis"]) : "pilihan";

        if (jenis == "essay") {
            cout << "Jenis Soal  : Essay\n";
            cout << "Kunci Jawaban : " << s["kunci"] << "\n";
            if (i < jwbArr.size())
                cout << "Jawaban Anda  : " << jwbArr[i] << '\n';
            else
                cout << "Jawaban Anda  : (tidak dijawab)\n";
        } else {
            cout << "Jenis Soal  : Pilihan Ganda\n";
            for (size_t o = 0; o < s["opsi"].size(); ++o) {
                cout << "  (" << char('A' + o) << ") " << s["opsi"][o] << '\n';
            }

            cout << "Kunci Jawaban : " << s["kunci"] << '\n';
            if (i < jwbArr.size())
                cout << "Jawaban Anda  : " << jwbArr[i] << '\n';
            else
                cout << "Jawaban Anda  : (tidak dijawab)\n";
        }
    }
}

/* ---------------------------------------------------------------- */
int mainDetailTugasSelesai() {
    const string filename = "database.json";
    if (!loadDatabase2(filename)) return 1;

    if (!db2.contains("session") || !db2["session"].contains("user_id")) {
        cout << "Session tidak valid.\n";
        return 1;
    }

    int userId = db2["session"]["user_id"];
    vector<int> daftar = tampilkanTugasUser(userId);

    if (daftar.empty()) {
        cout << "Belum ada tugas yang dikerjakan.\n";
        return 0;
    }

    int pilih;
    cout << "\nMasukkan ID tugas yang ingin dilihat detailnya: ";
    cin >> pilih;

    if (find(daftar.begin(), daftar.end(), pilih) == daftar.end()) {
        cout << "ID tugas tidak valid atau bukan milik user ini.\n";
        return 0;
    }

    tampilkanDetailTugas(userId, pilih);
    return 0;
}
