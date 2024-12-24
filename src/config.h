#ifndef CONFIG_H
#define CONFIG_H

#define APP_VERSION_ID      1330
#define APP_VERSION_NAME    "v1.3.3"
#define APP_VERSION_NUMS    1,3,3,0

#define APP_NAME            "BTC Tools"
#define APP_NAME_DEBUG      "BTC Tools Debug Mode"

#define APP_FULL_NAME       APP_NAME " " APP_VERSION_NAME
#define APP_FULL_NAME_DEBUG APP_NAME_DEBUG " " APP_VERSION_NAME

#define APP_COMPANY_NAME    "BTC.com"
#define APP_COPYRIGHT_LINE  "Copyright 2021 BTC.com"
#define APP_LEGAL_INFO_LINE "All Rights Reserved"

#define APP_FILE_NAME       APP_NAME ".exe"

#ifdef PLATFORM_NAME
  #define PLATFORM_SUFFIX   "-" PLATFORM_NAME
#elif defined (WIN32)
  #define PLATFORM_NAME     "win32"
  #define PLATFORM_SUFFIX   ""
#else
  #define PLATFORM_NAME     "linux"
  #define PLATFORM_SUFFIX   "-linux"
#endif

#define APP_AUTO_UPDATE_URL       "https://url.btc.com/btc-tools-update" PLATFORM_SUFFIX
#define APP_AUTO_UPDATE_URL_DEBUG "http://localhost/update-data" PLATFORM_SUFFIX ".json"

#define UI_CONFIG_FILE_PATH "./btctools.ini"
#define LOCALE_DIR_PATH ":/locale"

#define POOL_SERVER_DOMAIN_REGEXP "^.*\\.ss\\.btc\\.com$"
#define POOL_SERVER_IP_REGEXP "^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$"

#define SCAN_SESSION_TIMEOUT 3
#define CONFIG_SESSION_TIMEOUT 10
#define UPGRADE_SESSION_TIMEOUT 180
#define MONITOR_INTERVAL 5

#define SCAN_STEP_SIZE 100
#define CONFIG_STEP_SIZE 100
// Warning: If the value is too large, it will insufficient
// memory when upgrading and the program will crash.
#define UPGRADE_SEND_FIRMWARE_STEP_SIZE 5

// Note: The time required to retry when the connection is refused
// on Windows is significantly longer than that on Linux.
#define AUTO_RETRY_TIMES 2

#define WORKER_NAME_IP_PARTS 2

#define IS_HIGHLIGHT_TEMPERATURE true
#define HIGHLIGHT_TEMPERATURE_MORE_THAN 90
#define HIGHLIGHT_TEMPERATURE_LESS_THAN 0
#define IS_HIGHLIGHT_LOW_HASHRATE true
#define IS_HIGHLIGHT_WRONG_WORKER_NAME true
#define OPEN_MINER_CP_WITH_PASSWORD true

#define HIGHLIGHT_LOW_HASHRATES {{"Antminer S9", 10.0e12},\
                                 {"Antminer S7", 4.0e12}}

#define DEFAULT_MINER_PASSWORDS {{"Antminer", "root", "root"},\
                                 {"AvalonDevice", "root", "root"},\
                                 {"Avalon", "root", ""},\
                                 {"WhatsMiner", "admin", "admin"}}

#endif // CONFIG_H
