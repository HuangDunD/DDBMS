#pragma once
#ifndef DBCONFIG_H
#define DBCONFIG_H

#include <string>
#include <gflags/gflags.h>

//config for metaserver
static const std::string DATA_DIR = "/home/t500ttt/DDBMS/data/";
static const std::string META_SERVER_FILE_NAME = "META_SERVER.meta";
static const int META_SERVER_PORT = 8001; // maybe not used
static const std::string META_SERVER_LISTEN_ADDR = "0.0.0.0:8001";
static const int idle_timeout_s = -1;
static bool enable_logging(true);

DECLARE_string(META_SERVER_ADDR);

DECLARE_int32(SERVER_LISTEN_PORT);
DECLARE_string(SERVER_LISTEN_ADDR);

// log path
// static const std::string log_path = "/home/t500ttt/RucDDBS/data/";
DECLARE_string(log_path);

// 写死database name 目前只考虑数据库操作
static const std::string DB_NAME = "RucDDBS";

#endif