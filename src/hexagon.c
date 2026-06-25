#include "app.h"

#include <dirent.h>
#include <raylib.h>
#include <stdlib.h>
#include <unistd.h>

void UnloadDirectory(Directory* dir) {
	if (dir == NULL) {
		return;
	}

	for (int i = 0; i < dir->sub_dirs_cnt; i++) {
		UnloadDirectory(dir->sub_dirs[i]);
		free(dir->sub_dirs[i]);
	}
	free(dir->sub_dirs);

	for (int i = 0; i < dir->wallpaper_cnt; i++) {
		free(dir->wallpapers[i]->filename);
		free(dir->wallpapers[i]->dir_path);
		UnloadTexture(dir->wallpapers[i]->tex);
		free(dir->wallpapers[i]);
	}
	free(dir->wallpapers);
	free(dir->path);

	dir->wallpapers = NULL;
	dir->path = NULL;
}

void UnloadHexagons(App* app) {
	if (app == NULL || app->hexagons == NULL) {
		return;
	}

	for (int i = 0; i < app->hexagon_cnt; i++) {
		if (app->hexagons[i].type == WALLPAPER) {
			Wallpaper* wp = (Wallpaper*)app->hexagons[i].content;
			free(wp->filename);
			free(wp->dir_path);
			UnloadTexture(wp->tex);
			free(wp);
		} else if (app->hexagons[i].type == DIRECTORY) {
			Directory* dir = (Directory*)app->hexagons[i].content;
			UnloadDirectory(dir);
			free(dir);
		}
	}

	free(app->hexagons);
	app->hexagons = NULL;
	app->hexagon_cnt = 0;
}
