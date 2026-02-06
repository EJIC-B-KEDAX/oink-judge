#pragma once
#include <memory>
#include <oink_judge/socket/session.hpp>

namespace oink_judge::dispatcher {

using Session = socket::Session;

class Invoker {
  public:
    Invoker(std::string id, std::shared_ptr<Session> session);

    Invoker(const Invoker&) = delete;
    auto operator=(const Invoker&) -> Invoker& = delete;
    Invoker(Invoker&&) = delete;
    auto operator=(Invoker&&) -> Invoker& = delete;
    virtual ~Invoker() = default;

    [[nodiscard]] auto getId() const -> const std::string&;

    auto sendMessage(const std::string& message) -> void;

    [[nodiscard]] virtual auto canTestSubmission(const std::string& submission_id) const -> bool;

  private:
    std::string id_;
    std::shared_ptr<Session> session_;
};

} // namespace oink_judge::dispatcher
