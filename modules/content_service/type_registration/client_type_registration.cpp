#include <oink_judge/content_service/client/content_service_stub.h>

extern "C" auto registerTypes() -> void { oink_judge::content_service::registerContentServiceChannelStubType(); }
