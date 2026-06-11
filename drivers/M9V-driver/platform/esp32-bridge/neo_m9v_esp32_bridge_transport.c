#include "neo_m9v_esp32_bridge_transport.h"

#include <stdio.h>
#include <string.h>

#define GPS_BRIDGE_RX_CACHE_SIZE 512u

static uint8_t gps_rx_cache[GPS_BRIDGE_RX_CACHE_SIZE];
static size_t gps_rx_cache_len = 0u;
static size_t gps_rx_cache_pos = 0u;

static int hex_value(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return -1;
}

static bool parse_hex_bytes(
    const char *hex,
    uint8_t *data,
    size_t max_len,
    size_t *bytes_read
)
{
    if (hex == 0 || data == 0 || bytes_read == 0) {
        return false;
    }

    while (*hex == ' ') {
        ++hex;
    }

    size_t count = 0u;

    while (hex[0] != '\0' && hex[1] != '\0') {
        if (count >= max_len) {
            break;
        }

        int hi = hex_value(hex[0]);
        int lo = hex_value(hex[1]);

        if (hi < 0 || lo < 0) {
            return false;
        }

        data[count++] = (uint8_t)((hi << 4) | lo);
        hex += 2;
    }

    *bytes_read = count;
    return true;
}

bool neo_m9v_esp32_bridge_write(
    const uint8_t *data,
    size_t len,
    void *user
)
{
    esp32_bridge_t *bridge = (esp32_bridge_t *)user;

    if (bridge == 0 || data == 0) {
        return false;
    }

    if (len > 256u) {
        return false;
    }

    char command[16 + 2 * 256 + 1];
    size_t offset = 0u;

    offset += (size_t)snprintf(command + offset, sizeof(command) - offset, "@GPS_WRITE ");

    for (size_t i = 0u; i < len; ++i) {
        offset += (size_t)snprintf(
            command + offset,
            sizeof(command) - offset,
            "%02X",
            data[i]
        );
    }

    char response[128];

    if (!esp32_bridge_command(
            bridge,
            command,
            response,
            sizeof(response),
            2000u
        )) {
        return false;
    }

    if (strcmp(response, "@OK") != 0) {
        fprintf(stderr, "GPS write bridge error: %s\n", response);
        return false;
    }

    return true;
}

bool neo_m9v_esp32_bridge_read(
    uint8_t *data,
    size_t max_len,
    size_t *bytes_read,
    uint32_t timeout_ms,
    void *user
)
{
    esp32_bridge_t *bridge = (esp32_bridge_t *)user;

    if (bridge == 0 || data == 0 || bytes_read == 0) {
        return false;
    }

    *bytes_read = 0u;

    if (max_len == 0u) {
        return true;
    }

    /*
     * Serve cached bytes first.
     *
     * The portable UBX parser often asks for one byte at a time while looking
     * for B5 62 sync. That is cheap on a real UART but expensive through the
     * ESP32 bridge because every read becomes an @GPS_READ command. This cache
     * makes the bridge act more like a real byte stream.
     */
    if (gps_rx_cache_pos < gps_rx_cache_len) {
        size_t available = gps_rx_cache_len - gps_rx_cache_pos;
        size_t to_copy = available < max_len ? available : max_len;

        memcpy(data, &gps_rx_cache[gps_rx_cache_pos], to_copy);

        gps_rx_cache_pos += to_copy;
        *bytes_read = to_copy;

        if (gps_rx_cache_pos >= gps_rx_cache_len) {
            gps_rx_cache_pos = 0u;
            gps_rx_cache_len = 0u;
        }

        return true;
    }

    /*
     * Cache is empty. Ask the ESP32-C6 for a larger chunk.
     */
    char command[64];

    snprintf(
        command,
        sizeof(command),
        "@GPS_READ %u %u",
        (unsigned int)timeout_ms,
        (unsigned int)GPS_BRIDGE_RX_CACHE_SIZE
    );

    char response[1600];

    if (!esp32_bridge_command(
            bridge,
            command,
            response,
            sizeof(response),
            timeout_ms + 2000u
        )) {
        return false;
    }

    if (strncmp(response, "@OK", 3) != 0) {
        fprintf(stderr, "GPS read bridge error: %s\n", response);
        return false;
    }

    if (response[3] == '\0') {
        return true;
    }

    if (response[3] != ' ') {
        return false;
    }

    size_t parsed = 0u;

    if (!parse_hex_bytes(
            &response[4],
            gps_rx_cache,
            GPS_BRIDGE_RX_CACHE_SIZE,
            &parsed
        )) {
        return false;
    }

    gps_rx_cache_len = parsed;
    gps_rx_cache_pos = 0u;

    if (gps_rx_cache_len == 0u) {
        return true;
    }

    size_t to_copy = gps_rx_cache_len < max_len ? gps_rx_cache_len : max_len;

    memcpy(data, gps_rx_cache, to_copy);

    gps_rx_cache_pos = to_copy;
    *bytes_read = to_copy;

    if (gps_rx_cache_pos >= gps_rx_cache_len) {
        gps_rx_cache_pos = 0u;
        gps_rx_cache_len = 0u;
    }

    return true;
}
