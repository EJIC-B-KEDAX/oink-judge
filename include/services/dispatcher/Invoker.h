#pragma once
#include "socket/Session.hpp"

#include <memory>

namespace oink_judge::services::dispatcher {

using Session = socket::Session;

class Invoker {
  public:
    Invoker(std::string identificator, std::shared_ptr<Session> session);

    virtual ~Invoker() = default;

    [[nodiscard]] const std::string& get_id() const;

    void send_message(const std::string& message);

    [[nodiscard]] virtual bool can_test_submission(const std::string& submission_id) const;

  private:
    std::string _id;
    std::shared_ptr<Session> _session;
};

} // namespace oink_judge::services::dispatcher
