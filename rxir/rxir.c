#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"

const uint32_t IR_PWR_PIN = 21;
const uint32_t IR_DATA_PIN = 22;

const size_t BUF_LEN = 256;

void print_buffer(uint32_t *buffer) {
    for (size_t i = 0; i < BUF_LEN; i++) {
        printf("%d,", buffer[i]);
    }
    puts("\n");
}

int main() {
    bi_decl(bi_program_description("Receive IR"));
    bi_decl(bi_1pin_with_name(IR_PWR_PIN, "IR power"));
    bi_decl(bi_1pin_with_name(IR_DATA_PIN, "IR data"));
    bi_decl(bi_1pin_with_name(CYW43_WL_GPIO_LED_PIN, "On-board LED"));

    stdio_init_all();
    cyw43_arch_init();

    gpio_init(IR_DATA_PIN);
    gpio_set_dir(IR_DATA_PIN, GPIO_IN);
    gpio_pull_up(IR_DATA_PIN);

    gpio_init(IR_PWR_PIN);
    gpio_set_dir(IR_PWR_PIN, GPIO_OUT);
    gpio_put(IR_PWR_PIN, 1);

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(100);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

    puts("Waiting for input...\n");

    bool last = false;
    uint32_t counter = 0;
    uint32_t buffer[BUF_LEN];
    size_t index = 0;

    while (true) {
        if (last == gpio_get(IR_DATA_PIN)) {
            counter++;
        }
        else {
            last = !last;
            buffer[index] = counter;
            counter = 1;
            index++;
            if (index >= BUF_LEN) {
                index = 0;
                print_buffer(buffer);
            }
        }
    }
}
