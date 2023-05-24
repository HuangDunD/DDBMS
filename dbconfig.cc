#include "dbconfig.h"

// config for meta_server client
// DEFINE_string(META_SERVER_ADDR, "[::1]:8001", "meta server address.");
DEFINE_string(META_SERVER_ADDR, "127.0.0.1:8001", "meta server address.");

// config for every node
DEFINE_int32(SERVER_LISTEN_PORT, 8002, "TCP Port of server");
DEFINE_string(SERVER_LISTEN_ADDR, "0.0.0.0", "Server listen address, may be IPV4/IPV6/UDS."
            " If this is set, the flag port will be ignored");


// config for log path
DEFINE_string(log_path, "/home/t500ttt/RucDDBS/data/", "log path");