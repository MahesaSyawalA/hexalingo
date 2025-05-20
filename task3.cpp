#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <algorithm>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

struct Tugas {
    int id;
    int id_matakuliah;
    string nama_matkul;
    string tengat_waktu;
    string title;
    json soal;
    bool sudah_dikerjakan;
    float nilai; // Tambahkan field untuk menyimpan nilai tugas
};

// Struktur untuk menyimpan data user
struct User {
    int id;
    string name;
    string email;
    string role;
    vector<int> matakuliah;
};

// Global variables
queue<Tugas> antrianTugas;
vector<User> users;
vector<Tugas> semuaTugas;
User currentUser;
map<int, string> matakuliahMap;
json rawData; 


// Fungsi untuk memuat data dari JSON
void loadTugas() {
    semuaTugas.clear();
    
    // Create a map to track which assignments the current user has completed
    map<int, pair<bool, float>> tugasStatus; // {id_tugas -> {completed, nilai}}
    
    if (rawData.contains("jawaban")) {
        for (const auto& jawaban : rawData["jawaban"]) {
            if (jawaban["id_user"] == currentUser.id) {
                int id_tugas = jawaban["id_tugas"];
                float nilai = 0;
                
                // Jika ada informasi nilai, ambil nilai total
                if (jawaban.contains("nilai") && jawaban["nilai"].contains("total_nilai")) {
                    nilai = jawaban["nilai"]["total_nilai"];
                }
                
                tugasStatus[id_tugas] = {true, nilai};
            }
        }
    }
    
    // Load tugas
    int tugasId = 1;
    for (const auto& tugas : rawData["tugas"]) {
        Tugas t;
        t.id = tugasId++;
        t.id_matakuliah = tugas["id_matakuliah"];
        t.nama_matkul = matakuliahMap[t.id_matakuliah];
        t.tengat_waktu = tugas["tengat_waktu"];
        t.title = tugas["title"];
        t.soal = tugas["soal"];
        
        // Check if this task has been completed by the current user
        auto it = tugasStatus.find(t.id);
        if (it != tugasStatus.end()) {
            t.sudah_dikerjakan = it->second.first;
            t.nilai = it->second.second;
        } else {
            t.sudah_dikerjakan = false;
            t.nilai = 0;
        }
        
        // Hanya tambahkan tugas yang terkait dengan matakuliah yang diambil oleh user
        if (currentUser.role == "admin" || 
            find(currentUser.matakuliah.begin(), currentUser.matakuliah.end(), t.id_matakuliah) != currentUser.matakuliah.end()) {
            semuaTugas.push_back(t);
        }
    }
}

void loadData(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Gagal membuka file JSON" << endl;
        return;
    }

    file >> rawData;

    // Load matakuliah untuk mapping
    for (const auto& mk : rawData["matakuliah"]) {
        matakuliahMap[mk["id"]] = mk["nama"];
    }

    // Load users
    for (const auto& user : rawData["users"]) {
        User u;
        u.id = user["id"];
        u.name = user["name"];
        u.email = user["email"];
        u.role = user["role"];
        for (const auto& mk : user["matakuliah"]) {
            u.matakuliah.push_back(mk);
        }
        users.push_back(u);
    }

    // Load session
    if (rawData.contains("session")) {
        int userId = rawData["session"]["user_id"];
        for (const auto& user : users) {
            if (user.id == userId) {
                currentUser = user;
                break;
            }
        }
    }

    // Load tugas dan check completion status
    loadTugas();

    file.close();
}

void saveData() {
    cout << "Menyimpan perubahan ke database..." << endl;
    
    ofstream file("database.json");
    file << rawData.dump(4);
    file.close();
    
    cout << "Data berhasil disimpan!" << endl;
}

// Fungsi untuk menampilkan tabel tugas
void tampilkanSemuaTugas() {
    cout << "\n======================================================================================" << endl;
    cout << "                             DAFTAR TUGAS DAN STATUS PENGERJAAN" << endl;
    cout << "======================================================================================" << endl;
    cout << left << setw(5) << "No" 
         << setw(30) << "Judul Tugas" 
         << setw(25) << "Mata Kuliah" 
         << setw(15) << "Deadline" 
         << setw(25) << "Status" << endl;
    cout << "--------------------------------------------------------------------------------------" << endl;

    for (size_t i = 0; i < semuaTugas.size(); i++) {
        const Tugas& t = semuaTugas[i];
        string status = t.sudah_dikerjakan ? 
                       "✓ Sudah (Nilai: " + to_string(int(t.nilai)) + ")" : 
                       "✗ Belum Dikerjakan";
        
        cout << left << setw(5) << i+1 
             << setw(30) << (t.title.length() > 28 ? t.title.substr(0, 27) + "." : t.title)
             << setw(25) << t.nama_matkul 
             << setw(15) << t.tengat_waktu 
             << setw(25) << status
             << endl;
    }
    cout << "======================================================================================" << endl;
}

