#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"

const uint32_t IR_TX_PIN = 15;

const uint32_t NEC_HEADER_PULSES = 342;     // 9 ms
const uint32_t NEC_HEADER_PAUSE = 4500;     // 4.5 ms
const uint32_t NEC_DATA_PULSES = 21;        // 0.5625 ms
const uint32_t NEC_DATA_PAUSE_0 = 562;      // 0.5625 ms
const uint32_t NEC_DATA_PAUSE_1 = 1687;     // 1.6875 ms
const uint32_t NEC_FRAME_LEN = 32;

const uint32_t SONY_HEADER_PULSES = 96;     // 2.4 ms
const uint32_t SONY_HEADER_PAUSE = 600;     // 0.6 ms
const uint32_t SONY_DATA_PULSES_0 = 24;     // 0.6 ms
const uint32_t SONY_DATA_PULSES_1 = 48;     // 1.2 ms
const uint32_t SONY_DATA_PAUSE = 600;       // 0.6 ms
const uint32_t SONY_FRAME_LEN = 12;

const uint32_t TX_PAUSE_38KHZ = 13;
const uint32_t TX_PAUSE_40KHZ = 12;         // 13 should also work
const uint32_t SECOND = 1000000;

const uint32_t NEC_COMMANDS[32] = {
    0b11111100000000111110111100000000,
    0b11111101000000101110111100000000,
    0b11111110000000011110111100000000,
    0b11111111000000001110111100000000,
    0b11111000000001111110111100000000,
    0b11111001000001101110111100000000,
    0b11111010000001011110111100000000,
    0b11111011000001001110111100000000,
    0b11110100000010111110111100000000,
    0b11110101000010101110111100000000,
    0b11110110000010011110111100000000,
    0b11110111000010001110111100000000,
    0b11110000000011111110111100000000,
    0b11110001000011101110111100000000,
    0b11110010000011011110111100000000,
    0b11110011000011001110111100000000,
    0b11101100000100111110111100000000,
    0b11101101000100101110111100000000,
    0b11101110000100011110111100000000,
    0b11101111000100001110111100000000,
    0b11101000000101111110111100000000,
    0b11101001000101101110111100000000,
    0b11101010000101011110111100000000,
    0b11101011000101001110111100000000,
};

const uint32_t SONY_COMMANDS[1] = {
    0b100000010101,
};

static inline void tx_pulses(const size_t pulses, const uint32_t pause) {
    for (size_t i = 0; i < pulses; i++) {
        gpio_put(IR_TX_PIN, 1);
        busy_wait_us(pause);
        gpio_put(IR_TX_PIN, 0);
        busy_wait_us(pause);
    }
}

static inline void tx_nec_header() {
    tx_pulses(NEC_HEADER_PULSES, TX_PAUSE_38KHZ);
    busy_wait_us(NEC_HEADER_PAUSE);
}

static inline void tx_sony_header() {
    tx_pulses(SONY_HEADER_PULSES, TX_PAUSE_40KHZ);
    busy_wait_us(SONY_HEADER_PAUSE);
}

static inline void tx_nec_bit(bool bit) {
    tx_pulses(NEC_DATA_PULSES, TX_PAUSE_38KHZ);
    busy_wait_us(bit ? NEC_DATA_PAUSE_1 : NEC_DATA_PAUSE_0);
}

static inline void tx_sony_bit(bool bit) {
    tx_pulses(bit ? SONY_DATA_PULSES_1 : SONY_DATA_PULSES_0, TX_PAUSE_40KHZ);
    busy_wait_us(SONY_DATA_PAUSE);
}

static inline void tx_nec_close() {
    tx_pulses(NEC_DATA_PULSES, TX_PAUSE_38KHZ);
}

static inline void tx_nec(uint32_t data) {
    tx_nec_header();
    for (size_t i = 0; i < NEC_FRAME_LEN; i++) {
        tx_nec_bit(data & 1);
        data >>= 1;
    }
    tx_nec_close();
}

static inline void tx_sony(uint32_t data) {
    tx_sony_header();
    for (size_t i = 0; i < SONY_FRAME_LEN; i++) {
        tx_sony_bit(data & 1);
        data >>= 1;
    }
}

static inline void tx_benchmark(const uint32_t pulses, size_t pause) {
    uint64_t begin = time_us_64();
    tx_pulses(pulses, pause);
    uint64_t end = time_us_64();
    uint64_t delta = end - begin;
    printf("benchmark: %llu us\n", delta);
}

int main() {
    bi_decl(bi_program_description("Transmit IR"));
    bi_decl(bi_1pin_with_name(IR_TX_PIN, "IR transmit"));
    bi_decl(bi_1pin_with_name(CYW43_WL_GPIO_LED_PIN, "On-board LED"));

    stdio_init_all();
    cyw43_arch_init();

    gpio_init(IR_TX_PIN);
    gpio_set_dir(IR_TX_PIN, GPIO_OUT);

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(100);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

    // tx_benchmark(380000, TX_PAUSE_38KHZ);
    // tx_benchmark(400000, TX_PAUSE_40KHZ);

    puts("Transmitting...\n");

    while (true) {
        tx_sony(SONY_COMMANDS[0]);
        busy_wait_us(SECOND);
    }
}
