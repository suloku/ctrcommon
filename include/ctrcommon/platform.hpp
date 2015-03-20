#ifndef __CTRCOMMON_PLATFORM_HPP__
#define __CTRCOMMON_PLATFORM_HPP__

#include "ctrcommon/types.hpp"

#include <string>

typedef enum {
    MODULE_COMMON = 0,
    MODULE_KERNEL = 1,
    MODULE_UTIL = 2,
    MODULE_FILE_SERVER = 3,
    MODULE_LOADER_SERVER = 4,
    MODULE_TCB = 5,
    MODULE_OS = 6,
    MODULE_DBG = 7,
    MODULE_DMNT = 8,
    MODULE_PDN = 9,
    MODULE_GX = 10,
    MODULE_I2C = 11,
    MODULE_GPIO = 12,
    MODULE_DD = 13,
    MODULE_CODEC = 14,
    MODULE_SPI = 15,
    MODULE_PXI = 16,
    MODULE_FS = 17,
    MODULE_DI = 18,
    MODULE_HID = 19,
    MODULE_CAM = 20,
    MODULE_PI = 21,
    MODULE_PM = 22,
    MODULE_PM_LOW = 23,
    MODULE_FSI = 24,
    MODULE_SRV = 25,
    MODULE_NDM = 26,
    MODULE_NWM = 27,
    MODULE_SOC = 28,
    MODULE_LDR = 29,
    MODULE_ACC = 30,
    MODULE_ROMFS = 31,
    MODULE_AM = 32,
    MODULE_HIO = 33,
    MODULE_UPDATER = 34,
    MODULE_MIC = 35,
    MODULE_FND = 36,
    MODULE_MP = 37,
    MODULE_MPWL = 38,
    MODULE_AC = 39,
    MODULE_HTTP = 40,
    MODULE_DSP = 41,
    MODULE_SND = 42,
    MODULE_DLP = 43,
    MODULE_HIO_LOW = 44,
    MODULE_CSND = 45,
    MODULE_SSL = 46,
    MODULE_AM_LOW = 47,
    MODULE_NEX = 48,
    MODULE_FRIENDS = 49,
    MODULE_RDT = 50,
    MODULE_APPLET = 51,
    MODULE_NIM = 52,
    MODULE_PTM = 53,
    MODULE_MIDI = 54,
    MODULE_MC = 55,
    MODULE_SWC = 56,
    MODULE_FATFS = 57,
    MODULE_NGC = 58,
    MODULE_CARD = 59,
    MODULE_CARDNOR = 60,
    MODULE_SDMC = 61,
    MODULE_BOSS = 62,
    MODULE_DBM = 63,
    MODULE_CONFIG = 64,
    MODULE_PS = 65,
    MODULE_CEC = 66,
    MODULE_IR = 67,
    MODULE_UDS = 68,
    MODULE_PL = 69,
    MODULE_CUP = 70,
    MODULE_GYROSCOPE = 71,
    MODULE_MCU = 72,
    MODULE_NS = 73,
    MODULE_NEWS = 74,
    MODULE_RO = 75,
    MODULE_GD = 76,
    MODULE_CARD_SPI = 77,
    MODULE_EC = 78,
    MODULE_WEB_BROWSER = 80,
    MODULE_TEST = 81,
    MODULE_ENC = 82,
    MODULE_PIA = 83,
    MODULE_MVD = 92,
    MODULE_QTM = 96,
    MODULE_APPLICATION = 254,
    MODULE_INVALID = 255
} ErrorModule;

typedef enum {
    LEVEL_SUCCESS = 0,
    LEVEL_INFO = 1,
    LEVEL_STATUS = 25,
    LEVEL_TEMPORARY = 26,
    LEVEL_PERMANENT = 27,
    LEVEL_USAGE = 28,
    LEVEL_REINITIALIZE = 29,
    LEVEL_RESET = 30,
    LEVEL_FATAL = 31
} ErrorLevel;

typedef enum {
    SUMMARY_SUCCESS = 0,
    SUMMARY_NOTHING_HAPPENED = 1,
    SUMMARY_WOULD_BLOCK = 2,
    SUMMARY_OUT_OF_RESOURCE = 3,
    SUMMARY_NOT_FOUND = 4,
    SUMMARY_INVALID_STATE = 5,
    SUMMARY_NOT_SUPPORTED = 6,
    SUMMARY_INVALID_ARGUMENT = 7,
    SUMMARY_WRONG_ARGUMENT = 8,
    SUMMARY_CANCELED = 9,
    SUMMARY_STATUS_CHANGED = 10,
    SUMMARY_INTERNAL = 11,
    SUMMARY_INVALID = 63
} ErrorSummary;

