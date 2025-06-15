#include <iostream>
#include <fstream>
#include <iomanip>
#include "json.hpp"
#include <vector>
#include <string>
#include <limits>
#include <regex>
#include <ctime>
#include "cls.cpp"
#include "show-task-user.cpp"

using json = nlohmann::json;
using namespace std;

json db;


const int MAX_QUEUE = 3; 
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

    /* ---------- Validasi ID materi ---------- */
    if (!isValidId(db["daftar_mata_pelajaran"], idMateri)) {
        cout << "ID materi tidak ditemukan.\n";
        return;
    }

    /* ---------- Tentukan parent_id (level teratas) ---------- */
    string parentId = idMateri.substr(0, idMateri.find('.'));
    if (parentId.empty()) parentId = idMateri;

    /* ---------- Input deadline dengan validasi ---------- */
    do {
        cout << "Masukkan deadline (format: YYYY-MM-DD HH:MM): ";
        getline(cin, deadline);

        if (!isValidDeadlineFormat(deadline))
            cout << "Format deadline tidak valid. Harap gunakan format yang benar.\n";
    } while (!isValidDeadlineFormat(deadline));

    cout << "Masukkan jumlah soal: ";
    cin >> jumlahSoal;
    cin.ignore();

    json soalList = json::array();

    /* =======================================================
       LOOP INPUT SOAL
    ======================================================= */
    for (int i = 0; i < jumlahSoal; ++i) {
        cout << "\n=== Soal #" << (i + 1) << " ===\n";

        int tipe;
        do {
            cout << "Jenis soal (1 = Pilihan Ganda, 2 = Essay): ";
            cin >> tipe;
            cin.ignore();
            if (tipe != 1 && tipe != 2)
                cout << "Input harus 1 atau 2.\n";
        } while (tipe != 1 && tipe != 2);

        string pertanyaan;
        cout << "Pertanyaan: ";
        getline(cin, pertanyaan);

        /* ---------- PILIHAN GANDA ---------- */
        if (tipe == 1) {
            vector<string> opsiList;
            for (int j = 0; j < 4; ++j) {
                string opsi;
                do {
                    cout << "Opsi " << char('A' + j) << ": ";
                    getline(cin, opsi);
                    if (opsi.empty())
                        cout << "Opsi tidak boleh kosong.\n";
                } while (opsi.empty());
                opsiList.push_back(opsi);
            }

            string kunci;
            do {
                cout << "Kunci jawaban (A/B/C/D): ";
                getline(cin, kunci);
                if (!isValidKunciJawaban(kunci))
                    cout << "Masukkan hanya A, B, C, atau D.\n";
            } while (!isValidKunciJawaban(kunci));

            soalList.push_back({
                {"jenis", "pilihan"},
                {"pertanyaan", pertanyaan},
                {"opsi", opsiList},
                {"kunci", kunci}
            });
        }

        /* ---------- ESSAY ---------- */
        else {
            string kunciEssay;
            cout << "Kunci (jawaban contoh) ‑ boleh kosong: ";
            getline(cin, kunciEssay);

            json soal = {
                {"jenis", "essay"},
                {"pertanyaan", pertanyaan}
            };
            if (!kunciEssay.empty())
                soal["kunci"] = kunciEssay;

            soalList.push_back(soal);
        }
    }

    /* =======================================================
       SIMPAN KE DATABASE
    ======================================================= */
    json tugas = {
        {"id_materi", idMateri},
        {"parent_id", parentId},
        {"deadline", deadline},
        {"soal", soalList}
    };

    db["tugas"].push_back(tugas);
    cout << "\nTugas berhasil ditambahkan!\n";
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

    /* ---------- Daftar tugas ---------- */
    cout << "\n=== Daftar Tugas ===\n";
    for (size_t i = 0; i < db["tugas"].size(); ++i) {
        string idMateri = db["tugas"][i]["id_materi"];
        cout << i + 1 << ". Materi: ";
        printPathById(db["daftar_mata_pelajaran"], idMateri);
        cout << "   Deadline: " << db["tugas"][i]["deadline"] << '\n';
    }

    /* ---------- Pilih tugas ---------- */
    int index;
    cout << "\nPilih nomor tugas untuk melihat detail: ";
    cin >> index;
    cin.ignore();

    if (index < 1 || index > static_cast<int>(db["tugas"].size())) {
        cout << "Nomor tugas tidak valid.\n";
        return;
    }

    const json& tugas     = db["tugas"][index - 1];
    const json& soalList  = tugas["soal"];

    /* ---------- Header detail ---------- */
    cout << "\n=== Detail Tugas ===\n";
    cout << "Materi   : ";
    printPathById(db["daftar_mata_pelajaran"], tugas["id_materi"]);
    cout << "Deadline : " << tugas["deadline"] << '\n';
    cout << "Jumlah Soal: " << soalList.size() << "\n";

    /* ---------- Detail soal per nomor ---------- */
    for (size_t j = 0; j < soalList.size(); ++j) {
        const json& s   = soalList[j];
        string jenis    = s.value("jenis", "pilihan");   // default lama = pilihan

        cout << "\nSoal #" << j + 1 << " (" << (jenis == "essay" ? "Essay" : "Pilihan Ganda") << ")\n";
        cout << s["pertanyaan"] << '\n';

        if (jenis == "pilihan") {
            /* ---- tampilkan opsi A‑D ---- */
            const auto& opsi = s["opsi"];
            for (size_t k = 0; k < opsi.size(); ++k)
                cout << "  " << char('A' + k) << ". " << opsi[k] << '\n';

            cout << "Kunci Jawaban : " << s["kunci"] << '\n';
        } else { /* essay */
            if (s.contains("kunci") && !s["kunci"].get<string>().empty())
                cout << "Contoh Jawaban: " << s["kunci"] << '\n';
            else
                cout << "(Essay tidak memiliki kunci jawaban)\n";
        }
    }
}

