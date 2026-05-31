#include <oink_judge/content_service/server/content_server_protocol.h>

using namespace oink_judge::content_service;

extern "C" auto registerTypes() -> void { registerContentServerProtocolType(); }
