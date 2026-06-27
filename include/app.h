#ifndef APP_H
#define APP_H

#include <raylib.h>
#include <limits.h>
#include <stdbool.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define HEX_RADIUS 200.0f
#define DEFAULT_COLS 5
#define DEFAULT_SPACING 15.0f
#define DEFAULT_WINDOW_WIDTH 1920
#define DEFAULT_WINDOW_HEIGHT 1080
#define DEFAULT_RECURSIVE 0
#define DEFAULT_FLATTEN 0

typedef enum {
	WALLPAPER,
	DIRECTORY,
} HexagonType;

typedef struct {
	Texture2D tex;
	char* filename;
	char* dir_path;
} Wallpaper;

typedef struct Directory {
	char* path;
	Wallpaper** wallpapers;
	int wallpaper_cnt;

	struct Directory** sub_dirs;
	int sub_dirs_cnt;
} Directory;

typedef struct {
	HexagonType type;
	void* content;

	float currentScale;
	float currentColor;
	// Cached render coordinates to avoid redundant layout calculations in the
	// render loop
	float render_x;
	float render_y;
} Hexagon;

typedef struct {
	bool valid;
	char* full_target_path;
	float rel_x;
	float rel_y;
} SelectionResult;

typedef enum { SESSION_BACKEND_UNKNOWN = 0, SESSION_BACKEND_WAYLAND, SESSION_BACKEND_X11 } SessionBackend;

typedef struct {
	const char* wallpaper_dir;
	int window_width;
	int window_height;
	int cols;
	float spacing;
	float angle;
	SessionBackend backend;
	bool recursive;
	bool flatten;
} AppConfig;

typedef struct {
	char* wp_dir;
	char cache_dir[PATH_MAX];

	Hexagon* hexagons;
	int hexagon_cnt;

	int capacity;

	Image hex_mask;
	int img_size;

	bool recursive;
	bool flatten;

	float scroll_offset;
	float target_scroll_offset;
	float angle;
} App;

AppConfig AppConfigFromArgs(int argc, char** argv);
int AppRun(const AppConfig* config);

SessionBackend DetectSessionBackend(void);
const char* SessionBackendName(SessionBackend backend);

void FreeSelectionResult(SelectionResult* result);
void UnloadHexagons(App* app);
#endif
