#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "neighbour_table.h"

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