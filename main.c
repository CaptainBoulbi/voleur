#include <stdio.h>

#include "raylib.h"

#define MIN(a, b) (a) < (b) ? (a) : (b)
#define MAX(a, b) (a) > (b) ? (a) : (b)

typedef struct Vec2i {
    union {
        int x;
        int width;
    };
    union {
        int y;
        int height;
    };
} Vec2i;

typedef enum Cardinal {
    CARDINAL_BEGIN_NOT_AT_ZERO = 0,
    NORD_OUEST, NORD,   NORD_EST,
    OUEST,              EST,
    SUD_OUEST,  SUD,    SUD_EST,
    CARDINAL_SIZE,
} Cardinal;

#ifndef RELEASE
char *cardinal_text[CARDINAL_SIZE] = {
    [NORD_OUEST] = "NORD OUEST",
    [NORD] = "NORD",
    [NORD_EST] = "NORD EST",
    [OUEST] = "OUEST",
    [EST] = "EST",
    [SUD_OUEST] = "SUD OUEST",
    [SUD] = "SUD",
    [SUD_EST] = "SUD EST",
};
#endif

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

int move_player_inside_trap(Vec2i *player, Rectangle trap,
                            const int speed, const float delta_time)
{
    int step = speed * delta_time * 100;

    player->x -= step * IsKeyDown(KEY_A);
    player->x += step * IsKeyDown(KEY_D);

    player->y -= step * IsKeyDown(KEY_W);
    player->y += step * IsKeyDown(KEY_S);

    return point_rec_collision(*player, trap);
}

Cardinal snap_player_inside_trap(Vec2i *player, const Rectangle trap)
{
    Cardinal pushing = 0;

    int bound_n = player->y < trap.y;
    int bound_s = player->y > trap.y + trap.height;
    int bound_e = player->x > trap.x + trap.width;
    int bound_o = player->x < trap.x;
    int bound_ne = bound_n && bound_e;
    int bound_no = bound_n && bound_o;
    int bound_se = bound_s && bound_e;
    int bound_so = bound_s && bound_o;

    if (bound_n) {
        player->y = trap.y;
        pushing = NORD;
    } else if (bound_s) {
        player->y = trap.y + trap.height;
        pushing = SUD;
    }
    if (bound_e) {
        player->x = trap.x + trap.width;
        pushing = EST;
    } else if (bound_o) {
        player->x = trap.x;
        pushing = OUEST;
    }

    if (bound_ne)
        pushing = NORD_EST;
    else if (bound_no)
        pushing = NORD_OUEST;
    else if (bound_se)
        pushing = SUD_EST;
    else if (bound_so)
        pushing = SUD_OUEST;

    return pushing;
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
            const float DT = GetFrameTime();

            if (IsWindowResized())
                handle_resize_window();

            if (!move_player_inside_trap(&player, trap, player_speed, DT)) {
                Cardinal direction = snap_player_inside_trap(&player, trap);
#ifndef RELEASE
                printf("[%s]\n", cardinal_text[direction]);
#endif
            }
            
            DrawCircle(player.x, player.y, player_radius, BLUE);
            DrawRectangleLinesEx(trap, 1, RED);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
