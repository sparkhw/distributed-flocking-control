#include "neighbour_table.h"

void neighbour_init(void)
{
    for (int i = 0; i < MAX_NEIGHBOURS; i++) {
        neighbour_table[i].in_use = false;
    }
}

void neighbour_update(uint8_t  node_id,
                      uint32_t x_mm,
                      uint32_t y_mm,
                      uint32_t z_mm,
                      int32_t  vx_mms,
                      int32_t  vy_mms,
                      int32_t  vz_mms,
                      uint16_t seq,
                      uint32_t now_ms)
{

}

void neighbour_expire(uint32_t now_ms, uint32_t timeout_ms)
{
    
}

void neighbour_print_summary(void)
{

}