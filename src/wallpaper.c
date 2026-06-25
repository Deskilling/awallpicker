#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "app.h"
#include "fs.h"
#include "wallpaper.h"

#include <dirent.h>
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static Image GenerateHexMask(int size, float radius, float angle_deg);
static int CountWallpapers(const char* dir_path);
static bool LoadSingleWallpaper(App* app, const char* dir_path, const char* filename);
static int CountWallpapersRecursive(const char* dir_path);
static bool LoadWallpapersRecursive(App* app, const char* dir_path);

bool InitWallpaperResources(App* app) {
	if (app == NULL) {
		return false;
	}

	app->img_size = (int)(HEX_RADIUS * 2.0f);
	app->hex_mask = GenerateHexMask(app->img_size, HEX_RADIUS, app->angle);

	return true;
}

bool LoadWallpapers(App* app) {
	DIR* dir = NULL;
	struct dirent* ent = NULL;

	if (app == NULL || app->wp_dir == NULL) {
		fprintf(stderr, "Error: invalid app state for wallpaper loading\n");
		return false;
	}

	app->capacity = app->recursive ? CountWallpapersRecursive(app->wp_dir) : CountWallpapers(app->wp_dir);

	if (app->capacity < 0) {
		fprintf(stderr, "Error: Unable to open directory %s\n", app->wp_dir);
		return false;
	}

	if (app->capacity == 0) {
		app->hexagons = NULL;
		app->hexagon_cnt = 0;
		return true;
	}

	app->hexagons = calloc((size_t)app->capacity, sizeof(Hexagon));
	if (app->hexagons == NULL) {
		fprintf(stderr, "Fatal Error: memory allocation failed (attempted to allocate %d hexagon slots)\n", app->capacity);
		return false;
	}

	app->hexagon_cnt = 0;

	if (app->recursive) {
		return LoadWallpapersRecursive(app, app->wp_dir);
	}

	dir = opendir(app->wp_dir);
	if (dir == NULL) {
		fprintf(stderr, "Error: Unable to reopen directory %s\n", app->wp_dir);
		UnloadHexagons(app);
		return false;
	}

	while ((ent = readdir(dir)) != NULL) {
		if (!IsSupportedWallpaperFile(ent->d_name)) {
			continue;
		}

		if (WindowShouldClose()) {
			break;
		}

		BeginDrawing();
		ClearBackground(BLANK);
		DrawText("Loading & Caching Wallpapers...", GetScreenWidth() / 2 - 250, GetScreenHeight() / 2, 30, WHITE);
		DrawText(ent->d_name, GetScreenWidth() / 2 - 250, GetScreenHeight() / 2 + 40, 20, GRAY);
		EndDrawing();

		(void)LoadSingleWallpaper(app, app->wp_dir, ent->d_name);
	}

	closedir(dir);
	return true;
}

static int CountWallpapers(const char* dir_path) {
	DIR* dir = opendir(dir_path);
	struct dirent* ent;
	int count = 0;

	if (dir == NULL) {
		return -1;
	}

	while ((ent = readdir(dir)) != NULL) {
		if (IsSupportedWallpaperFile(ent->d_name)) {
			count++;
		}
	}

	closedir(dir);
	return count;
}

static int CountWallpapersRecursive(const char* dir_path) {
	DIR* dir = opendir(dir_path);
	struct dirent* ent;
	int count = 0;

	if (dir == NULL) {
		return -1;
	}

	while ((ent = readdir(dir)) != NULL) {
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
			continue;
		}

		if (ent->d_type == DT_DIR) {
			char* subdir_path = JoinPath(dir_path, ent->d_name);
			if (subdir_path != NULL) {
				int subdir_count = CountWallpapersRecursive(subdir_path);
				if (subdir_count >= 0) {
					count += subdir_count;
				}
				free(subdir_path);
			}
		} else if (IsSupportedWallpaperFile(ent->d_name)) {
			count++;
		}
	}

	closedir(dir);
	return count;
}

