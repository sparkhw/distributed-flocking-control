#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MAX_NEIGHBOURS 3

typedef struct {
    uint8_t node_id;
    uint32_t x_mm;
    uint32_t y_mm;
    uint32_t z_mm;
    int32_t vx_mms;
    int32_t vy_mms;
    int32_t vz_mms;
    uint16_t seq;
} agent_t;

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

// Constraints
static uint32_t minPos = 0;
static uint32_t maxPos = 100000;

static uint32_t maxSpeed = 19000; // Maximum speed limit in UK
// static uint32_t maxSpeed = 44703; // Maximum speed limit in USA

// static uint32_t maxAccel =

// Simulation Parameters
#define NUM_AGENTS 3
#define AGENT_UPDATE_DT_MS 100

// Parameters
static uint32_t timeout_ms = 3000;

static agent_t agents[NUM_AGENTS];
static neighbour_entry_t neighbour_table[MAX_NEIGHBOURS];

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

            // Existing entry for this node_id → update in place
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

        // No existing entry – either use free slot or overwrite oldest
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
            // printf("Expiring neighbout %u from slot %d (age = %u ms)\n",
            //        neighbour_table[i].node_id, i, age);
            neighbour_table[i].in_use = false;
        }
    }
}

void neighbour_expiry_task(void *arg)
{
    while (1) {
        uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        neighbour_expire(now_ms, timeout_ms);

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

static int32_t random_velocity(void)
{
    const int32_t v_min = -maxSpeed;
    const int32_t v_max =  maxSpeed;

    int32_t r = rand() % (v_max - v_min + 1);
    return v_min + r;
}

static void update_agent_position(agent_t *a, uint32_t dt_ms)
{
    int64_t dt = (int64_t)dt_ms;

    // Compute deltas: dx = vx * dt_ms / 1000
    int64_t dx = (int64_t)a->vx_mms * dt / 1000;
    int64_t dy = (int64_t)a->vy_mms * dt / 1000;
    int64_t dz = (int64_t)a->vz_mms * dt / 1000;

    int64_t x = (int64_t)a->x_mm + dx;
    int64_t y = (int64_t)a->y_mm + dy;
    int64_t z = (int64_t)a->z_mm + dz;

    int64_t max = (int64_t)maxPos;

    if (x < 0) {
        x = -x;
        a->vx_mms = -a->vx_mms;
    } else if (x > max) {
        x = 2 * max - x;
        a->vx_mms = -a->vx_mms;
    }

    if (y < 0) {
        y = -y;
        a->vy_mms = -a->vy_mms;
    } else if (y > max) {
        y = 2 * max - y;
        a->vy_mms = -a->vy_mms;
    }

    if (z < 0) {
        z = -z;
        a->vz_mms = -a->vz_mms;
    } else if (z > max) {
        z = 2 * max - z;
        a->vz_mms = -a->vz_mms;
    }

    // Cast back to uint32_t (we know they are in range now)
    a->x_mm = (uint32_t)x;
    a->y_mm = (uint32_t)y;
    a->z_mm = (uint32_t)z;
}


void app_main(void)
{

}