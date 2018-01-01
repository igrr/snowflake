/* This code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <libstm8/stm8l.h>
#include <stdint.h>

#define PWM_RANGE 1024

/* Enable the timer used for timekeeping */
static void enable_clock();

/* Return current tick count */
static uint16_t clock(void);

/* LED channels definition */
typedef struct {
    uint16_t clk;           /* Peripheral mask for clock enable */
    uint16_t timer_periph;  /* Timer peripheral base */
    enum tim_oc_id oc_id;   /* Output compare unit */
    int gpio;               /* GPIO (assumed in port B) */
} channel_t;

const channel_t channels[3] = {
    { .clk = CLK_TIM2, .timer_periph = TIM2_BASE, .oc_id = TIM_OC1, .gpio = GPIO0 },
    { .clk = CLK_TIM3, .timer_periph = TIM3_BASE, .oc_id = TIM_OC1, .gpio = GPIO1 },
    { .clk = CLK_TIM2, .timer_periph = TIM2_BASE, .oc_id = TIM_OC2, .gpio = GPIO2 },
};

/* Initialize PWM output for a channel */
static void init_pwm(int ch);

/* Set PWM output value for a channel */
static void set_pwm(int ch, int val);

/* Go to low power mode */
static void halt();

/* State handling.
   First byte of EEPROM is used to store the last state: halted (no LEDs blinking) or running (LEDs are blinking) */

typedef enum {
    STATE_UNKNOWN = 0,
    STATE_HALTED,
    STATE_RUNNING 
} state_t;

#define EEPROM(i) (*(uint8_t *) (0x1000 + (i)))

/* Read and write state from/to EEPROM */
static void eeprom_write_state(state_t state);
static state_t eeprom_read_state();

/* If previous state is running, go to halted state.
   Otherwise, go to running state */
static void reset_reason_check();

/* Generate sawtooth shaped signal between 0 and 255. */
static uint16_t sawtooth(uint16_t t, uint16_t period);
#define SAWTOOTH_RANGE 256

/* Time (in some units...) when to halt */
#define MAX_OVERFLOW_COUNT 10

void main(void)
{
    int channel;
    uint16_t clk;
    uint16_t last_clk = 0;
    int ovf = 0;

    reset_reason_check();

    CLK_DIVR = 0x03; // Set the frequency to 16/3 MHz

    for (channel = 0; channel < 3; ++channel) {
        init_pwm(channel);
    }

    enable_clock();

    while(1) {
        clk = clock();
        if (clk < last_clk) {
            ovf++;
            if (ovf == MAX_OVERFLOW_COUNT) {
                halt();
            }
        }
        if (clk != last_clk) {
            set_pwm(0,
                (sawtooth(clk, 70) 
                + sawtooth(clk + 30, 40)
                + sawtooth(clk + 10, 130)) / 3);

            set_pwm(1,
                (sawtooth(clk, 110)
                + sawtooth(clk + 20, 70)
                + sawtooth(clk + 15, 30)) / 3);

            set_pwm(2,
                (sawtooth(clk, 50)
                + sawtooth(clk + 10, 90)
                + sawtooth(clk + 20, 110)) / 3);

            last_clk = clk;
        }
    }
}


static void enable_clock()
{
    clk_periph_clock_enable(CLK_TIM4);
    timer_set_mode(TIM4_BASE, TIM_CMS_EDGE, TIM_DIR_UP);
    timer_set_period(TIM4_BASE, 0xff);
    timer_set_prescaler(TIM4_BASE, 15);
    timer_enable_counter(TIM4_BASE);
}

static uint16_t clock(void)
{
    static uint8_t last_cnt = 0;
    uint8_t cnt;
    static uint16_t t = 0;

    cnt = TIM4_CNTR;
    if (cnt != last_cnt) {
        last_cnt = cnt;
        t++;
    }

    return t;
}

static void init_pwm(int ch)
{
    uint16_t t = channels[ch].timer_periph;
    enum tim_oc_id oc = channels[ch].oc_id;
    gpio_mode_setup (GPIOB, GPIO_MODE_OUTPUT, GPIO_CR1_PP, GPIO_CR2_2, channels[ch].gpio);
    clk_periph_clock_enable(channels[ch].clk);
    timer_set_mode(t, TIM_CMS_EDGE, TIM_DIR_UP);
    timer_enable_preload(t);
    timer_set_period(t, PWM_RANGE);
    timer_set_oc_mode(t, oc, TIM_OCM_PWM_1);
    timer_enable_oc_preload(t, oc);
    timer_set_oc_value(t, oc, PWM_RANGE / 2);
    timer_enable_oc_output(t, oc);
    timer_enable_break_main_output(t);
    timer_enable_counter(t);
}

static void set_pwm(int ch, int val) 
{
    timer_set_oc_value(channels[ch].timer_periph,
                       channels[ch].oc_id,
                       val);
}

static void halt()
{
    __asm;
    halt
    __endasm;
}

static void eeprom_write_state(state_t state)
{
    FLASH_DUKR = FLASH_DUKR_1;
    FLASH_DUKR = FLASH_DUKR_2;
    EEPROM(0) = state;
}

static state_t eeprom_read_state()
{
    state_t result = EEPROM(0);
    if (result != STATE_HALTED && result != STATE_RUNNING) {
        result = STATE_UNKNOWN;
    }
    return result;
}

static void reset_reason_check()
{
    state_t state = eeprom_read_state();
    if (state == STATE_RUNNING) {
        eeprom_write_state(STATE_HALTED);
        halt();
    } else {
        eeprom_write_state(STATE_RUNNING);
    }
}

static uint16_t sawtooth(uint16_t t, uint16_t period)
{
    uint16_t mod;
    uint16_t v;

    mod = t % (2 * period);
    if (mod < period) {
        v = (mod * SAWTOOTH_RANGE) / period;
    } else {
        v = (SAWTOOTH_RANGE - (mod - period) * SAWTOOTH_RANGE / period);
    }

    return (v * (v - 1)) / SAWTOOTH_RANGE;
}
