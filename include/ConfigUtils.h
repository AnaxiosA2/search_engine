#pragma once

#include <string>
#include <vector>

bool load_config(const std::string& filename,
                 std::vector<std::wstring>& out_files,
                 std::vector<std::wstring>& out_queries);

bool save_config(const std::string& filename,
                 const std::vector<std::wstring>& files,
                 const std::vector<std::wstring>& queries);

std::wstring utf8_to_wstring(const std::string& str);
std::string wstring_to_utf8(const std::wstring& wstr);
std::wstring normalize_dash(const std::wstring& input);

std::wstring to_lower(const std::wstring& input);
