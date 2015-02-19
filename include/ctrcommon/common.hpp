#ifndef __CTRCOMMON_COMMON_H__
#define __CTRCOMMON_COMMON_H__

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include <vector>
#include <string>
#include <functional>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#ifdef _3DS
#include <3ds/types.h>
#elif !defined(BIT)
#define BIT(n) (1U << (n))
#endif

bool platform_init();
void platform_cleanup();
bool platform_is_running();
u64 platform_get_time();
void platform_delay(int ms);
void platform_printf(const char* format, ...);
bool platform_is_io_waiting();

typedef enum {
    NAND,
    SD
} MediaType;

typedef enum {
    WII,
    DSI,
    THREEDS,
    WIIU,
    UNKNOWN_PLATFORM
} AppPlatform;

// TODO: verify categories.
typedef enum {
    APP,
    DLC,
    PATCH,
    SYSTEM,
    TWL
} AppCategory;

typedef struct {
    u64 titleId;
    u32 uniqueId;
    char productCode[16];
    MediaType mediaType;
    AppPlatform platform;
    AppCategory category;
} App;

#define APP_SUCCESS 0
#define APP_AM_INIT_FAILED -1
#define APP_OPERATION_CANCELLED -2
#define APP_BEGIN_INSTALL_FAILED -3
#define APP_IO_ERROR -4
#define APP_FINALIZE_INSTALL_FAILED -5
#define APP_OPEN_FILE_FAILED -6
#define APP_DELETE_FAILED -7
#define APP_LAUNCH_FAILED -8

const std::string app_get_result_string(int result);
const std::string app_get_platform_name(AppPlatform platform);
const std::string app_get_category_name(AppCategory category);
std::vector<App> app_list(MediaType mediaType);
int app_install_file(MediaType mediaType, const std::string path, std::function<bool(int progress)> onProgress);
int app_install(MediaType mediaType, FILE* fd, u64 size, std::function<bool(int progress)> onProgress);
int app_delete(App app);
int app_launch(App app);

u64 fs_get_free_space(MediaType mediaType);
bool fs_exists(const std::string path);
void fs_delete(const std::string path);

typedef enum {
    BUTTON_A = BIT(0),
    BUTTON_B = BIT(1),
    BUTTON_SELECT = BIT(2),
    BUTTON_START = BIT(3),
    BUTTON_DRIGHT = BIT(4),
    BUTTON_DLEFT = BIT(5),
    BUTTON_DUP = BIT(6),
    BUTTON_DDOWN = BIT(7),
    BUTTON_R = BIT(8),
    BUTTON_L = BIT(9),
    BUTTON_X = BIT(10),
    BUTTON_Y = BIT(11),
    BUTTON_ZL = BIT(14),
    BUTTON_ZR = BIT(15),
    BUTTON_TOUCH = BIT(20),
    BUTTON_CSTICK_RIGHT = BIT(24),
    BUTTON_CSTICK_LEFT = BIT(25),
    BUTTON_CSTICK_UP = BIT(26),
    BUTTON_CSTICK_DOWN = BIT(27),
    BUTTON_CPAD_RIGHT = BIT(28),
    BUTTON_CPAD_LEFT = BIT(29),
    BUTTON_CPAD_UP = BIT(30),
    BUTTON_CPAD_DOWN = BIT(31),
    BUTTON_UP = BUTTON_DUP | BUTTON_CPAD_UP,
    BUTTON_DOWN = BUTTON_DDOWN | BUTTON_CPAD_DOWN,
    BUTTON_LEFT = BUTTON_DLEFT | BUTTON_CPAD_LEFT,
    BUTTON_RIGHT = BUTTON_DRIGHT | BUTTON_CPAD_RIGHT,
} Button;

typedef struct {
    int x;
    int y;
} Touch;

const std::string input_get_button_name(Button button);
void input_poll();
bool input_is_any_pressed();
Button input_get_any_pressed();
bool input_is_released(Button button);
bool input_is_pressed(Button button);
bool input_is_held(Button button);
Touch input_get_touch();

typedef struct {
    u8 r;
    u8 g;
    u8 b;
} Color;

typedef enum {
    TOP_SCREEN,
    BOTTOM_SCREEN
} Screen;

bool screen_begin_draw(Screen screen);
bool screen_end_draw();
void screen_swap_buffers_quick();
void screen_swap_buffers();
u16 screen_get_width();
u16 screen_get_height();
bool screen_read_pixels(u8* dest, int srcX, int srcY, int dstX, int dstY, u16 width, u16 height);
bool screen_take_screenshot();
bool screen_draw(int x, int y, u8 r, u8 g, u8 b);
bool screen_draw_packed(int x, int y, u32 color);
bool screen_fill(int x, int y, u16 width, u16 height, u8 r, u8 g, u8 b);
bool screen_clear(u8 r, u8 g, u8 b);
void screen_clear_all();
u16 screen_get_str_width(const std::string str);
u16 screen_get_str_height(const std::string str);
bool screen_draw_char(char c, int x, int y, u8 r, u8 g, u8 b);
bool screen_draw_string(const std::string str, int x, int y, u8 r, u8 g, u8 b);

u64 htonll(u64 value);
u64 ntohll(u64 value);
u32 socket_get_host_ip();
int socket_listen(u16 port);
FILE* socket_accept(int listeningSocket);
FILE* socket_connect(const std::string ipAddress, u16 port);

typedef struct {
	std::string id;
	std::string name;
	std::vector<std::string> details;
} SelectableElement;

typedef struct {
	FILE* fd;
	u64 fileSize;
} RemoteFile;

bool ui_select(SelectableElement* selected, std::vector<SelectableElement> elements, std::function<bool(std::vector<SelectableElement>& currElements, bool& elementsDirty, bool &resetCursorIfDirty)> onLoop, std::function<bool(SelectableElement select)> onSelect, bool useTopScreen = true);
bool ui_select_file(std::string* selectedFile, const std::string rootDirectory, std::vector<std::string> extensions, std::function<bool(bool inRoot)> onLoop, bool useTopScreen = true);
bool ui_select_app(App* selectedApp, MediaType mediaType, std::function<bool()> onLoop, bool useTopScreen = true);
void ui_display_message(Screen screen, const std::string message);
bool ui_prompt(Screen screen, const std::string message, bool question);
void ui_display_progress(Screen screen, const std::string operation, const std::string details, bool quickSwap, int progress);
RemoteFile ui_accept_remote_file(Screen screen);

#endif