// Fungsi untuk mengerjakan tugas
void kerjakanTugas(int tugasIndex) {
    if (tugasIndex < 0 || tugasIndex >= semuaTugas.size()) {
        cout << "Indeks tugas tidak valid!" << endl;
        return;
    }
    
    Tugas& tugas = semuaTugas[tugasIndex];
    
    if (tugas.sudah_dikerjakan) {
        cout << "Tugas ini sudah dikerjakan sebelumnya!" << endl;
        return;
    }

    cout << "\nMengerjakan Tugas: " << tugas.title << endl;
    cout << "Mata Kuliah: " << tugas.nama_matkul << endl;
    cout << "Deadline: " << tugas.tengat_waktu << endl;
    cout << "Jumlah soal: " << tugas.soal.size() << endl;

    // Simulasi pengerjaan tugas
    vector<json> jawabanUser;
    float totalNilai = 0;
    vector<json> detailNilai;
    
    for (const auto& soal : tugas.soal) {
        int id_soal = soal["id_soal"];
        string pertanyaan = soal["pertanyaan"];
        string tipe = soal["tipe"];
        float bobot = soal["bobot_nilai"];
        
        cout << "\nSoal " << id_soal << ": " << pertanyaan << endl;
        
        if (tipe == "pilihan_ganda") {
            for (const auto& pilihan : soal["pilihan"]) {
                cout << pilihan["option"] << ". " << pilihan["text"] << endl;
            }
        }
        
        cout << "Jawaban Anda: ";
        string jawaban;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer
        getline(cin, jawaban);
        
        // Simulasi penilaian otomatis untuk pilihan ganda
        float nilai = 0;
        if (tipe == "pilihan_ganda" && soal.contains("jawaban_benar")) {
            if (jawaban == soal["jawaban_benar"]) {
                nilai = bobot;
            }
        } else {
            // Untuk essay, anggap mendapat nilai penuh (simulasi)
            nilai = bobot;
        }
        
        // Tambahkan jawaban ke array jawaban
        json jawabanItem;
        jawabanItem["id_soal"] = id_soal;
        jawabanItem["jawaban"] = jawaban;
        jawabanUser.push_back(jawabanItem);
        
        // Tambahkan nilai ke array nilai
        json nilaiItem;
        nilaiItem["id_soal"] = id_soal;
        nilaiItem["nilai"] = nilai;
        detailNilai.push_back(nilaiItem);
        
        totalNilai += nilai;
    }
    
    // Simpan jawaban ke database
    json newJawaban;
    newJawaban["id_user"] = currentUser.id;
    newJawaban["id_tugas"] = tugas.id;
    newJawaban["jawaban_user"] = jawabanUser;
    
    // Tambahkan informasi nilai
    json nilaiInfo;
    nilaiInfo["total_nilai"] = totalNilai;
    nilaiInfo["detail_nilai"] = detailNilai;
    newJawaban["nilai"] = nilaiInfo;
    
    // Update data di memori
    rawData["jawaban"].push_back(newJawaban);
    
    // Update status tugas
    tugas.sudah_dikerjakan = true;
    tugas.nilai = totalNilai;
    
    // Hapus dari antrian jika ada
    queue<Tugas> tempQueue;
    while (!antrianTugas.empty()) {
        Tugas t = antrianTugas.front();
        antrianTugas.pop();
        if (t.id != tugas.id) {
            tempQueue.push(t);
        }
    }
    antrianTugas = tempQueue;
    
    // Simpan perubahan
    saveData();
    
    cout << "\nTugas berhasil dikerjakan dengan total nilai: " << totalNilai << endl;
}

