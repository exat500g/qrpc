QT           += widgets network websockets
CONFIG += c++11 c++14

HEADERS       = \ 
    example_service.h \
    jcon/jcon.h \
    jcon/jcon_assert.h \
    jcon/json_rpc_client.h \
    jcon/json_rpc_debug_logger.h \
    jcon/json_rpc_endpoint.h \
    jcon/json_rpc_error.h \
    jcon/json_rpc_file_logger.h \
    jcon/json_rpc_logger.h \
    jcon/json_rpc_request.h \
    jcon/json_rpc_result.h \
    jcon/json_rpc_server.h \
    jcon/json_rpc_socket.h \
    jcon/json_rpc_success.h \
    jcon/json_rpc_websocket.h \
    jcon/json_rpc_websocket_client.h \
    jcon/json_rpc_websocket_server.h \
    jcon/string_util.h
SOURCES       = main.cpp \
    example_service.cpp \
    jcon/json_rpc_client.cpp \
    jcon/json_rpc_debug_logger.cpp \
    jcon/json_rpc_endpoint.cpp \
    jcon/json_rpc_error.cpp \
    jcon/json_rpc_file_logger.cpp \
    jcon/json_rpc_logger.cpp \
    jcon/json_rpc_request.cpp \
    jcon/json_rpc_server.cpp \
    jcon/json_rpc_success.cpp \
    jcon/json_rpc_websocket.cpp \
    jcon/json_rpc_websocket_client.cpp \
    jcon/json_rpc_websocket_server.cpp \
    jcon/string_util.cpp


