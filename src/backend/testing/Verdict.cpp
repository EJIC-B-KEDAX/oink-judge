#include "backend/testing/Verdict.h"
#include <optional>
#include <sstream>
#include <fstream>

namespace oink_judge::backend::testing {

namespace {
    struct MetaInfo {
        std::string status;
        std::optional<double> time;
        std::optional<int> max_rss;
        std::optional<int> exit_code;
        std::optional<int> exit_signal;
    };

    MetaInfo parse_meta_file(const std::string& path) {
        MetaInfo info;
        std::ifstream file(path);
        std::string line;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string key;
            if (std::getline(iss, key, ':')) {
                std::string value;
                std::getline(iss, value);
                // Удаляем ведущие пробелы
                value.erase(0, value.find_first_not_of(" \t"));

                if (key == "status") {
                    info.status = value;
                } else if (key == "time") {
                    info.time = std::stod(value);
                } else if (key == "max-rss") {
                    info.max_rss = std::stoi(value);
                } else if (key == "exitcode") {
                    info.exit_code = std::stoi(value);
                } else if (key == "exitsig") {
                    info.exit_signal = std::stoi(value);
                }
            }
        }
        if (info.status.empty()) {
            info.status = "OK";
        }

        return info;
    }


} // namespace

void Verdict::set_score(double score) {
    _score = score;
}

void Verdict::set_time_used(double time_used) {
    _time_used = time_used;
}

void Verdict::set_memory_used(double memory_used) {
    _memory_used = memory_used;
}

void Verdict::set_type(Type type) {
    _type = type;
}

void Verdict::set_score_merge_policy(ScoreMergePolicy policy) {
    _score_merge_policy = policy;
}

double Verdict::get_score() const {
    return _score;
}

double Verdict::get_time_used() const {
    return _time_used;
}

double Verdict::get_memory_used() const {
    return _memory_used;
}

Verdict::Type Verdict::get_type() const {
    return _type;
}

Verdict::ScoreMergePolicy Verdict::get_score_merge_policy() const {
    return _score_merge_policy;
}

Verdict load_verdict_from_meta(const std::string &path_to_meta) {
    Verdict verdict;

    MetaInfo meta = parse_meta_file(path_to_meta);

    if (meta.status == "OK") {
        verdict.set_type(Verdict::Type::OK);
        verdict.set_score(100);
    } else {
        verdict.set_score(0);
        if (meta.status == "TL") {
            verdict.set_type(Verdict::Type::TL);
        } else if (meta.status == "RE") {
            verdict.set_type(Verdict::Type::RE);
        } else if (meta.status == "ML") {
            verdict.set_type(Verdict::Type::ML);
        } else if (meta.status == "IL") {
            verdict.set_type(Verdict::Type::IL);
        } else {
            verdict.set_type(Verdict::Type::SV);
        }
    }

    verdict.set_score_merge_policy(Verdict::ScoreMergePolicy::Minimum);
    verdict.set_time_used(meta.time.value());
    verdict.set_memory_used(meta.max_rss.value());

    return verdict;
}

Verdict load_verdict_from_checker_output(const std::string &path_to_meta, const std::string &path_to_checker_output) {
    Verdict verdict;

    MetaInfo meta = parse_meta_file(path_to_meta);

    if (meta.status == "OK") {
        std::ifstream file(path_to_checker_output);
        std::string line;

        while (std::getline(file, line)) {
            if (line[0] == 'o') {
                verdict.set_type(Verdict::Type::OK);
                verdict.set_score(0);
            } else if (line[0] == 'w') {
                verdict.set_type(Verdict::Type::WA);
                verdict.set_score(0);
            } else if (line[0] == 'p') {
                verdict.set_type(Verdict::Type::PE);
                verdict.set_score(0);
            } else {
                verdict.set_type(Verdict::Type::FAIL);
                verdict.set_score(0);
            }
        }
    } else {
        return load_verdict_from_meta(path_to_meta);
    }

    verdict.set_score_merge_policy(Verdict::ScoreMergePolicy::Minimum);
    verdict.set_time_used(meta.time.value());
    verdict.set_memory_used(meta.max_rss.value());

    return verdict;
}

Verdict operator+(const Verdict &verdict1, const Verdict &verdict2) {
    Verdict verdict;

    if (verdict1.get_type() == verdict2.get_type() && verdict1.get_type() == Verdict::Type::OK) {
        verdict.set_type(Verdict::Type::OK);
    } else if (verdict1.get_type() == Verdict::Type::OK) {
        verdict.set_type(verdict2.get_type());
    } else {
        verdict.set_type(verdict1.get_type());
    }

    verdict.set_score(std::min(verdict1.get_score(), verdict2.get_score()));
    verdict.set_time_used(std::max(verdict1.get_time_used(), verdict2.get_time_used()));
    verdict.set_memory_used(std::max(verdict1.get_memory_used(), verdict2.get_memory_used()));

    verdict.set_score_merge_policy(verdict1.get_score_merge_policy());

    return verdict;
}

} // namespace oink_judge::backend::testing