typedef enum {
    DESCRIPTION_SUCCESS = 0,
    DESCRIPTION_INVALID_MEMORY_PERMISSIONS = 2,
    DESCRIPTION_INVALID_TICKET_VERSION = 4,
    DESCRIPTION_STRING_TOO_BIG = 5,
    DESCRIPTION_ACCESS_DENIED = 6,
    DESCRIPTION_STRING_TOO_SMALL = 7,
    DESCRIPTION_CAMERA_BUSY = 8,
    DESCRIPTION_NOT_ENOUGH_MEMORY = 10,
    DESCRIPTION_SESSION_CLOSED_BY_REMOTE = 26,
    DESCRIPTION_INVALID_NCCH = 37,
    DESCRIPTION_INVALID_TITLE_VERSION = 39,
    DESCRIPTION_DATABASE_DOES_NOT_EXIST = 43,
    DESCRIPTION_TRIED_TO_UNINSTALL_SYSTEM_APP = 44,
    DESCRIPTION_ARCHIVE_NOT_MOUNTED = 101,
    DESCRIPTION_REQUEST_TIMED_OUT = 105,
    DESCRIPTION_TITLE_NOT_FOUND = 120,
    DESCRIPTION_GAMECARD_NOT_INSERTED = 141,
    DESCRIPTION_INVALID_FILE_OPEN_FLAGS = 230,
    DESCRIPTION_INVALID_CONFIGURATION = 271,
    DESCRIPTION_NCCH_HASH_CHECK_FAILED = 391,
    DESCRIPTION_AES_VERIFICATION_FAILED = 392,
    DESCRIPTION_SAVE_HASH_CHECK_FAILED = 395,
    DESCRIPTION_COMMAND_PERMISSION_DENIED = 630,
    DESCRIPTION_INVALID_PATH = 702,
    DESCRIPTION_INCORRECT_READ_SIZE = 761,
    DESCRIPTION_INVALID_SECTION = 1000,
    DESCRIPTION_TOO_LARGE = 1001,
    DESCRIPTION_NOT_AUTHORIZED = 1002,
    DESCRIPTION_ALREADY_DONE = 1003,
    DESCRIPTION_INVALID_SIZE = 1004,
    DESCRIPTION_INVALID_ENUM_VALUE = 1005,
    DESCRIPTION_INVALID_COMBINATION = 1006,
    DESCRIPTION_NO_DATA = 1007,
    DESCRIPTION_BUSY = 1008,
    DESCRIPTION_MISALIGNED_ADDRESS = 1009,
    DESCRIPTION_MISALIGNED_SIZE = 1010,
    DESCRIPTION_OUT_OF_MEMORY = 1011,
    DESCRIPTION_NOT_IMPLEMENTED = 1012,
    DESCRIPTION_INVALID_ADDRESS = 1013,
    DESCRIPTION_INVALID_POINTER = 1014,
    DESCRIPTION_INVALID_HANDLE = 1015,
    DESCRIPTION_NOT_INITIALIZED = 1016,
    DESCRIPTION_ALREADY_INITIALIZED = 1017,
    DESCRIPTION_NOT_FOUND = 1018,
    DESCRIPTION_CANCEL_REQUESTED = 1019,
    DESCRIPTION_ALREADY_EXISTS = 1020,
    DESCRIPTION_OUT_OF_RANGE = 1021,
    DESCRIPTION_TIMEOUT = 1022,
    DESCRIPTION_INVALID = 1023
} ErrorDescription;

typedef struct {
    u32 raw;
    ErrorModule module;
    ErrorLevel level;
    ErrorSummary summary;
    ErrorDescription description;
} Error;

bool platform_init();
void platform_cleanup();
bool platform_is_running();
bool platform_is_ninjhax();
u32 platform_get_device_id();
u64 platform_get_time();
void platform_delay(int ms);
void platform_printf(const char* format, ...);
Error platform_get_error();
void platform_set_error(Error error);
std::string platform_get_error_string(Error error);

#endif