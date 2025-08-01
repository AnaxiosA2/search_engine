#pragma once

#include <string>
#include <vector>
#include "RelativeIndex.h"

class ConverterJSON {
public:
    ConverterJSON() = default;

    bool SaveRequests(const std::string& filename) const;
    void SetRequests(const std::vector<std::string>& requests);

    // Загружает конфиг
    bool LoadConfig(const std::string& filename, std::string& error);

    // Загружает запросы
    bool LoadRequests(const std::string& filename, std::string& error);

    // Возвращает загруженные документы
    const std::vector<std::string>& GetTextDocuments() const;

    // Возвращает загруженные запросы
    const std::vector<std::string>& GetRequests() const;

    // Возвращает лимит ответов max_responses из конфига, по умолчанию 5
    int GetResponsesLimit() const;

    // Проверяет, совпадает ли версия из конфига с версией приложения
    bool CheckConfigVersion(const std::string& app_version) const;

    // Сохраняет ответы в файл answers.json
    bool SaveAnswers(const std::string& filename,
                     const std::vector<std::string>& requests,
                     const std::vector<std::vector<RelativeIndex>>& answers) const;

    // Сохраняет ответы в answers.json (по умолчанию)
    bool putAnswers(const std::vector<std::string>& requests,
                    const std::vector<std::vector<RelativeIndex>>& answers) const;

private:
    std::vector<std::wstring> file_paths_w;
    std::vector<std::string> text_documents_;
    std::vector<std::string> requests_;
    int max_responses_ = 5;
    std::string config_version_;
};