static bool LoadSingleWallpaper(App* app, const char* dir_path, const char* filename) {
	char* full_img_path = NULL;
	char* cache_img_path = NULL;
	Image img = (Image){0};
	bool ok = false;

	if (app == NULL || filename == NULL) {
		return false;
	}

	if (app->hexagon_cnt >= app->capacity) {
		return false;
	}

	full_img_path = JoinPath(dir_path, filename);
	cache_img_path = BuildCacheImagePath(app->cache_dir, filename);

	if (full_img_path == NULL || cache_img_path == NULL) {
		goto cleanup;
	}

	if (access(cache_img_path, F_OK) == 0) {
		img = LoadImage(cache_img_path);
	} else {
		img = LoadImage(full_img_path);

		if (img.data != NULL && img.width > 1) {
			float scaleX = (float)app->img_size / (float)img.width;
			float scaleY = (float)app->img_size / (float)img.height;
			float scale = (scaleX > scaleY) ? scaleX : scaleY;

			int newW = (int)roundf((float)img.width * scale);
			int newH = (int)roundf((float)img.height * scale);

			if (newW < app->img_size) {
				newW = app->img_size;
			}
			if (newH < app->img_size) {
				newH = app->img_size;
			}

			ImageResize(&img, newW, newH);

			{
				int cropX = (newW - app->img_size) / 2;
				int cropY = (newH - app->img_size) / 2;

				ImageCrop(&img, (Rectangle){(float)cropX, (float)cropY, (float)app->img_size, (float)app->img_size});
			}

			ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
			ImageAlphaMask(&img, app->hex_mask);

			if (!ExportImage(img, cache_img_path)) {
				fprintf(stderr, "Warning: failed to write cache image: %s\n", cache_img_path);
			}
		}
	}

	if (img.data != NULL && img.width > 1) {
		Wallpaper* wp = calloc(1, sizeof(Wallpaper));
		if (wp == NULL) {
			fprintf(stderr, "Warning: out of memory allocating Wallpaper for %s\n", filename);
			goto cleanup;
		}

		wp->filename = strdup(filename);
		if (wp->filename == NULL) {
			fprintf(stderr, "Warning: out of memory allocating filename for %s\n", filename);
			free(wp);
			goto cleanup;
		}

		wp->dir_path = strdup(dir_path);
		if (wp->dir_path == NULL) {
			fprintf(stderr, "Warning: out of memory allocating dir_path for %s\n", filename);
			free(wp->filename);
			free(wp);
			goto cleanup;
		}

		wp->tex = LoadTextureFromImage(img);
		if (wp->tex.id == 0) {
			fprintf(stderr, "Warning: failed to create texture for %s\n", filename);
			free(wp->filename);
			free(wp->dir_path);
			free(wp);
			goto cleanup;
		}

		Hexagon* h = &app->hexagons[app->hexagon_cnt];
		h->type = WALLPAPER;
		h->content = wp;
		h->currentScale = 1.0f;
		h->currentColor = 130.0f;
		app->hexagon_cnt++;
		ok = true;
	}

cleanup:
	if (img.data != NULL) {
		UnloadImage(img);
	}

	free(cache_img_path);
	free(full_img_path);
	return ok;
}

// TODO add support for flatten option (currently it just flattens)
static bool LoadWallpapersRecursive(App* app, const char* dir_path) {
	DIR* dir = opendir(dir_path);
	struct dirent* ent;

	if (dir == NULL) {
		return -1;
	}

	while ((ent = readdir(dir)) != NULL) {
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
			continue;
		}

		if (WindowShouldClose()) {
			closedir(dir);
			return -1;
		}

		if (ent->d_type == DT_DIR) {
			char* subdir_path = JoinPath(dir_path, ent->d_name);
			if (subdir_path != NULL) {
				LoadWallpapersRecursive(app, subdir_path);
				free(subdir_path);
			}
		} else if (IsSupportedWallpaperFile(ent->d_name)) {
			BeginDrawing();
			ClearBackground(BLANK);
			DrawText("Loading & Caching Wallpapers...", GetScreenWidth() / 2 - 250, GetScreenHeight() / 2, 30, WHITE);
			DrawText(ent->d_name, GetScreenWidth() / 2 - 250, GetScreenHeight() / 2 + 40, 20, GRAY);
			EndDrawing();

			(void)LoadSingleWallpaper(app, dir_path, ent->d_name);
		}
	}

	closedir(dir);
	return true;
}

static Image GenerateHexMask(int size, float radius, float angle_deg) {
	Image mask = GenImageColor(size, size, BLANK);
	float cx = (float)size / 2.0f;
	float cy = (float)size / 2.0f;

	float rad = -angle_deg * DEG2RAD;
	float cosA = cosf(rad);
	float sinA = sinf(rad);

	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			float tx = (float)x - cx;
			float ty = (float)y - cy;

			float rot_x = tx * cosA - ty * sinA;
			float rot_y = tx * sinA + ty * cosA;

			float dx = fabsf(rot_x);
			float dy = fabsf(rot_y);

			if (dx <= 0.866025f * radius && dy <= radius - dx * 0.57735f) {
				ImageDrawPixel(&mask, x, y, WHITE);
			} else {
				ImageDrawPixel(&mask, x, y, BLANK);
			}
		}
	}

	return mask;
}
