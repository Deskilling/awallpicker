#include "app.h"
#include "render.h"

#include <raylib.h>
#include <stddef.h>
#include <string.h>

static void RenderHexagon(const App* app, int i);

void RenderWallpapers(const App* app, int hoveredIndex, float angle_deg) {

	if (app == NULL || app->hexagons == NULL || app->hexagon_cnt <= 0) {
		return;
	}

	BeginDrawing();
	ClearBackground(BLANK);

	for (int i = 0; i < app->hexagon_cnt; i++) {
		if (i == hoveredIndex) {
			continue;
		}
		RenderHexagon(app, i);
	}

	if (hoveredIndex >= 0 && hoveredIndex < app->hexagon_cnt) {
		RenderHexagon(app, hoveredIndex);
		Vector2 center = {app->hexagons[hoveredIndex].render_x, app->hexagons[hoveredIndex].render_y};
		float scale = app->hexagons[hoveredIndex].currentScale;
		DrawPolyLinesEx(center, 6, HEX_RADIUS * scale, 30.0f + angle_deg, 8.0f, WHITE);
	}

	EndDrawing();
}

static void RenderHexagon(const App* app, int i) {
	float currentX = app->hexagons[i].render_x;
	float currentY = app->hexagons[i].render_y;
	float scale = app->hexagons[i].currentScale;
	unsigned char c = (unsigned char)app->hexagons[i].currentColor;
	Vector2 center = {currentX, currentY};

	if (app->hexagons[i].type == DIRECTORY) {
		Directory* dir = (Directory*)app->hexagons[i].content;
		float radius = ((float)app->img_size * scale) / 2.0f;

		DrawPoly(center, 6, radius, 30.0f, DARKGRAY);
		DrawPolyLines(center, 6, radius, 30.0f, WHITE);

		if (dir->path != NULL) {
			const char* label = dir->path;
			const char* slash = strrchr(dir->path, '/');

			if (slash != NULL && *(slash + 1) != '\0') {
				label = slash + 1;
			}

			int fontSize = 16;
			int textWidth = MeasureText(label, fontSize);
			DrawText(label, (int)(center.x - textWidth / 2.0f), (int)(center.y - fontSize / 2.0f), fontSize, WHITE);
		}
		return;
	}

	Wallpaper* wp = (Wallpaper*)app->hexagons[i].content;
	Color tint = (Color){c, c, c, 255};
	Rectangle sourceRec = {0.0f, 0.0f, (float)app->img_size, (float)app->img_size};
	Rectangle destRec = {currentX, currentY, (float)app->img_size * scale, (float)app->img_size * scale};
	Vector2 origin = {((float)app->img_size * scale) / 2.0f, ((float)app->img_size * scale) / 2.0f};
	DrawTexturePro(wp->tex, sourceRec, destRec, origin, 0.0f, tint);
}
