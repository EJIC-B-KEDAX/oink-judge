#include <oink_judge/socket/protocols/auth_required_protocol.h>
#include <oink_judge/socket/protocols/authorizing_protocol.h>
#include <oink_judge/socket/protocols/pinging_protocol.h>
#include <oink_judge/socket/protocols/ponging_protocol.h>
#include <oink_judge/socket/sessions/ssl_session.h>
#include <oink_judge/socket/simple_connection_handler.h>

using namespace oink_judge::socket;

extern "C" void registerTypes() {
    registerSimpleConnectionHandlerType();
    registerSSLSessionClientType();
    registerSSLSessionServerType();
    registerAuthRequiredProtocolType();
    registerAuthorizingProtocolType();
    registerPingingProtocolType();
    registerPongingProtocolType();
}