bool isValidJawabanPilihan(const string& jawaban, size_t opsiSize) {
    if (jawaban.length() != 1) return false;
    char ch = toupper(jawaban[0]);
    int idx  = ch - 'A';
    return ch >= 'A' && ch <= 'Z' && idx < static_cast<int>(opsiSize);
}

void kerjakanTugas() {
    if (!db.contains("tugas") || db["tugas"].empty()) {
        cout << "Belum ada tugas yang tersedia.\n";
        return;
    }

    int user_id = db["session"]["user_id"];
    vector<int> mataUser = getMataPelajaranUser(user_id);

    /* ---------- Filter tugas relevan & belum dikerjakan ---------- */
    vector<int> tugasRelevanIndex;
    for (size_t i = 0; i < db["tugas"].size(); ++i) {
        const auto& tugas = db["tugas"][i];
        string parentId   = tugas["parent_id"];

        if (!idTerkaitDenganUser(db["daftar_mata_pelajaran"], parentId, mataUser))
            continue;

        bool sudah = false;
        for (const auto& j : db["jawaban"])
            if (j["user_id"] == user_id && j["id_tugas"] == i) { sudah = true; break; }

        if (!sudah) tugasRelevanIndex.push_back(i);
    }

    if (tugasRelevanIndex.empty()) {
        cout << "Tidak ada tugas relevan atau semua sudah dikerjakan.\n";
        return;
    }

    /* ---------- Tampilkan daftar ---------- */
    cout << "\n=== Daftar Tugas ===\n";
    for (size_t i = 0; i < tugasRelevanIndex.size(); ++i) {
        int idx = tugasRelevanIndex[i];
        cout << i + 1 << ". Materi: ";
        printPathById(db["daftar_mata_pelajaran"], db["tugas"][idx]["id_materi"]);
        cout << "   Deadline: " << db["tugas"][idx]["deadline"] << '\n';
    }

    int pilihan;
    cout << "\nPilih nomor tugas: ";
    cin >> pilihan; cin.ignore();
    if (pilihan < 1 || pilihan > static_cast<int>(tugasRelevanIndex.size())) {
        cout << "Pilihan tidak valid.\n"; return;
    }

    int tugasIndex      = tugasRelevanIndex[pilihan - 1];
    const json& tugas   = db["tugas"][tugasIndex];
    const json& soalArr = tugas["soal"];

    /* ---------- Kerjakan ---------- */
    int skor = 0, soalDinilai = 0, essayManual = 0;
    vector<string> jawabanUser;

    for (size_t i = 0; i < soalArr.size(); ++i) {
        const json& s   = soalArr[i];
        string jenis    = s.value("jenis", "pilihan");

        cout << "\nSoal #" << i + 1 << " (" << (jenis == "essay" ? "Essay" : "Pilihan") << "):\n";
        cout << s["pertanyaan"] << '\n';

        string jawaban;

        /* ----- pilihan ganda ----- */
        if (jenis == "pilihan") {
            const auto& opsi = s["opsi"];
            for (size_t o = 0; o < opsi.size(); ++o)
                cout << "  " << char('A' + o) << ". " << opsi[o] << '\n';

            do {
                cout << "Jawaban (A/B/C/D): ";
                getline(cin, jawaban);
                if (!isValidJawabanPilihan(jawaban, opsi.size()))
                    cout << "Input tidak valid.\n";
            } while (!isValidJawabanPilihan(jawaban, opsi.size()));

            char jChar = toupper(jawaban[0]);
            if (jChar == toupper(s["kunci"].get<string>()[0])) ++skor;
            ++soalDinilai;
            jawabanUser.push_back(string(1, jChar));
        }
        /* ----- essay ----- */
        else {
            cout << "Jawaban Anda (essay, baris tunggal): ";
            getline(cin, jawaban);
            jawabanUser.push_back(jawaban);

            if (s.contains("kunci") && !s["kunci"].get<string>().empty()) {
                /* perbandingan sederhana, case‑insensitive */
                string kunci = s["kunci"]; 
                transform(kunci.begin(), kunci.end(), kunci.begin(), ::tolower);
                string jtmp = jawaban; 
                transform(jtmp.begin(), jtmp.end(), jtmp.begin(), ::tolower);
                if (jtmp == kunci) ++skor;
                ++soalDinilai;
            } else {
                ++essayManual; /* belum bisa auto‑grade */
            }
        }
    }

    double nilai = soalDinilai ? (double)skor / soalDinilai * 100.0 : 0.0;
    cout << "\nNilai otomatis: " << fixed << setprecision(2) << nilai 
         << " (dari " << soalDinilai << " soal)\n";
    if (essayManual)
        cout << "Catatan: " << essayManual << " soal essay perlu penilaian manual.\n";

    /* ---------- Simpan ---------- */
    db["jawaban"].push_back({
        {"user_id", user_id},
        {"id_tugas", tugasIndex},
        {"jawaban" , jawabanUser},
        {"nilai"   , nilai}
    });
    saveDatabase("database.json");
    cout << "Jawaban tersimpan.\n";
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

void prosesAntrianTugas() {
    if (isEmpty()) {
        cout << "Antrian kosong. Tambahkan tugas terlebih dahulu.\n";
        return;
    }
    int user_id = db["session"]["user_id"];
    cin.ignore();

    while (!isEmpty()) {
        int tugasIndex   = dequeue();
        const json& tugas   = db["tugas"][tugasIndex];
        const json& soalArr = tugas["soal"];

        cout << "\n--- Mengerjakan Tugas ---\nMateri: ";
        printPathById(db["daftar_mata_pelajaran"], tugas["id_materi"]);
        cout << "Deadline: " << tugas["deadline"] << '\n';

        int skor = 0, soalDinilai = 0, essayManual = 0;
        vector<string> jawabanUser;

        for (size_t i = 0; i < soalArr.size(); ++i) {
            const json& s = soalArr[i];
            string jenis  = s.value("jenis", "pilihan");

            cout << "\nSoal #" << i + 1 << " (" << (jenis=="essay"?"Essay":"Pilihan") << "):\n";
            cout << s["pertanyaan"] << '\n';

            string jawaban;

            if (jenis == "pilihan") {
                const auto& opsi = s["opsi"];
                for (size_t o = 0; o < opsi.size(); ++o)
                    cout << "  " << char('A' + o) << ". " << opsi[o] << '\n';

                do {
                    cout << "Jawaban (A/B/…): ";
                    getline(cin, jawaban);
                    if (!isValidJawabanPilihan(jawaban, opsi.size()))
                        cout << "Input tidak valid.\n";
                } while (!isValidJawabanPilihan(jawaban, opsi.size()));

                char jChar = toupper(jawaban[0]);
                if (jChar == toupper(s["kunci"].get<string>()[0])) ++skor;
                ++soalDinilai;
                jawabanUser.push_back(string(1, jChar));
            } else {
                cout << "Jawaban Anda: ";
                getline(cin, jawaban);
                jawabanUser.push_back(jawaban);

                if (s.contains("kunci") && !s["kunci"].get<string>().empty()) {
                    string kunci = s["kunci"], jtmp = jawaban;
                    transform(kunci.begin(), kunci.end(), kunci.begin(), ::tolower);
                    transform(jtmp.begin(),  jtmp.end(),  jtmp.begin(),  ::tolower);
                    if (jtmp == kunci) ++skor;
                    ++soalDinilai;
                } else ++essayManual;
            }
        }

        double nilai = soalDinilai ? (double)skor / soalDinilai * 100.0 : 0.0;
        cout << "\nNilai otomatis: " << fixed << setprecision(2) << nilai << '\n';
        if (essayManual)
            cout << "Catatan: " << essayManual << " soal essay perlu penilaian manual.\n";

        db["jawaban"].push_back({
            {"user_id", user_id},
            {"id_tugas", tugasIndex},
            {"jawaban" , jawabanUser},
            {"nilai"   , nilai}
        });
        saveDatabase("database.json");
        cout << "Jawaban disimpan.\n";
    }
    cout << "\nSemua tugas di antrian selesai.\n";
}

int mainTugas() {
    const std::string filename = "database.json";
    if (!loadDatabase(filename)) return 1;

    std::string role = db["session"]["role"];
    int pilihan;

    do {
        clearCls();   // ──→ bersihkan layar sebelum menampilkan menu

        std::cout << "\n=== DASHBOARD " << (role == "admin" ? "ADMIN" : "USER") << " ===\n";

        if (role == "admin") {
            std::cout << "1. Tambah Tugas\n";
            std::cout << "2. Lihat Semua Tugas\n";
            std::cout << "3. Lihat Detail Tugas\n";
            std::cout << "4. Tampilkan Semua Judul Materi/SubMateri\n";
            std::cout << "5. Simpan dan Keluar\n";
        } else {
            std::cout << "1. Lihat Semua Tugas\n";
            std::cout << "2. Tampilkan Semua Judul Materi/SubMateri\n";
            std::cout << "3. Kerjakan Tugas\n";
            std::cout << "4. Fitur Antri Tugas\n";
            std::cout << "5. Lihat Detail tugas yang telah dikerjakan\n";
            std::cout << "6. Keluar\n";
        }

        std::cout << "Pilih: ";
        std::cin >> pilihan;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Input tidak valid.\n";
            pause();
            continue;   // kembali ke atas loop (akan clearCls lagi)
        }

        /* ----------------- ADMIN ----------------- */
        if (role == "admin") {
            switch (pilihan) {
                case 1:
                    tampilkanDaftarMateri(db["daftar_mata_pelajaran"]);
                    tambahTugas();
                    saveDatabase(filename);
                    break;

                case 2: tampilkanSemuaTugas();                     break;
                case 3: tampilkanDetailTugas();                    break;
                case 4: tampilkanDaftarMateri(db["daftar_mata_pelajaran"]); break;
                case 5: saveDatabase(filename); std::cout << "Data disimpan.\n"; break;
                default: std::cout << "Pilihan tidak valid.\n";
            }
        }
        /* ----------------- USER ------------------ */
        else {
            switch (pilihan) {
                case 1: tampilkanSemuaTugasUntukUser(db["session"]["user_id"]); break;
                case 2: tampilkanDaftarMateri(db["daftar_mata_pelajaran"]);      break;
                case 3: kerjakanTugas();                                         break;
                case 4: antriTugas();  prosesAntrianTugas();                     break;
                case 5: mainDetailTugasSelesai();                                break;
                case 6: std::cout << "Keluar...\n";                              break;
                default: std::cout << "Pilihan tidak valid.\n";
            }
        }

        /* ------ setelah menyelesaikan salah satu operasi ------ */
        if ((role == "admin" && pilihan != 5) || (role != "admin" && pilihan != 5)) {
            pause();        // beri kesempatan membaca hasil
        }

    } while ((role == "admin" && pilihan != 5) || (role != "admin" && pilihan != 5));

    return 0;
}