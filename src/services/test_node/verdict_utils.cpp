#include "services/test_node/verdict_utils.h"
#include "services/test_node/DefaultVerdict.h"
#include <fstream>

namespace oink_judge::services::test_node {

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

std::shared_ptr<Verdict> load_verdict_from_meta(const std::string &test_name, const std::string &path_to_meta) {
    auto verdict = std::make_shared<DefaultVerdict>(test_name);

    MetaInfo meta = parse_meta_file(path_to_meta);

    if (meta.status == "OK") {
        verdict->set_info({
            {VerdictType::ACCEPTED,
            "Accepted",
            "AC"},
            100,
            meta.time.value(),
            static_cast<double>(meta.max_rss.value())
        });
    } else {
        VerdictType type;
        type.type = VerdictType::WRONG;
        if (meta.status == "TO") {
            type.full_name = "Time limit exceeded";
            type.short_name = "TL";
        } else if (meta.status == "RE") {
            type.full_name = "Runtime error";
            type.short_name = "RE";
        } else if (meta.status == "SG") {
            type.full_name = "Memory limit exceeded";
            type.short_name = "ML";
        } else if (meta.status == "IL") {
            type.full_name = "Idle limit exceeded";
            type.short_name = "IL";
        } else {
            type.full_name = "Security Violation";
            type.short_name = "SV";
        }
        verdict->set_info({
            type,
            0,
            meta.time.value(),
            static_cast<double>(meta.max_rss.value())
        });
    }

    return verdict;
}

std::shared_ptr<Verdict> load_verdict_from_checker_output(const std::string &test_name, const std::string &path_to_meta, const std::string &path_to_checker_output) {
    auto verdict = std::make_shared<DefaultVerdict>(test_name);

    MetaInfo meta = parse_meta_file(path_to_meta);

    if (meta.status == "OK") {
        std::ifstream file(path_to_checker_output);
        std::string line;

        VerdictType type;

        while (std::getline(file, line)) {
            if (line[0] == 'o') {
                type.type = VerdictType::ACCEPTED;
                type.full_name = "Accepted";
                type.short_name = "AC";
            } else if (line[0] == 'w') {
                type.type = VerdictType::WRONG;
                type.full_name = "Wrong answer";
                type.short_name = "WA";
            } else if (line[0] == 'p') {
                type.type = VerdictType::WRONG;
                type.full_name = "Presentation error";
                type.short_name = "PE";
            } else {
                type.type = VerdictType::FAILED;
                type.full_name = "FAIL";
                type.short_name = "FAIL";
            }
        }
        double score = 0;
        if (type.short_name == "AC") score = 100;
        verdict->set_info({
            type,
            score,
            meta.time.value(),
            static_cast<double>(meta.max_rss.value())
        });

        return verdict;
    }
    
    return load_verdict_from_meta(test_name, path_to_meta);
}


} // namespace oink_judge::services::test_node
