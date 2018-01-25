QT           += widgets network websockets
CONFIG += c++11 c++14

HEADERS       = \ 
    example_service.h \
    jcon/jcon.h \
    jcon/jcon_assert.h \
    jcon/json_rpc_client.h \
    jcon/json_rpc_endpoint.h \
    jcon/json_rpc_request.h \
    jcon/json_rpc_result.h \
    jcon/json_rpc_server.h \
    jcon/json_rpc_socket.h \
    jcon/json_rpc_websocket.h \
    jcon/json_rpc_websocket_client.h \
    jcon/json_rpc_websocket_server.h \
    jcon/string_util.h \
    msgpack/endianhelper.h \
    msgpack/msgpack.h \
    msgpack/msgpack_export.h \
    msgpack/msgpackcommon.h \
    msgpack/msgpackstream.h \
    msgpack/private/pack_p.h \
    msgpack/private/unpack_p.h \
    jcon/json_rpc_logger.h
SOURCES       = main.cpp \
    example_service.cpp \
    jcon/json_rpc_client.cpp \
    jcon/json_rpc_endpoint.cpp \
    jcon/json_rpc_request.cpp \
    jcon/json_rpc_server.cpp \
    jcon/json_rpc_websocket.cpp \
    jcon/json_rpc_websocket_client.cpp \
    jcon/json_rpc_websocket_server.cpp \
    jcon/string_util.cpp \
    msgpack/msgpack.cpp \
    msgpack/msgpackcommon.cpp \
    msgpack/msgpackstream.cpp \
    msgpack/private/pack_p.cpp \
    msgpack/private/unpack_p.cpp \
    jcon/json_rpc_logger.cpp


