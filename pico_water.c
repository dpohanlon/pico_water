#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "pico/util/queue.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"

#define WET_THRESHOLD 1.5 // Volts, moisture sensor ADC voltage ABOVE which we will water

#define WARN_HIGH_THRESHOLD 1.45 // Volts, moisture sensor ADC voltage BELOW which we trigger the 'too wet' alarm
#define WARN_LOW_THRESHOLD 1.65 // Volts, moisture sensor ADC voltage ABOVE which we trigger the 'too dry' alarm

#define ADC_PIN 26
#define TRIGGER_PIN 0
#define TEMPERATURE_PIN 29

#define MONITOR_INTERVAL 10 // seconds, interval between ADC poll
#define RESPONSE_INTERVAL 60 // seconds, interval between checking moisture sensor whilst watering
#define WATER_INTERVAL 3600 // seconds, interval between commencing watering

#define MAX_ITER 10 // Maximum number of iterations of the watering loop before it terminates (probably due to issue)

#define FLOW_TIME 1 // seconds, length of time the pump is on

#define MOISTURE 0
#define TEMP 4

queue_t moisture_q;

// Assuming 3.3 V reference, 12 bit ADC
const float adc_conversion_factor = 3.3f / (1 << 12);

float moisture_value() {

  adc_select_input(MOISTURE);
  uint16_t result = adc_read();

  return result * adc_conversion_factor;

}

float temperature_value() {

  adc_select_input(TEMP);
  uint16_t result = adc_read();

  float temperature = 27. - (result * adc_conversion_factor - 0.706)/0.001721;

  return temperature;

}

void add_to_q(float moist) {

  if (!queue_is_full(&moisture_q)) {

    queue_add_blocking(&moisture_q, &moist);

  } else {

    float tmp;

    // try_remove to avoid deadlock situations where the producer
    // is blocking due to this already being removed by the consumer

    queue_try_remove(&moisture_q, &tmp);
    queue_add_blocking(&moisture_q, &moist);

  }

}

float remove_from_q() {

  float tmp;
  queue_remove_blocking(&moisture_q, &tmp);

  return tmp;

}

void water() {

  gpio_put(TRIGGER_PIN, 1);

  sleep_ms(1000 * FLOW_TIME);

  gpio_put(TRIGGER_PIN, 0);

}

void water_loop() {

  int n_iter = 0;

  while ((remove_from_q() > WET_THRESHOLD) && (n_iter < MAX_ITER)) {

    water();

    // Wait some time for the water to percolate before we check again
    sleep_ms(1000 * RESPONSE_INTERVAL);

    n_iter += 1;

  }

}

void monitor() {

  while (true) {

    float moist = moisture_value();
    float temp = temperature_value();

    printf("%f %f\n", moist, temp);

    add_to_q(moist);

    sleep_ms(1000 * MONITOR_INTERVAL);

  }

}

void consume_monitor_debug() {

  while (true) {

    float moist = remove_from_q();

    printf("CONSUMER %f \n", moist);

    sleep_ms(1000);

  }

}

void init() {

  stdio_init_all();

  adc_init();
  adc_gpio_init(ADC_PIN);
  adc_gpio_init(TEMPERATURE_PIN);

  adc_set_temp_sensor_enabled(true);

  gpio_init(TRIGGER_PIN);
  gpio_set_dir(TRIGGER_PIN, GPIO_OUT);

}

int main() {

  init();

  queue_init(&moisture_q, sizeof(float), 1);

  multicore_launch_core1(monitor);

  while (true){

    water_loop();

    sleep_ms(1000 * WATER_INTERVAL);

  }

  return 0;
}
