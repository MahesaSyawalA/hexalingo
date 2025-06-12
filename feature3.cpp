#include <iostream>
#include <fstream>
#include <iomanip>
#include "json.hpp"
#include <vector>
#include <string>
#include <limits>
#include <regex>
#include <ctime>

using json = nlohmann::json;
using namespace std;

json db;


const int MAX_QUEUE = 100; // atau sesuaikan batas maksimal
int queue[MAX_QUEUE];
int frontQueue = 0;
int rearQueue = -1;
int queueSize = 0;

// Fungsi dasar
bool isFull(int kapasitas) {
    return queueSize >= kapasitas;
}

bool isEmpty() {
    return queueSize == 0;
}

void enqueue(int data, int kapasitas) {
    if (!isFull(kapasitas)) {
        rearQueue++;
        queue[rearQueue] = data;
        queueSize++;
    } else {
        cout << "Antrian penuh. Tidak dapat menambahkan tugas lagi.\n";
    }
}

int dequeue() {
    if (!isEmpty()) {
        int data = queue[frontQueue];
        frontQueue++;
        queueSize--;
        return data;
    } else {
        cout << "Antrian kosong.\n";
        return -1;
    }
}

void resetQueue() {
    frontQueue = 0;
    rearQueue = -1;
    queueSize = 0;
}

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

bool isValidDeadlineFormat(const std::string& deadline) {
    std::regex pattern(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}$)");
    if (!std::regex_match(deadline, pattern)) return false;

    int year, month, day, hour, minute;
    if (sscanf(deadline.c_str(), "%d-%d-%d %d:%d", &year, &month, &day, &hour, &minute) != 5)
        return false;

    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;  // Bisa dikembangkan untuk validasi kalender yang benar
    if (hour < 0 || hour > 23) return false;
    if (minute < 0 || minute > 59) return false;

    // Buat struct tm untuk deadline
    std::tm tmDeadline = {};
    tmDeadline.tm_year = year - 1900;
    tmDeadline.tm_mon = month - 1;
    tmDeadline.tm_mday = day;
    tmDeadline.tm_hour = hour;
    tmDeadline.tm_min = minute;
    tmDeadline.tm_sec = 0;

    std::time_t deadlineTime = std::mktime(&tmDeadline);
    if (deadlineTime == -1) return false;

    std::time_t now = std::time(nullptr);
    if (deadlineTime < now) {
        std::cout << "Deadline tidak boleh lebih lampau dari waktu sekarang.\n";
        return false;
    }

    return true;
}

bool isValidKunciJawaban(const string& input) {
    return input == "A" || input == "B" || input == "C" || input == "D" ||
           input == "a" || input == "b" || input == "c" || input == "d";
}