// Fungsi untuk menampilkan menu admin
void adminMenu() {
    while (true) {
        cout << "\n=== MENU ADMIN ===" << endl;
        cout << "1. Tambahkan Tugas ke Antrian" << endl;
        cout << "2. Lihat Antrian Tugas" << endl;
        cout << "3. Tampilkan Semua Tugas" << endl;
        cout << "4. Tampilkan Status Pengerjaan Tugas" << endl;
        cout << "5. Reset Status Tugas Mahasiswa" << endl;
        cout << "6. Keluar" << endl;
        cout << "Pilihan: ";

        int choice;
        cin >> choice;

        if (choice == 1) {
            cout << "\nDaftar Tugas yang Tersedia:" << endl;
            for (int i = 0; i < semuaTugas.size(); i++) {
                cout << i+1 << ". " << semuaTugas[i].title 
                     << " (Mata Kuliah: " << semuaTugas[i].nama_matkul << ")" << endl;
            }

            cout << "Pilih tugas untuk ditambahkan ke antrian (0 untuk batal): ";
            int pilihan;
            cin >> pilihan;

            if (pilihan > 0 && pilihan <= semuaTugas.size()) {
                antrianTugas.push(semuaTugas[pilihan-1]);
                cout << "Tugas berhasil ditambahkan ke antrian!" << endl;
            }
        } 
        else if (choice == 2) {
            cout << "\nAntrian Tugas Saat Ini:" << endl;
            if (antrianTugas.empty()) {
                cout << "Antrian kosong" << endl;
            } else {
                queue<Tugas> temp = antrianTugas;
                int posisi = 1;
                while (!temp.empty()) {
                    Tugas t = temp.front();
                    cout << posisi++ << ". " << t.title 
                         << " (Deadline: " << t.tengat_waktu << ")" << endl;
                    temp.pop();
                }
            }
        }
        else if (choice == 3) {
            tampilkanSemuaTugas();
        }
        else if (choice == 4) {
            // Tampilkan status pengerjaan tugas semua mahasiswa
            cout << "\n=== STATUS PENGERJAAN TUGAS MAHASISWA ===" << endl;
            
            for (const auto& user : users) {
                if (user.role == "mahasiswa") {
                    cout << "\nMahasiswa: " << user.name << endl;
                    cout << "------------------------------------" << endl;
                    
                    // Hitung berapa tugas yang sudah dikerjakan
                    int tugasDikerjakan = 0;
                    int totalTugas = 0;
                    
                    for (const auto& jawaban : rawData["jawaban"]) {
                        if (jawaban["id_user"] == user.id) {
                            tugasDikerjakan++;
                        }
                    }
                    
                    for (const auto& tugas : rawData["tugas"]) {
                        int id_matkul = tugas["id_matakuliah"];
                        if (find(user.matakuliah.begin(), user.matakuliah.end(), id_matkul) != user.matakuliah.end()) {
                            totalTugas++;
                        }
                    }
                    
                    cout << "Total tugas: " << totalTugas << endl;
                    cout << "Sudah dikerjakan: " << tugasDikerjakan << endl;
                    cout << "Belum dikerjakan: " << (totalTugas - tugasDikerjakan) << endl;
                    
                    if (totalTugas > 0) {
                        float persentase = (float)tugasDikerjakan / totalTugas * 100;
                        cout << "Persentase: " << fixed << setprecision(2) << persentase << "%" << endl;
                    }
                }
            }
        }
        else if (choice == 5) {
            cout << "\nPilih mahasiswa untuk reset status tugas:" << endl;
            int i = 1;
            for (const auto& user : users) {
                if (user.role == "mahasiswa") {
                    cout << i++ << ". " << user.name << endl;
                }
            }
            
            cout << "Pilihan (0 untuk batal): ";
            int pilihan;
            cin >> pilihan;
            
            // Hitung indeks mahasiswa yang dipilih
            int mahasiswaIndex = 0;
            for (size_t j = 0; j < users.size(); j++) {
                if (users[j].role == "mahasiswa") {
                    mahasiswaIndex++;
                    if (mahasiswaIndex == pilihan) {
                        // Hapus semua jawaban mahasiswa tersebut
                        json newJawaban = json::array();
                        for (const auto& jawaban : rawData["jawaban"]) {
                            if (jawaban["id_user"] != users[j].id) {
                                newJawaban.push_back(jawaban);
                            }
                        }
                        rawData["jawaban"] = newJawaban;
                        cout << "Status tugas untuk " << users[j].name << " berhasil direset!" << endl;
                        
                        // Reload tugas untuk memperbarui status
                        loadTugas();
                        break;
                    }
                }
            }
        }
        else if (choice == 6) {
            break;
        } 
        else {
            cout << "Pilihan tidak valid!" << endl;
        }
    }
}

