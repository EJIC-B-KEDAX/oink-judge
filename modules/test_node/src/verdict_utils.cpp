#include "oink_judge/test_node/verdict_utils.h"

#include "oink_judge/test_node/verdicts/default_verdict.h"

#include <fstream>

namespace oink_judge::test_node {

namespace {

struct MetaInfo {
    std::string status;
    std::optional<double> time;
    std::optional<int> max_rss;
    std::optional<int> exit_code;
    std::optional<int> exit_signal;
};

auto parseMetaFile(const std::string& path) -> MetaInfo {
    MetaInfo info;
    std::ifstream file(path);
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, ':')) {
            std::string value;
            std::getline(iss, value);
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

auto loadVerdictFromMeta(const std::string& test_name, const std::string& path_to_meta) -> std::shared_ptr<Verdict> {
    auto verdict = std::make_shared<DefaultVerdict>(test_name);

    MetaInfo meta = parseMetaFile(path_to_meta);

    if (meta.status == "OK") {
        verdict->setInfo({.type = VerdictType::ACCEPTED,
                          .score = 100, // NOLINT // TODO: make configurable score
                          .time_used = meta.time.value(),
                          .memory_used = static_cast<double>(meta.max_rss.value())});
    } else {
        VerdictType type;
        if (meta.status == "TO") {
            type = VerdictType::TIME_LIMIT_EXCEEDED;
        } else if (meta.status == "RE") {
            type = VerdictType::RUNTIME_ERROR;
        } else if (meta.status == "SG") {
            type = VerdictType::MEMORY_LIMIT_EXCEEDED;
        } else if (meta.status == "IL") {
            type = VerdictType::IDLE_LIMIT_EXCEEDED;
        } else {
            type = VerdictType::SECURITY_VIOLATION;
        }
        verdict->setInfo(
            {.type = type, .score = 0, .time_used = meta.time.value(), .memory_used = static_cast<double>(meta.max_rss.value())});
    }

    return verdict;
}

auto loadVerdictFromCheckerOutput(const std::string& test_name, const std::string& path_to_meta,
                                  const std::string& path_to_checker_output) -> std::shared_ptr<Verdict> {
    auto verdict = std::make_shared<DefaultVerdict>(test_name);

    MetaInfo meta = parseMetaFile(path_to_meta);

    if (meta.status == "OK") {
        std::ifstream file(path_to_checker_output);
        std::string line;

        VerdictType type;

        while (std::getline(file, line)) {
            if (line[0] == 'o') {
                type = VerdictType::ACCEPTED;
            } else if (line[0] == 'w') {
                type = VerdictType::WRONG_ANSWER;
            } else if (line[0] == 'p') {
                type = VerdictType::PRESENTATION_ERROR;
            } else {
                type = VerdictType::FAILED;
            }
        }
        double score = 0;
        if (type.short_name == "AC") {
            score = 100; // NOLINT
        }
        verdict->setInfo({.type = type,
                          .score = score,
                          .time_used = meta.time.value(),
                          .memory_used = static_cast<double>(meta.max_rss.value())});

        return verdict;
    }

    return loadVerdictFromMeta(test_name, path_to_meta);
}

} // namespace oink_judge::test_node
