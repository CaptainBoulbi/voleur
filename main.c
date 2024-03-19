#include "raylib.h"

#define MIN(a, b) (a) < (b) ? (a) : (b)
#define MAX(a, b) (a) > (b) ? (a) : (b)

typedef struct {
    union {
        int x;
        int width;
    };
    union {
        int y;
        int height;
    };
} Vec2i;

Vec2i screen = {.width = 1344, .height = 756};

const int trap_len = 200;
Rectangle trap = {
    .width = trap_len,
    .height = trap_len,
};

Vec2i player;
const int player_radius = 25;
const int player_speed = 5;

void handle_resize_window(void)
{
    Vec2i old_screen = screen;
    screen.width = GetScreenWidth();
    screen.height = GetScreenHeight();
    trap.x = screen.width/2 - trap_len/2;
    trap.y = screen.height/2 - trap_len/2;
    player.x -= (old_screen.width - screen.width) / 2;
    player.y -= (old_screen.height - screen.height) / 2;
}

int point_rec_collision(Vec2i point, Rectangle rec)
{
    return
        (point.x >= rec.x && point.x <= rec.x + rec.width) &&
        (point.y >= rec.y && point.y <= rec.y + rec.height);
}

int move_player_inside_trap(Vec2i *player, Rectangle trap, const int speed, const float delta_time)
{
    int step = speed * delta_time * 100;

    player->x -= step * IsKeyDown(KEY_A);
    player->x += step * IsKeyDown(KEY_D);

    player->y -= step * IsKeyDown(KEY_W);
    player->y += step * IsKeyDown(KEY_S);

    return point_rec_collision(*player, trap);
}

void snap_player_inside_trap(Vec2i *player, const Rectangle trap)
{
    player->x = MAX(MIN(player->x, trap.x + trap.width), trap.x);
    player->y = MAX(MIN(player->y, trap.y + trap.height), trap.y);
}

int main(void)
{
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(screen.width, screen.height, "voleur");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    handle_resize_window();

    player = (Vec2i) {.x = screen.width/2, .y = screen.height/2};

    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            ClearBackground(BLACK);
            const float delta_time = GetFrameTime();

            if (IsWindowResized())
                handle_resize_window();

            if (!move_player_inside_trap(&player, trap, player_speed, delta_time)) {
                snap_player_inside_trap(&player, trap);
            }
            
            DrawCircle(player.x, player.y, player_radius, BLUE);
            DrawRectangleLinesEx(trap, 1, RED);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