void tambahTugas() {
    string idMateri, deadline;
    int jumlahSoal;

    cout << "Masukkan ID materi (contoh: 4.1.1): ";
    cin >> idMateri;
    cin.ignore(); 

    if (!isValidId(db["daftar_mata_pelajaran"], idMateri)) {
        cout << "ID materi tidak ditemukan.\n";
        return;
    }

    string parentId = idMateri;
    size_t pos = idMateri.find('.');
    if (pos != string::npos) {
        parentId = idMateri.substr(0, pos);
    }

    do {
        cout << "Masukkan deadline (format: YYYY-MM-DD HH:MM): ";
        getline(cin, deadline);

        if (!isValidDeadlineFormat(deadline)) {
            cout << "Format deadline tidak valid. Harap gunakan format yang benar.\n";
        }
    } while (!isValidDeadlineFormat(deadline));

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
            do {
                cout << "Opsi " << (char)('A' + j) << ": ";
                getline(cin, opsi);
                if (opsi.empty()) {
                    cout << "Opsi tidak boleh kosong. Silakan masukkan lagi.\n";
                }
            } while (opsi.empty());

            opsiList.push_back(opsi);
        }

        do {
            cout << "Kunci jawaban (A/B/C/D): ";
            getline(cin, kunci);

            if (!isValidKunciJawaban(kunci)) {
                cout << "Input tidak valid. Masukkan hanya A, B, C, atau D.\n";
            }
        } while (!isValidKunciJawaban(kunci));


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

        // Lewati jika tidak relevan dengan mata pelajaran user
        if (!idTerkaitDenganUser(db["daftar_mata_pelajaran"], parentId, mataUser)) {
            continue;
        }

        // Cek apakah user sudah mengerjakan tugas ini
        bool sudahDikerjakan = false;
        for (const auto& j : db["jawaban"]) {
            if (j["user_id"] == user_id && j["id_tugas"] == i) {
                sudahDikerjakan = true;
                break;
            }
        }

        // Lewati jika sudah dikerjakan
        if (sudahDikerjakan) continue;

        // Tampilkan tugas
        cout << "\nTugas #" << i + 1 << endl;
        cout << "ID Materi: " << tugas["id_materi"] << endl;
        printPathById(db["daftar_mata_pelajaran"], tugas["id_materi"]);
        cout << "Deadline: " << tugas["deadline"] << endl;
        cout << "Jumlah Soal: " << tugas["soal"].size() << endl;
        ditemukan = true;
    }

    if (!ditemukan) {
        cout << "Tidak ada tugas yang relevan atau semua sudah Anda kerjakan.\n";
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

    // Cari semua tugas relevan dan belum dikerjakan
    vector<int> tugasRelevanIndex;
    for (size_t i = 0; i < db["tugas"].size(); ++i) {
        const auto& tugas = db["tugas"][i];
        string parentId = tugas["parent_id"];

        // Cek keterkaitan dengan mata pelajaran user
        if (!idTerkaitDenganUser(db["daftar_mata_pelajaran"], parentId, mataUser))
            continue;

        // Cek apakah user sudah mengerjakan tugas ini
        bool sudahDikerjakan = false;
        for (const auto& jawaban : db["jawaban"]) {
            if (jawaban["user_id"] == user_id && jawaban["id_tugas"] == i) {
                sudahDikerjakan = true;
                break;
            }
        }

        if (!sudahDikerjakan) {
            tugasRelevanIndex.push_back(i);
        }
    }

    // Jika tidak ada tugas yang sesuai
    if (tugasRelevanIndex.empty()) {
        cout << "Tidak ada tugas yang relevan atau semua sudah Anda kerjakan.\n";
        return;
    }

    // Tampilkan daftar tugas relevan
    cout << "\n=== Daftar Tugas ===\n";
    for (size_t i = 0; i < tugasRelevanIndex.size(); ++i) {
        int idx = tugasRelevanIndex[i];
        const auto& tugas = db["tugas"][idx];
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
    const auto& tugas = db["tugas"][tugasIndex];
    const auto& soalList = tugas["soal"];
    int benar = 0;
    vector<string> jawabanUser;

    for (size_t i = 0; i < soalList.size(); ++i) {
        cout << "\nSoal #" << i + 1 << ": " << soalList[i]["pertanyaan"] << endl;
        const auto& opsi = soalList[i]["opsi"];
        for (size_t j = 0; j < opsi.size(); ++j) {
            cout << (char)('A' + j) << ". " << opsi[j] << endl;
        }

        string jawaban;
        do {
            cout << "Jawaban Anda (A/B/C/D): ";
            getline(cin, jawaban);
            if (!isValidKunciJawaban(jawaban)) {
                cout << "Input tidak valid. Masukkan hanya A, B, C, atau D.\n";
            }
        } while (!isValidKunciJawaban(jawaban));

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

void antriTugas() {
    resetQueue();

    if (!db.contains("tugas") || db["tugas"].empty()) {
        cout << "Belum ada tugas.\n";
        return;
    }

    int user_id = db["session"]["user_id"];
    vector<int> mataUser = getMataPelajaranUser(user_id);

    // Simpan index tugas dan deadline untuk disortir
    vector<pair<int, string>> tugasRelevan;

    for (size_t i = 0; i < db["tugas"].size(); ++i) {
        const auto& tugas = db["tugas"][i];
        string parentId = tugas["parent_id"];

        if (!idTerkaitDenganUser(db["daftar_mata_pelajaran"], parentId, mataUser))
            continue;

        bool sudahDikerjakan = false;
        for (const auto& j : db["jawaban"]) {
            if (j["user_id"] == user_id && j["id_tugas"] == i) {
                sudahDikerjakan = true;
                break;
            }
        }

        if (!sudahDikerjakan) {
            tugasRelevan.emplace_back(i, tugas["deadline"]);
        }
    }

    // Urutkan berdasarkan deadline (ascending)
    sort(tugasRelevan.begin(), tugasRelevan.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    // Masukkan ke antrian sesuai urutan deadline
    for (const auto& tugas : tugasRelevan) {
        enqueue(tugas.first, MAX_QUEUE);
    }

    if (isEmpty()) {
        cout << "Tidak ada tugas yang relevan untuk diantrikan.\n";
        return;
    }

    cout << "\nTugas-tugas telah dimasukkan ke dalam antrian berdasarkan *deadline*.\n";
    cout << "Total antrian: " << queueSize << "\n";

    int count = 1;
    for (int i = frontQueue; i <= rearQueue; ++i) {
        int idx = queue[i];
        const auto& tugas = db["tugas"][idx];
        cout << count++ << ". Materi: ";
        printPathById(db["daftar_mata_pelajaran"], tugas["id_materi"]);
        cout << "   Deadline: " << tugas["deadline"] << "\n";
    }
}

bool isValidKunciJawaban(const string& jawaban, const json& opsi) {
    if (jawaban.length() != 1) return false;
    char ch = toupper(jawaban[0]);
    int index = ch - 'A';
    return ch >= 'A' && ch <= 'D' && index < opsi.size() && !opsi[index].get<string>().empty();
}

void prosesAntrianTugas() {
    if (isEmpty()) {
        cout << "Antrian kosong. Tambahkan tugas ke antrian terlebih dahulu.\n";
        return;
    }

    int user_id = db["session"]["user_id"];

    cin.ignore();
    while (!isEmpty()) {
        int tugasIndex = dequeue();
        const auto& tugas = db["tugas"][tugasIndex];
        const auto& soalList = tugas["soal"];
        int benar = 0;
        vector<string> jawabanUser;

        cout << "\n--- Mengerjakan Tugas ---\n";
        cout << "Materi: ";
        printPathById(db["daftar_mata_pelajaran"], tugas["id_materi"]);
        cout << "Deadline: " << tugas["deadline"] << endl;

        for (size_t i = 0; i < soalList.size(); ++i) {
            cout << "\nSoal #" << i + 1 << ": " << soalList[i]["pertanyaan"] << endl;
            const auto& opsi = soalList[i]["opsi"];
            for (size_t j = 0; j < opsi.size(); ++j) {
                cout << (char)('A' + j) << ". " << opsi[j] << endl;
            }

            string jawaban;
            do {
                cout << "Jawaban Anda (A/B/C/D): ";
                getline(cin, jawaban);
                if (!isValidKunciJawaban(jawaban, opsi)) {
                    cout << "Input tidak valid.\n";
                }
            } while (!isValidKunciJawaban(jawaban, opsi));

            char jawabanChar = toupper(jawaban[0]);
            jawabanUser.push_back(string(1, jawabanChar));

            if (jawabanChar == toupper(soalList[i]["kunci"].get<string>()[0])) {
                benar++;
            }
        }

        double nilai = (double)benar / soalList.size() * 100.0;
        cout << "\nTugas selesai! Nilai: " << fixed << setprecision(2) << nilai << "\n";

        json jawabanData = {
            {"user_id", user_id},
            {"id_tugas", tugasIndex},
            {"jawaban", jawabanUser},
            {"nilai", nilai}
        };

        db["jawaban"].push_back(jawabanData);
        saveDatabase("database.json");
        cout << "Jawaban disimpan.\n";
    }

    cout << "\nSeluruh tugas dalam antrian telah selesai.\n";
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
            cout << "4. Tampilkan Semua Judul Materi dan SubMateri\n";
            cout << "5. Simpan dan Keluar\n";
        } else {
            cout << "1. Lihat Semua Tugas\n";
            cout << "2. Tampilkan Semua Materi\n";
            cout << "3. Kerjakan Tugas\n";
            cout << "4. Keluar\n";
        }

        cout << "Pilih: ";
        cin >> pilihan;

        if (cin.fail()) {
            cin.clear(); // hapus flag kesalahan
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // buang input buffer sampai baris baru
            cout << "Input tidak valid. Harap masukkan angka.\n";
            pilihan = -1; // memicu default case
        }

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
                case 3: kerjakanTugas(); break;
                case 4: cout << "Keluar...\n"; break;
                default: cout << "Pilihan tidak valid.\n";
            }
        }

    } while ((role == "admin" && pilihan != 5) || (role != "admin" && pilihan != 5));

    return 0;
}