#include "../include/ConfigUtils.h"
#include "../external/json.hpp"
#include <cwctype>
#include <codecvt>
#include <locale>
#include <fstream>
#include <iostream>
#include <algorithm>

using json = nlohmann::json;

std::string wstring_to_utf8(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
}

std::wstring utf8_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(str);
}

std::wstring normalize_dash(const std::wstring& input) {
    std::wstring result = input;
    std::replace(result.begin(), result.end(), L'\u2014', L'-');
    std::replace(result.begin(), result.end(), L'\u2013', L'-');
    return result;
}

std::wstring to_lower(const std::wstring& input) {
    std::wstring result = input;
    std::locale loc;
    for (wchar_t& ch : result) {
        ch = std::towlower(ch);
    }
    return result;
}

bool is_valid_word(const std::wstring& word) {
    if (word.empty() || word.size() > 100) return false;

    for (wchar_t ch : word) {
        if (!std::iswalpha(ch)) {
            return false;
        }
    }
    return true;
}

bool load_config(const std::string& filename,
                 std::vector<std::wstring>& out_files,
                 std::vector<std::wstring>& out_queries) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        std::wcerr << L"Cannot open config file: " << utf8_to_wstring(filename) << L"\n";
        return false;
    }

    json j;
    try {
        ifs >> j;
        if (j.contains("files") && j["files"].is_array()) {
            for (auto& f : j["files"]) {
                std::wstring w = utf8_to_wstring(f.get<std::string>());
                w = to_lower(normalize_dash(w));
                if (is_valid_word(w)) {
                    out_files.push_back(w);
                } else {
                    // Можно логировать или просто игнорировать
                    std::wcerr << L"Ignored invalid file word: " << w << L"\n";
                }
            }
        }
        if (j.contains("requests") && j["requests"].is_array()) {
            for (auto& q : j["requests"]) {
                std::wstring w = utf8_to_wstring(q.get<std::string>());
                w = to_lower(normalize_dash(w));
                if (is_valid_word(w)) {
                    out_queries.push_back(w);
                } else {
                    std::wcerr << L"Ignored invalid request word: " << w << L"\n";
                }
            }
        }
    } catch (const std::exception& e) {
        std::wcerr << L"Error parsing config.json: " << utf8_to_wstring(e.what()) << L"\n";
        return false;
    }

    return true;
}

bool save_config(const std::string& filename,
                 const std::vector<std::wstring>& files,
                 const std::vector<std::wstring>& queries) {
    json j;
    std::vector<std::string> files_utf8;
    for (const auto& f : files) {
        files_utf8.push_back(wstring_to_utf8(f));
    }
    std::vector<std::string> queries_utf8;
    for (const auto& q : queries) {
        queries_utf8.push_back(wstring_to_utf8(q));
    }
    j["files"] = files_utf8;
    j["requests"] = queries_utf8;

    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        std::wcerr << L"Cannot open config file for writing: " << utf8_to_wstring(filename) << L"\n";
        return false;
    }
    ofs << j.dump(4);
    return true;
}