// Fungsi untuk menampilkan menu mahasiswa
void mahasiswaMenu() {
    while (true) {
        cout << "\n=== MENU MAHASISWA ===" << endl;
        cout << "1. Kerjakan Tugas Berikutnya dari Antrian" << endl;
        cout << "2. Lihat Tugas yang Belum Dikerjakan" << endl;
        cout << "3. Tampilkan Semua Tugas dan Status" << endl;
        cout << "4. Kerjakan Tugas Berdasarkan Nomor" << endl;
        cout << "5. Lihat Detail Nilai" << endl;
        cout << "6. Keluar" << endl;
        cout << "Pilihan: ";

        int choice;
        cin >> choice;

        if (choice == 1) {
            if (antrianTugas.empty()) {
                cout << "Tidak ada tugas dalam antrian" << endl;
                continue;
            }

            Tugas tugas = antrianTugas.front();
            cout << "\nMengerjakan Tugas: " << tugas.title << endl;
            cout << "Mata Kuliah: " << tugas.nama_matkul << endl;
            cout << "Deadline: " << tugas.tengat_waktu << endl;
            cout << "Jumlah soal: " << tugas.soal.size() << endl;

            cout << "Apakah ingin mengerjakan tugas ini sekarang? (y/n): ";
            char confirm;
            cin >> confirm;

            if (confirm == 'y' || confirm == 'Y') {
                kerjakanTugas(find_if(semuaTugas.begin(), semuaTugas.end(), 
                                     [&tugas](const Tugas& t) { return t.id == tugas.id; }) - semuaTugas.begin());
            }
        } 
        else if (choice == 2) {
            cout << "\nTugas yang Belum Dikerjakan:" << endl;
            bool adaTugas = false;
            for (size_t i = 0; i < semuaTugas.size(); i++) {
                if (!semuaTugas[i].sudah_dikerjakan) {
                    cout << i+1 << ". " << semuaTugas[i].title 
                         << " (Deadline: " << semuaTugas[i].tengat_waktu << ")" << endl;
                    adaTugas = true;
                }
            }
            if (!adaTugas) {
                cout << "Semua tugas sudah dikerjakan. Selamat!" << endl;
            }
        }
        else if (choice == 3) {
            tampilkanSemuaTugas();
        }
        else if (choice == 4) {
            tampilkanSemuaTugas();
            cout << "\nPilih nomor tugas yang ingin dikerjakan (0 untuk batal): ";
            int pilihan;
            cin >> pilihan;
            
            if (pilihan > 0 && pilihan <= semuaTugas.size()) {
                if (semuaTugas[pilihan-1].sudah_dikerjakan) {
                    cout << "Tugas ini sudah dikerjakan sebelumnya!" << endl;
                } else {
                    kerjakanTugas(pilihan-1);
                }
            }
        }
        else if (choice == 5) {
            // Menampilkan detail nilai
            cout << "\n=== DETAIL NILAI TUGAS ===" << endl;
            bool adaNilai = false;
            
            for (const auto& jawaban : rawData["jawaban"]) {
                if (jawaban["id_user"] == currentUser.id && jawaban.contains("nilai")) {
                    adaNilai = true;
                    
                    // Cari tugas yang sesuai
                    int id_tugas = jawaban["id_tugas"];
                    string judulTugas = "Unknown";
                    string namaMatkul = "Unknown";
                    
                    for (const auto& tugas : semuaTugas) {
                        if (tugas.id == id_tugas) {
                            judulTugas = tugas.title;
                            namaMatkul = tugas.nama_matkul;
                            break;
                        }
                    }
                    
                    float totalNilai = jawaban["nilai"]["total_nilai"];
                    
                    cout << "\nTugas: " << judulTugas << endl;
                    cout << "Mata Kuliah: " << namaMatkul << endl;
                    cout << "Total Nilai: " << totalNilai << endl;
                    
                    cout << "Detail Nilai Per Soal:" << endl;
                    for (const auto& nilai : jawaban["nilai"]["detail_nilai"]) {
                        cout << "  Soal " << nilai["id_soal"] << ": " << nilai["nilai"] << endl;
                    }
                    cout << "-------------------------------" << endl;
                }
            }
            
            if (!adaNilai) {
                cout << "Anda belum memiliki nilai untuk tugas apapun." << endl;
            }
        }
        else if (choice == 6) {
            break;
        } 
        else {
            cout << "Pilihan tidak valid!" << endl;
        }
    }
}

int main() {
    // Load data dari file JSON
    loadData("database.json");

    // Login sederhana
    cout << "=== LOGIN ===" << endl;
    cout << "Masukkan ID User: ";
    int userId;
    cin >> userId;

    bool found = false;
    for (const auto& user : users) {
        if (user.id == userId) {
            currentUser = user;
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "User tidak ditemukan!" << endl;
        return 1;
    }

    cout << "Selamat datang, " << currentUser.name << " (" << currentUser.role << ")" << endl;

    // Tampilkan menu sesuai role
    if (currentUser.role == "admin") {
        adminMenu();
    } else if (currentUser.role == "mahasiswa") {
        mahasiswaMenu();
    } else {
        cout << "Role tidak dikenali" << endl;
    }

    return 0;
}