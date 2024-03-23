#include <stdio.h>
#include <math.h>

#include "raylib.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))

#define RAD_TO_DEG(rad) (rad) * (180 / PI)
#define DEG_TO_RAD(deg) (deg) / (180 / PI)

#define NB_MAX_BULLET 5000

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

typedef struct Bullet {
    Vector2 coord;
    float angle;
    float lifetime;
} Bullet;

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

Rectangle trap;

Texture player_t;
Vec2i player;
const int player_radius = 25;
const int player_speed = 5;

Texture map;
Vector2 map_coord;
const float map_factor = 2.0f;

Texture bullet;
Bullet bullets[NB_MAX_BULLET];
int bullet_index = 0;

void handle_resize_window(void)
{
    Vec2i old_screen = screen;
    screen.width = GetScreenWidth();
    screen.height = GetScreenHeight();
    trap.width = screen.width/5;
    trap.height = screen.height/5;
    trap.x = (int) (screen.width/2 - trap.width/2);
    trap.y = (int) (screen.height/2 - trap.height/2);
    player.x -= (old_screen.width - screen.width) / 2;
    player.y -= (old_screen.height - screen.height) / 2;
    map_coord.x -= (old_screen.width - screen.width) / 2;
    map_coord.y -= (old_screen.height - screen.height) / 2;
}

int point_rec_collision(Vec2i point, Rectangle rec)
{
    return
        (point.x >= rec.x && point.x <= rec.x + rec.width) &&
        (point.y >= rec.y && point.y <= rec.y + rec.height);
}

Vector2 move_forward_angle(Vector2 origin, float angle, float step)
{
    float rad = DEG_TO_RAD(angle);
    origin.x += sin(rad) * step;
    origin.y += cos(rad) * step;
    return origin;
}

int move_player_inside_trap(Vec2i *player, Rectangle trap,
                            const int speed, const float delta_time)
{
    int step = speed * delta_time * 100;

    int w = IsKeyDown(KEY_W);
    int a = IsKeyDown(KEY_A);
    int s = IsKeyDown(KEY_S);
    int d = IsKeyDown(KEY_D);

    if (w + a + s + d >= 2)
        step *= sin(45);

    player->x -= step * a;
    player->x += step * d;

    player->y -= step * w;
    player->y += step * s;

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

void move_map(Vector2 *map, Cardinal direction, const int speed, const float DT)
{
    float step = speed * DT * 100;
    float diag_step = sin(45) * step;

    switch (direction) {
    case NORD:
        map->y += step;
        break;
    case SUD:
        map->y -= step;
        break;
    case EST:
        map->x -= step;
        break;
    case OUEST:
        map->x += step;
        break;

    case NORD_EST:
        map->y += diag_step;
        map->x -= diag_step;
        break;
    case NORD_OUEST:
        map->y += diag_step;
        map->x += diag_step;
        break;
    case SUD_EST:
        map->y -= diag_step;
        map->x -= diag_step;
        break;
    case SUD_OUEST:
        map->y -= diag_step;
        map->x += diag_step;
        break;
    default:
        break;
    }
}

float vector_angle(Vector2 base, Vector2 point)
{
    float x = point.x - base.x;
    float y = point.y - base.y;
    int pad = 0;
    if (x < 0 && y <= 0) {
        pad = 180;
    } else if (x < 0 && y >= 0) {
        pad = 180;
    } else if (x >= 0 && y >= 0) {
        pad = 360;
    }
    return ABS(pad - RAD_TO_DEG(atan(y / x)));
}

Vector2 map_to_screen_coord(Vector2 coord)
{
    Vector2 r = {.x = map_coord.x + coord.x, .y = map_coord.y + coord.y};
    return r;
}

Vec2i screen_to_map_coord(Vec2i coord)
{
    Vec2i r = {.x = -map_coord.x + coord.x, .y = -map_coord.y + coord.y};
    return r;
}

int main(void)
{
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(screen.width, screen.height, "voleur");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    player = (Vec2i) {.x = screen.width/2, .y = screen.height/2};
    player_t = LoadTexture("data/player.png");
    map = LoadTexture("data/map2.png");
    map_coord = (Vector2){
        screen.width/2 - map.width*map_factor/2,
        screen.height/2 - map.height*map_factor/2,
    };

    bullet = LoadTexture("data/bullet.png");

    handle_resize_window();

    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            ClearBackground(LIME);
            const float DT = GetFrameTime();

            if (IsWindowResized())
                handle_resize_window();

            if (!move_player_inside_trap(&player, trap, player_speed, DT)) {
                Cardinal direction = snap_player_inside_trap(&player, trap);
                move_map(&map_coord, direction, player_speed, DT);
            }

            // map
            DrawTextureEx(map, map_coord, 0.0f, map_factor, WHITE);

            Vector2 mouse = GetMousePosition();
            float angle = vector_angle((Vector2){player.x, player.y}, mouse);

            Vec2i player_map = screen_to_map_coord(player);

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                bullets[bullet_index] = (Bullet) {
                    .coord.x = player_map.x,
                    .coord.y = player_map.y,
                    .angle = -angle,
                    .lifetime = 5.0f,
                };

                bullet_index = (bullet_index + 1) % NB_MAX_BULLET;
            }

            for (int i=0; i<NB_MAX_BULLET; i++) {
                Bullet *b = &bullets[i];
                if (b->lifetime > 0.0f) {
                    b->lifetime -= DT;
                    Vector2 co = map_to_screen_coord(b->coord);
                    DrawTexturePro(
                        bullet,
                        (Rectangle) {0, 0, bullet.width, bullet.height},
                        (Rectangle) {co.x, co.y, bullet.width, bullet.height},
                        (Vector2) {bullet.width/2, bullet.height/2},
                        b->angle, WHITE
                    );
                    b->coord = move_forward_angle(b->coord, 90-b->angle, 50.0f);
                }
            }

            // player
            DrawTexturePro(
                player_t,
                (Rectangle) {0, 0, player_t.width, player_t.height},
                (Rectangle) {player.x, player.y, player_t.width, player_t.height},
                (Vector2) {player_t.width/2, player_t.height/2},
                450 - angle, WHITE
            );
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
