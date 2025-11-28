#ifndef NEIGHBOUR_TABLE_H
#define NEIGHBOUR_TABLE_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_NEIGHBOURS 3

typedef struct {
    uint8_t  node_id;
    uint32_t x_mm;
    uint32_t y_mm;
    uint32_t z_mm;
    int32_t  vx_mms;
    int32_t  vy_mms;
    int32_t  vz_mms;
    uint16_t seq;
    uint32_t last_heard_ms;
    bool     in_use;
} neighbour_entry_t;

// Global table
extern neighbour_entry_t neighbour_table[MAX_NEIGHBOURS];

// API functions
void neighbour_init(void);
void neighbour_update(uint8_t node_id, 
                      uint32_t x_mm, 
                      uint32_t y_mm, 
                      uint32_t z_mm,
                      int32_t  vx_mms,
                      int32_t  vy_mms,
                      int32_t  vz_mms,
                      uint16_t seq,
                      uint32_t now_ms);
void neighbour_expire(uint32_t now_ms, uint32_t timeout_ms);
void neighbour_print_summary(uint32_t now_ms);

#endif