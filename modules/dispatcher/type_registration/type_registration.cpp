#include <oink_judge/dispatcher/protocol_with_fastapi.h>
#include <oink_judge/dispatcher/protocol_with_invoker.h>
#include <oink_judge/dispatcher/send_submission_to_invoker.h>

using namespace oink_judge::dispatcher;

extern "C" auto registerTypes() -> void {
    registerProtocolWithFastAPIType();
    registerProtocolWithInvokerType();
    registerSendSubmissionToInvokerType();
}
