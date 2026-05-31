#include <oink_judge/auth_service/protocol_with_fastapi.h>

using namespace oink_judge::auth_service;

extern "C" auto registerTypes() -> void { registerProtocolWithFastAPIType(); }
