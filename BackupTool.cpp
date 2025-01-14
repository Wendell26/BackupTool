#include <windows.h>
#include <shlobj.h>
#include <commdlg.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <zip.h>

namespace fs = std::filesystem;

void addFileToZip(zip_t* archive, const fs::path& filePath, const std::string& baseDir) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return;

    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::string relativePath = filePath.string().substr(baseDir.size());

    zip_source_t* source = zip_source_buffer(archive, fileContent.data(), fileContent.size(), 0);
    if (!source || zip_file_add(archive, relativePath.c_str(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_GUESS) < 0) {
        zip_source_free(source);
    }
}

void createZipBackup(const std::string& folderPath, const std::string& zipPath) {
    zip_t* archive = zip_open(zipPath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, nullptr);
    if (!archive) {
        MessageBox(nullptr, "Error creating zip file!", "Error", MB_ICONERROR);
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            addFileToZip(archive, entry.path(), folderPath);
        }
    }

    zip_close(archive);
    MessageBox(nullptr, "Backup completed successfully!", "Success", MB_ICONINFORMATION);
}

std::string selectFolder() {
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = "Select a folder to backup";
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

    if (pidl != nullptr) {
        char path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path)) {
            return std::string(path);
        }
    }
    return "";
}

std::string selectSaveFile() {
    OPENFILENAME ofn;
    char fileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "ZIP Files\0*.zip\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Select a location to save the backup";
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
        return std::string(fileName);
    }
    return "";
}

int main() {
    std::string folderPath = selectFolder();
    if (folderPath.empty()) {
        MessageBox(nullptr, "No folder selected!", "Error", MB_ICONERROR);
        return 1;
    }

    std::string zipPath = selectSaveFile();
    if (zipPath.empty()) {
        MessageBox(nullptr, "No save location selected!", "Error", MB_ICONERROR);
        return 1;
    }

    createZipBackup(folderPath, zipPath);
    return 0;
}
