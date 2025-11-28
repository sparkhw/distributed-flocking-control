#include "neighbour_table.h"
#include <stdio.h>

void neighbour_init(void)
{
    for (int i = 0; i < MAX_NEIGHBOURS; i++) {
        neighbour_table[i].in_use = false;
    }
}

void neighbour_update(uint8_t node_id, 
                      uint32_t x_mm, 
                      uint32_t y_mm, 
                      uint32_t z_mm,
                      int32_t  vx_mms,
                      int32_t  vy_mms,
                      int32_t  vz_mms,
                      uint16_t seq,
                      uint32_t now_ms)
{
    int free_index = -1;
    int oldest_index = -1;
    uint32_t oldest_time = 0;

    // First pass: look for existing entry, remember first free slot and oldest entry
    for (int i = 0; i < MAX_NEIGHBOURS; i++) {

        if (neighbour_table[i].in_use) {
            // Track oldest
            if (oldest_index < 0 || neighbour_table[i].last_heard_ms < oldest_time) {
                oldest_index = i;
                oldest_time = neighbour_table[i].last_heard_ms;
            }

            // Update existing entry for node_id in place
            if (neighbour_table[i].node_id == node_id) {
                neighbour_table[i].x_mm = x_mm;
                neighbour_table[i].y_mm = y_mm;
                neighbour_table[i].z_mm = z_mm;

                neighbour_table[i].vx_mms = vx_mms;
                neighbour_table[i].vy_mms = vy_mms;
                neighbour_table[i].vz_mms = vz_mms;

                neighbour_table[i].seq = seq;
                neighbour_table[i].last_heard_ms = now_ms;

                printf("Updated neighbour %u in slot %d\n", node_id, i);
                return;
            }
        } else {
            // Remember first free slot
            if (free_index < 0) {
                free_index = i;
            }
        }

        // No existing entry – use free slot or overwrite oldest
        int target = free_index;

        if (target < 0) {
            // Table full: overwrite oldest entry
            target = oldest_index;
            printf("Neighbour table full, replacing slot %d (node %u)\n",
                   target, neighbour_table[target].node_id);
        } else {
            printf("Inserting neighbour %u into free slot %d\n", node_id, target);
        }

        neighbour_table[target].node_id = node_id;

        neighbour_table[target].x_mm = x_mm;
        neighbour_table[target].y_mm = y_mm;
        neighbour_table[target].z_mm = z_mm;

        neighbour_table[target].vx_mms = vx_mms;
        neighbour_table[target].vy_mms = vy_mms;
        neighbour_table[target].vz_mms = vz_mms;

        neighbour_table[target].seq = seq;
        neighbour_table[target].last_heard_ms = now_ms;

        neighbour_table[target].in_use = true;
    }    
}

void neighbour_expire(uint32_t now_ms, uint32_t timeout_ms)
{
    for (int i = 0; i < MAX_NEIGHBOURS; i++) {
        if (!neighbour_table[i].in_use) {
            continue;
        }

        uint32_t age = now_ms - neighbour_table[i].last_heard_ms;
        
        if (age > timeout_ms) {
            neighbour_table[i].in_use = false;
        }
    }
}

void neighbour_print_summary(uint32_t now_ms)
{
    printf("Neighbour Table\n");

    for (int i = 0; i < MAX_NEIGHBOURS; i++) {
        neighbour_entry_t *e = &neighbour_table[i];
        if (!e->in_use) continue;

        uint32_t age = now_ms - e->last_heard_ms;

        printf("id=%u pos=(%lu,%lu,%lu) vel=(%ld,%ld,%ld) "
               "seq=%u age=%lums\n",
               e->node_id,
               e->x_mm, e->y_mm, e->z_mm,
               (long)e->vx_mms, (long)e->vy_mms, (long)e->vz_mms,
               e->seq,
               age);
    }
}