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
