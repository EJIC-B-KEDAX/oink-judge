#include <oink_judge/content_service/client/content_client_protocol.h>

using namespace oink_judge::content_service;

extern "C" auto registerTypes() -> void { registerContentClientProtocolType(); }
