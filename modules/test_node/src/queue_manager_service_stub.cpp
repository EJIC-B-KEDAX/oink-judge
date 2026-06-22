#include "oink_judge/test_node/queue_manager_service_stub.h"

#include "oink_judge/test_node/test_storage.h"

#include <oink_judge/content_service/client/content_storage.h>
#include <oink_judge/database/async_table_submissions.h>
#include <oink_judge/logger/logger.h>
#include <oink_judge/utils/grpc/factories.hpp>

#include <agrpc/client_rpc.hpp>
#include <agrpc/grpc_context.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <nlohmann/json.hpp>

namespace oink_judge::test_node {

using ConnectRPC = agrpc::ClientRPC<&QueueManagerService::Stub::PrepareAsyncConnect>;
using database::AsyncTableSubmissions;
using nlohmann::json;

QueueManagerServiceStub::QueueManagerServiceStub(std::shared_ptr<grpc::Channel> channel) : channel_(std::move(channel)) {
    stub_ = QueueManagerService::NewStub(channel_);
}

auto QueueManagerServiceStub::connect(std::string test_node_id, std::string test_node_type)
    -> awaitable<tl::expected<void, grpc::Status>> {
    auto& grpc_context = static_cast<agrpc::GrpcContext&>((co_await boost::asio::this_coro::executor).context()); // NOLINT

    ClientMessage handshake_message;
    auto* handshake = handshake_message.mutable_handshake();
    handshake->set_node_id(std::move(test_node_id));
    handshake->set_node_type(std::move(test_node_type));

    auto rpc = ConnectRPC(grpc_context);
    co_await rpc.start(*stub_);
    co_await rpc.write(handshake_message, boost::asio::use_awaitable);

    while (true) {
        ServerMessage request_message;
        co_await rpc.read(request_message, boost::asio::use_awaitable);
        if (!request_message.has_test_submission_request()) {
            auto status = co_await rpc.finish(boost::asio::use_awaitable);
            if (!status.ok()) {
                co_return tl::unexpected(status);
            }
            co_return tl::unexpected(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid request from queue manager"));
        }
        const TestSubmissionRequest& test_submission_request = request_message.test_submission_request();
        std::string submission_id = test_submission_request.submission_id();
        logger::logInfo("test_node", "Received submission " + submission_id + " for testing");
        std::string problem_id = co_await AsyncTableSubmissions::instance().problemOfSubmission(submission_id);
        auto test = co_await TestStorage::instance().getTest(problem_id);
        if (!test) {
            logger::logError("test_node", "Test not found for problem " + problem_id);
            auto status = co_await rpc.finish(boost::asio::use_awaitable);
            if (!status.ok()) {
                co_return tl::unexpected(status);
            }
            co_return tl::unexpected(grpc::Status(grpc::StatusCode::NOT_FOUND, "Test not found for problem_id: " + problem_id));
        }

        std::vector<std::string> boxes;
        size_t boxes_required = test->boxesRequired();
        boxes.reserve(boxes_required);
        for (int i = 0; i < boxes_required; ++i) {
            boxes.push_back(std::to_string(i));
        }
        co_await content_service::ContentStorage::instance().ensureContentExists("submission", submission_id);
        co_await test->run(submission_id, boxes, json::object());
        co_await content_service::ContentStorage::instance().updateContentOnServer("submission", submission_id);
        logger::logInfo("test_node", "Finished testing submission " + submission_id);

        ClientMessage response_message;
        auto* response = response_message.mutable_test_submission_response();
        co_await rpc.write(response_message, boost::asio::use_awaitable);
    }

    grpc::Status status = co_await rpc.finish(boost::asio::use_awaitable);
    if (!status.ok()) {
        co_return tl::unexpected(status);
    }
    co_return tl::expected<void, grpc::Status>{};
}

auto registerQueueManagerServiceStubType() -> void {
    QueueManagerServiceStubFactory::instance().registerType(
        QueueManagerServiceStub::REGISTERED_NAME, [](const std::string& params) -> std::unique_ptr<QueueManagerServiceStub> {
            std::vector<std::string> parts = factory::parseParameters(params, ",");
            if (parts.size() != 1) {
                throw std::invalid_argument("Expected exactly one parameter for queue_manager_service_stub: channel type");
            }
            std::string channel_type = factory::normalizeArgument(parts[0], true);
            auto channel = utils::grpc::ChannelFactory::instance().create(channel_type);
            return std::make_unique<QueueManagerServiceStub>(std::move(channel));
        });
}

} // namespace oink_judge::test_node
