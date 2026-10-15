#ifndef BADGES_H
#define BADGES_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct design;

enum badge_id {
    BADGE_GREEN = 0,   /* no powered wheels */
    BADGE_BROWN,       /* no wheels of any kind */
    BADGE_BLUE,        /* no wheels, no wooden rods */
    BADGE_CARPENTER,   /* no wheels, no water rods */
    BADGE_RODLESS,     /* no rods of any kind */
    BADGE_DESERT,      /* no water rods */
    BADGE_PILOT,       /* goal pieces never rotated during the solve */
    BADGE_MINIMALIST,  /* few parts */
    BADGE_SPEEDRUN,    /* fast solve */
    BADGE_UNCONNECTED, /* no joints at all */
    BADGE_HANDS_OFF,   /* goal objects untouched */
    BADGE_SOLVED,      /* consolation: solved with no other badges */
    BADGE_NUM_BADGES
};

struct badge_info {
    enum badge_id id;
    const char *name;
    const char *description;
};

const struct badge_info *badge_get_info(enum badge_id id);

/* Returns a bitmask of earned badges. */
uint32_t badge_check_solve(struct design *design, int64_t tick_solve);

#ifdef __cplusplus
}
#endif

#endif