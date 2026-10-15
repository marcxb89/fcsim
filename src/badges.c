#include "badges.h"
#include "graph.h"
#include <stddef.h>

/* How many player pieces still count as "minimalist". 0 counts. */
#define MINIMALIST_MAX_PIECES 9

static const struct badge_info badge_table[BADGE_NUM_BADGES] = {
    [BADGE_GREEN]       = { BADGE_GREEN,       "Green",
                            "No powered wheels." },
    [BADGE_BROWN]       = { BADGE_BROWN,       "Brown",
                            "No wheels of any kind." },
    [BADGE_BLUE]        = { BADGE_BLUE,        "Blue",
                            "No wheels and no wooden rods." },
    [BADGE_CARPENTER]   = { BADGE_CARPENTER,   "Carpenter",
                            "No wheels and no water rods." },
    [BADGE_RODLESS]     = { BADGE_RODLESS,     "Rodless",
                            "No rods of any kind." },
    [BADGE_MINIMALIST]  = { BADGE_MINIMALIST,  "Minimalist",
                            "Solve with very few parts." },
    [BADGE_SPEEDRUN]    = { BADGE_SPEEDRUN,    "Speedrun",
                            "Solve under a time limit." },
    [BADGE_UNCONNECTED] = { BADGE_UNCONNECTED, "Unconnected",
                            "No piece is jointed to any other piece." },
    [BADGE_HANDS_OFF]   = { BADGE_HANDS_OFF,   "Hands Off",
                            "Left every goal object exactly where the level placed it." },
    [BADGE_DESERT]      = { BADGE_DESERT,      "Desert",
                            "No water rods." },
    [BADGE_PILOT]       = { BADGE_PILOT,       "Pilot",
                            "Solve without ever rotating a goal piece." },
    [BADGE_SOLVED]      = { BADGE_SOLVED,      "Solved",
                            "Solve a level without earning any other badge." },
};

const struct badge_info *badge_get_info(enum badge_id id) {
    if (id < 0 || id >= BADGE_NUM_BADGES) return NULL;
    return &badge_table[id];
}

uint32_t badge_check_solve(struct design *design, int64_t tick_solve) {
    uint32_t mask = 0;

    bool any_powered_wheel   = false;
    bool any_unpowered_wheel = false;
    bool any_wooden_rod      = false;
    bool any_water_rod       = false;
    int  player_piece_count  = 0;

    /* Scan the player's pieces. Goal blocks are level geometry, not the
     * player's design, so they're skipped here. */
    for (struct block *b = design->design_blocks.head; b; b = b->next) {
        if (b->goal) continue;
        player_piece_count++;

        switch (b->type_id) {
        case FCSIM_WHEEL:
            any_unpowered_wheel = true;
            break;
        case FCSIM_CW_WHEEL:
        case FCSIM_CCW_WHEEL:
            any_powered_wheel = true;
            break;
        case FCSIM_SOLID_ROD:
            any_wooden_rod = true;
            break;
        case FCSIM_ROD:
            any_water_rod = true;
            break;
        default:
            break;
        }
    }

    bool any_wheel = any_powered_wheel || any_unpowered_wheel;

    /* Negative-logic badges. All are vacuously true on an empty design. */
    if (!any_powered_wheel) {
        mask |= 1u << BADGE_GREEN;
    }
    if (!any_wheel) {
        mask |= 1u << BADGE_BROWN;
    }
    if (!any_wheel && !any_wooden_rod) {
        mask |= 1u << BADGE_BLUE;
    }
    if (!any_wheel && !any_water_rod) {
        mask |= 1u << BADGE_CARPENTER;
    }
    if (!any_wooden_rod && !any_water_rod) {
        mask |= 1u << BADGE_RODLESS;
    }

    if (player_piece_count <= MINIMALIST_MAX_PIECES) {
        mask |= 1u << BADGE_MINIMALIST;
    }
        /* Desert: no water rods. */
    if (!any_water_rod) {
        mask |= 1u << BADGE_DESERT;
    }
    /* Pilot: no goal piece rotated during the run.
     * Any real collision imparts torque, so a goal piece that just sat there
     * (or was carried rigidly) won't rotate. Vacuously true if the level has
     * no goal pieces. */
    #define PILOT_ANGLE_TOLERANCE 0.01  /* radians; ~0.57 degrees */
    bool pilot_ok = true;
    for (struct block *b = design->design_blocks.head; b; b = b->next) {
        if (!b->goal) continue;
        double delta = body_get_angle(b->body) - b->initial_angle;
        if (delta < 0) delta = -delta;
        if (delta > PILOT_ANGLE_TOLERANCE) {
            pilot_ok = false;
            break;
        }
    }
    if (pilot_ok) {
        mask |= 1u << BADGE_PILOT;
    }
        /* Solved: consolation badge. Only awarded if nothing else was earned. */
    if (mask == 0) {
        mask |= 1u << BADGE_SOLVED;
    }

    return mask;

    /* Unconnected: no joint connects two blocks (player or goal).
     * A joint connects its `gen` block (if any) to every block in `att`.
     * So "connected" means gen + att_count >= 2. */
    bool any_connection = false;
    for (struct joint *j = design->joints.head; j; j = j->next) {
        int count = j->gen ? 1 : 0;
        for (struct attach_node *a = j->att.head; a; a = a->next) {
            count++;
        }
        if (count >= 2) { any_connection = true; break; }
    }
    if (!any_connection) {
        mask |= 1u << BADGE_UNCONNECTED;
    }

    /* TODO: BADGE_SPEEDRUN — needs a tick threshold.
     * TODO: BADGE_HANDS_OFF — needs original goal-object positions to compare
     *       against. There's no such field on `struct block`, so the arena
     *       will need to snapshot goal positions at level load. */

    (void)tick_solve;
    return mask;
}