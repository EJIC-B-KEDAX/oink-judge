#include "oink_judge/content_service/server/content_service.h"

#include "oink_judge/content_service/config_utils.h"
#include "oink_judge/content_service/manifest_storage.h"

#include <oink_judge/config/common_utils.h>
#include <oink_judge/logger/logger.h>
#include <oink_judge/utils/filesystem.h>

#include <algorithm>
#include <filesystem>
#include <ios>
#include <optional>
#include <sstream>
#include <string>

namespace oink_judge::content_service {

namespace {

auto getPathToContent(const std::string& content_type, const std::string& content_id) -> std::optional<fs::path> {
    std::optional<fs::path> base_directory_opt = getContentDirectory(content_type);
    if (!base_directory_opt) {
        return std::nullopt;
    }
    const fs::path& base_directory = *base_directory_opt;
    fs::path content_path = base_directory / content_id;
    if (!fs::exists(content_path)) {
        return std::nullopt;
    }
    return content_path;
}

auto getFullPath(const std::string& content_type, const std::string& content_id, const fs::path& file_path)
    -> std::optional<fs::path> {
    std::optional<fs::path> content_path_opt = getPathToContent(content_type, content_id);
    if (!content_path_opt) {
        return std::nullopt;
    }
    const fs::path& content_path = *content_path_opt;
    fs::path full_file_path = content_path / file_path;
    if (!fs::exists(full_file_path)) {
        return std::nullopt;
    }
    return full_file_path;
}

auto checkFullFilePath(const std::string& content_type, const std::string& content_id, const std::string& file_path,
                       bool must_exist = true) -> grpc::Status {
    std::string error_message;
    if (content_type.empty() || content_id.empty() || file_path.empty()) {
        error_message = "Content type, content id and file path must be provided";
        return {grpc::StatusCode::INVALID_ARGUMENT, error_message};
    }
    if (fs::path(file_path).is_absolute()) {
        error_message = "File path must be relative";
        return {grpc::StatusCode::INVALID_ARGUMENT, error_message};
    }
    std::optional<fs::path> content_path_opt = getPathToContent(content_type, content_id);
    if (!content_path_opt) {
        error_message = "Content not found";
        return {grpc::StatusCode::NOT_FOUND, error_message};
    }
    auto canonical = fs::weakly_canonical(*content_path_opt / file_path);
    auto content_canonical = fs::canonical(*content_path_opt);
    auto distance_content = std::distance(content_canonical.begin(), content_canonical.end());
    auto distance_canonical = std::distance(canonical.begin(), canonical.end());
    if (distance_content > distance_canonical || !std::equal(content_canonical.begin(), content_canonical.end(),
                                                             canonical.begin(), std::next(canonical.begin(), distance_content))) {
        error_message = "File path must not contain '..'";
        return {grpc::StatusCode::INVALID_ARGUMENT, error_message};
    }

    if (!must_exist) {
        return grpc::Status::OK;
    }

    std::optional<fs::path> full_file_path_opt = getFullPath(content_type, content_id, file_path);
    if (!full_file_path_opt) {
        error_message = "File not found";
        return {grpc::StatusCode::NOT_FOUND, error_message};
    }
    const fs::path& full_file_path = *full_file_path_opt;
    if (!fs::is_regular_file(full_file_path)) {
        error_message = "Path is not a file";
        return {grpc::StatusCode::INVALID_ARGUMENT, error_message};
    }
    return grpc::Status::OK;
}

} // namespace

// NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
auto getManifestHandler(GetManifestRPC& rpc, GetManifestRequest& request) -> awaitable<void> {
    std::string error_message;
    if (request.content_type().empty() || request.content_id().empty()) {
        error_message = "Content type and content id must be provided";
        logger::logWarning("content_service", error_message);
        co_await rpc.finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, error_message));
        co_return;
    }
    std::optional<fs::path> content_path_opt = getPathToContent(request.content_type(), request.content_id());
    if (!content_path_opt) {
        error_message = "Content not found";
        logger::logWarning("content_service", error_message + ": " + request.content_type() + "/" + request.content_id());
        co_await rpc.finish(grpc::Status(grpc::StatusCode::NOT_FOUND, error_message));
        co_return;
    }

    try {
        constexpr auto CHUNK_SIZE = static_cast<size_t>(64 * 1024); // 64KB
        std::vector<char> buffer(CHUNK_SIZE);
        const std::string& manifest =
            ManifestStorage::instance().getManifest(request.content_type(), request.content_id()).toString();
        std::istringstream manifest_stream(manifest);
        while (manifest_stream) {
            manifest_stream.read(buffer.data(), CHUNK_SIZE);
            std::streamsize bytes_read = manifest_stream.gcount();
            if (bytes_read == 0) {
                break;
            }
            if (bytes_read > 0) {
                GetManifestResponse response_chunk;
                response_chunk.set_manifest(std::string(buffer.data(), static_cast<size_t>(bytes_read)));
                co_await rpc.write(response_chunk);
            }
        }
        co_await rpc.finish(grpc::Status::OK);
        co_return;
    } catch (const std::exception& e) {
        error_message = e.what();
        logger::logError("content_service", "Failed to get manifest for " + request.content_type() + "/" + request.content_id() +
                                                ": " + error_message);
    }
    co_await rpc.finish(grpc::Status(grpc::StatusCode::INTERNAL, error_message));
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
auto getFileHandler(GetFileRPC& rpc, GetFileRequest& request) -> awaitable<void> {
    grpc::Status check_status = checkFullFilePath(request.content_type(), request.content_id(), request.file_path());
    if (!check_status.ok()) {
        logger::logWarning("content_service", check_status.error_message());
        co_await rpc.finish(check_status);
        co_return;
    }

    std::string error_message;
    try {
        constexpr auto BUFFER_SIZE = static_cast<size_t>(64 * 1024); // 64KB
        std::vector<char> buffer(BUFFER_SIZE);
        std::string full_file_path = *getFullPath(request.content_type(), request.content_id(), request.file_path());
        // GetFileResponse response;
        std::istringstream file_stream(utils::filesystem::loadFile(full_file_path), std::ios::binary);
        while (file_stream) {
            file_stream.read(buffer.data(), BUFFER_SIZE);
            std::streamsize bytes_read = file_stream.gcount();
            if (bytes_read == 0) {
                break;
            }
            if (bytes_read > 0) {
                GetFileResponse response_chunk;
                auto& file_chunk = *response_chunk.mutable_file_chunk();
                file_chunk.set_data(std::string(buffer.data(), static_cast<size_t>(bytes_read)));
                co_await rpc.write(response_chunk);
            }
        }
        co_await rpc.finish(grpc::Status::OK);
        co_return;
    } catch (const std::exception& e) {
        error_message = e.what();
        logger::logError("content_service", "Failed to get file " + request.file_path() + " for " + request.content_type() + "/" +
                                                request.content_id() + ": " + error_message);
    }
    co_await rpc.finish(grpc::Status(grpc::StatusCode::INTERNAL, error_message));
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
auto createFileHandler(CreateFileRPC& rpc) -> awaitable<void> {
    CreateFileRequest info_request;
    co_await rpc.read(info_request);

    if (!info_request.has_file_info()) {
        std::string error_message = "First message must contain file info";
        logger::logWarning("content_service", error_message);
        co_await rpc.finish_with_error(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, error_message));
        co_return;
    }
    const FileInfo& info = info_request.file_info();

    grpc::Status check_status = checkFullFilePath(info.content_type(), info.content_id(), info.file_path(), false);
    if (!check_status.ok()) {
        logger::logWarning("content_service", check_status.error_message());
        co_await rpc.finish_with_error(check_status);
        co_return;
    }

    std::string error_message;
    try {
        fs::path full_file_path = *getPathToContent(info.content_type(), info.content_id()) / info.file_path();
        if (fs::exists(full_file_path)) {
            error_message = "File already exists";
            logger::logWarning("content_service", error_message + ": " + info.file_path());
            co_await rpc.finish_with_error(grpc::Status(grpc::StatusCode::ALREADY_EXISTS, error_message));
            co_return;
        }
        std::ostringstream file_content_stream(std::ios::binary);
        CreateFileRequest chunk_request;
        while (co_await rpc.read(chunk_request)) {
            if (!chunk_request.has_file_chunk()) {
                error_message = "Subsequent messages must contain file chunks";
                co_await rpc.finish_with_error(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, error_message));
                co_return;
            }
            const FileChunk& chunk = chunk_request.file_chunk();
            file_content_stream.write(chunk.data().data(), static_cast<std::streamsize>(chunk.data().size()));
        }
        utils::filesystem::storeFile(full_file_path, file_content_stream.str());
        logger::logInfo("content_service",
                        "Created file " + info.file_path() + " for " + info.content_type() + "/" + info.content_id(), 2);
        co_await rpc.finish(google::protobuf::Empty{}, grpc::Status::OK);
        co_return;
    } catch (const std::exception& e) {
        error_message = e.what();
        logger::logError("content_service", "Failed to create file: " + error_message);
    }
    co_await rpc.finish_with_error(grpc::Status(grpc::StatusCode::INTERNAL, error_message));
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
auto updateFileHandler(UpdateFileRPC& rpc) -> awaitable<void> {
    UpdateFileRequest info_request;
    co_await rpc.read(info_request);
    if (!info_request.has_file_info()) {
        std::string error_message = "First message must contain file info";
        logger::logWarning("content_service", error_message);
        co_await rpc.finish_with_error(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, error_message));
        co_return;
    }
    const FileInfo& info = info_request.file_info();
    grpc::Status check_status = checkFullFilePath(info.content_type(), info.content_id(), info.file_path());
    if (!check_status.ok()) {
        logger::logWarning("content_service", check_status.error_message());
        co_await rpc.finish_with_error(check_status);
        co_return;
    }

    std::string error_message;
    try {
        fs::path full_file_path = *getPathToContent(info.content_type(), info.content_id()) / info.file_path();
        std::ostringstream file_content_stream(std::ios::binary);
        UpdateFileRequest chunk_request;
        while (co_await rpc.read(chunk_request)) {
            if (!chunk_request.has_file_chunk()) {
                error_message = "Subsequent messages must contain file chunks";
                co_await rpc.finish_with_error(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, error_message));
                co_return;
            }
            const FileChunk& chunk = chunk_request.file_chunk();
            file_content_stream.write(chunk.data().data(), static_cast<std::streamsize>(chunk.data().size()));
        }
        utils::filesystem::storeFile(full_file_path, file_content_stream.str());
        logger::logInfo("content_service",
                        "Updated file " + info.file_path() + " for " + info.content_type() + "/" + info.content_id(), 2);
        co_await rpc.finish(google::protobuf::Empty{}, grpc::Status::OK);
        co_return;
    } catch (const std::exception& e) {
        error_message = e.what();
        logger::logError("content_service", "Failed to update file: " + error_message);
    }
    co_await rpc.finish_with_error(grpc::Status(grpc::StatusCode::INTERNAL, error_message));
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
auto deleteFileHandler(DeleteFileRPC& rpc, DeleteFileRequest& request) -> awaitable<void> {
    grpc::Status check_status = checkFullFilePath(request.content_type(), request.content_id(), request.file_path());
    if (!check_status.ok()) {
        logger::logWarning("content_service", check_status.error_message());
        co_await rpc.finish_with_error(check_status);
        co_return;
    }

    std::string error_message;
    try {
        fs::path full_file_path = *getFullPath(request.content_type(), request.content_id(), request.file_path());
        fs::remove(full_file_path);
        logger::logInfo("content_service",
                        "Deleted file " + request.file_path() + " for " + request.content_type() + "/" + request.content_id(), 2);
        co_await rpc.finish(google::protobuf::Empty{}, grpc::Status::OK);
        co_return;
    } catch (const std::exception& e) {
        error_message = e.what();
        logger::logError("content_service", "Failed to delete file: " + error_message);
    }
    co_await rpc.finish_with_error(grpc::Status(grpc::StatusCode::INTERNAL, error_message));
}

} // namespace oink_judge::content_service
