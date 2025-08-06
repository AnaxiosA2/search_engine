#pragma once

struct Entry {
    size_t doc_id;
    size_t count;

    bool operator==(const Entry& other) const = default;
};
