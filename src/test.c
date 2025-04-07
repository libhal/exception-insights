#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#define MAX_TIME 10000
#define DELAY_MS 3

void delay_ms(uint32_t count)
{
    uint32_t start;
    uint64_t end, diff;
    start = Timer32_getValue(TIMER32_0_BASE);
    diff = ((uint64_t)count * CS_getMCLK()) / 1000;
    end = start - diff;

    if (count == 0) {
        return;
    }

    while (Timer32_getValue(TIMER32_0_BASE) > end) {
    }

    return;
}

void calibrate(uint32_t count, uint32_t* freq_threshold, bool* check)
{

    uint32_t sum = 0, time_temp, freq_temp;

    int i;
    for (i = 0; i < count; i++) {
        Timer_A_clearTimer(TIMER_A2_BASE);
        delay_ms(DELAY_MS);
        time_temp = Timer_A_getCounterValue(TIMER_A2_BASE);
        freq_temp = (time_temp * 1000) / (DELAY_MS);
        sum += freq_temp;
    }
    *freq_threshold = (sum / count) * 0.8;
    *check = true;
}
int main(void)
{

    WDT_A_holdTimer();

    uint32_t frequency, counter, current_freq;
    bool threshold_check = false;

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    CAPTIO0CTL = 0;
    CAPTIO0CTL |= 0x0100;
    CAPTIO0CTL |= 4 << 4;
    CAPTIO0CTL |= 1 << 1;

    Timer32_initModule(TIMER32_0_BASE,
                       TIMER32_PRESCALER_1,
                       TIMER32_32BIT,
                       TIMER32_FREE_RUN_MODE);
    Timer32_startTimer(TIMER32_0_BASE, 0);

    Timer_A_ContinuousModeConfig timer_continuous_obj;
    timer_continuous_obj.clockSource =
      TIMER_A_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK;
    timer_continuous_obj.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    timer_continuous_obj.timerInterruptEnable_TAIE =
      TIMER_A_TAIE_INTERRUPT_DISABLE;
    timer_continuous_obj.timerClear = TIMER_A_DO_CLEAR;
    Timer_A_configureContinuousMode(TIMER_A2_BASE, &timer_continuous_obj);
    Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_CONTINUOUS_MODE);

    calibrate(10, &frequency, &threshold_check);

    printf("\nThreshold Frequency: %d Hz\n", frequency);

    while (1) {
        Timer_A_clearTimer(TIMER_A2_BASE);
        delay_ms(DELAY_MS);
        counter = Timer_A_getCounterValue(TIMER_A2_BASE);
        current_freq = (counter * 1000) / (DELAY_MS);

        if (current_freq < frequency && threshold_check == true) {
            GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
            threshold_check = false;
        }

        if (current_freq > frequency) {
            threshold_check = true;
        }
    }
}
