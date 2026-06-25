#include "app.h"
#include "render.h"

#include <raylib.h>
#include <stddef.h>

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

		float currentX = app->hexagons[i].render_x;
		float currentY = app->hexagons[i].render_y;

		float scale = app->hexagons[i].currentScale;
		unsigned char c = (unsigned char)app->hexagons[i].currentColor;

		if (app->hexagons[i].type == DIRECTORY) {
			// TODO render directory name (currently not possible because everything gets flattended)
		}

		Wallpaper* wp = (Wallpaper*)app->hexagons[i].content;
		Color tint = (Color){c, c, c, 255};
		Rectangle sourceRec = {0.0f, 0.0f, (float)app->img_size, (float)app->img_size};
		Rectangle destRec = {currentX, currentY, (float)app->img_size * scale, (float)app->img_size * scale};
		Vector2 origin = {((float)app->img_size * scale) / 2.0f, ((float)app->img_size * scale) / 2.0f};

		DrawTexturePro(wp->tex, sourceRec, destRec, origin, 0.0f, tint);
	}

	if (hoveredIndex >= 0 && hoveredIndex < app->hexagon_cnt) {
		Hexagon* h = &app->hexagons[hoveredIndex];
		float currentX = h->render_x;
		float currentY = h->render_y;
		float scale = h->currentScale;
		unsigned char c = (unsigned char)h->currentColor;
		Vector2 currentCenter = {currentX, currentY};

		if (h->type == WALLPAPER) {
			Wallpaper* wp = (Wallpaper*)h->content;
			Color tint = (Color){c, c, c, 255};
			Rectangle sourceRec = {0.0f, 0.0f, (float)app->img_size, (float)app->img_size};
			Rectangle destRec = {currentX, currentY, (float)app->img_size * scale, (float)app->img_size * scale};
			Vector2 origin = {((float)app->img_size * scale) / 2.0f, ((float)app->img_size * scale) / 2.0f};
			DrawTexturePro(wp->tex, sourceRec, destRec, origin, 0.0f, tint);
		} else if (h->type == DIRECTORY) {
			// TODO dir look above
		}

		DrawPolyLinesEx(currentCenter, 6, HEX_RADIUS * scale, 30.0f + angle_deg, 8.0f, WHITE);
	}

	EndDrawing();
}
