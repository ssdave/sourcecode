/**
 * @file i2s_bitbang_driver.c
 * @brief Digital Audio Serial Interface Bit-Banged I2S Bus Controller and Peripheral Driver.
 *
 * Concepts Demonstrated:
 * 1. Synchronous 3-wire digital audio serial bus: BCLK (Bit Clock), WS / LRCK (Word Select),
 *    and SD (Serial Data).
 * 2. Official Philips I2S specification compliance: Left channel (WS = 0), Right channel (WS = 1).
 * 3. The iconic 1-BCLK period delay between Word Select (WS) transition and the first data bit
 * (MSB).
 * 4. 16-bit two's complement stereo PCM audio serialization (MSB first down to LSB).
 * 5. Complete transmitter (DAC driver) and receiver (ADC sampler) interfaces verified against
 *    a mock digital audio codec with hardware timing and frame synchronization validation.
 *
 * I2S (Inter-IC Sound) Bus Architecture & Philips Timing Invariants:
 *
 * 3-Wire Physical Bus Lines:
 * - BCLK (Bit Clock / Continuous Serial Clock): Clocks each discrete bit.
 * - WS / LRCK (Word Select / Left-Right Clock):
 *     * WS = 0 (LOW): Left Audio Channel
 *     * WS = 1 (HIGH): Right Audio Channel
 * - SD (Serial Data): Signed two's complement PCM audio samples, transmitted MSB first.
 *
 * Signal Timing Invariants:
 * - Transmitter updates WS and SD on the FALLING edge of BCLK.
 * - Receiver samples WS and SD on the RISING edge of BCLK.
 * - The MSB of the audio word appears on SD exactly ONE BCLK period AFTER the WS transition!
 *
 * Philips I2S Timing Diagram (16-bit Audio Word):
 *
 *             _   _   _   _   _   _   _   _       _   _   _   _   _   _
 *   BCLK:   _| |_| |_| |_| |_| |_| |_| |_| |_ ... _| |_| |_| |_| |_| |_ ...
 *
 *           -------.                               .-----------------------
 *   WS:            |_______________________________|
 *                  ^ Left Channel (WS = 0)         ^ Right Channel (WS = 1)
 *
 *           -------._______+-------+-------+-------._______+-------+-------
 *   SD:            | Delay | Bit 15| Bit 14| Bit 13| Delay | Bit 15| Bit 14
 *           -------'-------+-------+-------+-------'-------+-------+-------
 *                  ^       ^ (MSB)                 ^       ^ (MSB)
 *             1-BCLK Delay                    1-BCLK Delay
 *
 * Academic Attribution:
 * Exercise design based on digital audio signal processing and computer architecture
 * (CS:APP Chapter 6, OSTEP Chapter 36, and Philips Semiconductors I2S Bus Specification Feb 1986).
 * Retained strictly for independent educational self-study.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==============================================================================
 * Hardware Abstraction Layer (HAL) & Protocol Definitions
 * ============================================================================== */

#define I2S_CHANNEL_LEFT 0
#define I2S_CHANNEL_RIGHT 1

#define I2S_BITS_PER_SAMPLE 16

/**
 * Mock Digital Audio Codec / DAC & ADC Peripheral State
 */
typedef struct
{
  /* Receiver (DAC) state */
  uint8_t rx_channel;        /* Current active channel (0 = Left, 1 = Right) */
  bool rx_delay_pending;     /* Waiting for 1-BCLK delay slot */
  uint8_t rx_bit_count;      /* Number of audio bits received in current word */
  uint16_t rx_shift_reg;     /* Shift register assembling incoming sample */
  int16_t last_left_sample;  /* Most recently received complete Left sample */
  int16_t last_right_sample; /* Most recently received complete Right sample */
  uint32_t total_frames_rx;  /* Total complete stereo frames received */
  uint32_t sync_errors;      /* Synchronization and timing protocol violations */

  /* Transmitter (ADC) state for receiver test verification */
  int16_t tx_left_pattern;  /* Pattern loaded to simulate incoming Left sample */
  int16_t tx_right_pattern; /* Pattern loaded to simulate incoming Right sample */
  uint16_t tx_shift_reg;    /* Shift register delivering outgoing bits to master */
  uint8_t tx_bit_count;     /* Number of bits shifted out */
} mock_i2s_codec_t;

/**
 * I2S Bus Controller Context
 */
typedef struct
{
  uint8_t bclk;           /* Bit Clock line state: 0 or 1 */
  uint8_t ws;             /* Word Select line state: 0 (Left) or 1 (Right) */
  uint8_t sd_tx;          /* Serial Data line driven by Master (Transmitter) */
  uint8_t sd_rx;          /* Serial Data line driven by Slave Codec (Receiver) */
  mock_i2s_codec_t codec; /* Mock connected audio codec peripheral */
  uint32_t clock_cycles;  /* Total BCLK clock cycles recorded */
} i2s_bus_t;

/* Forward declaration for internal codec bus transition handler */
static void mock_codec_on_bclk_edge(i2s_bus_t *bus, uint8_t edge_type);

/**
 * Samples the physical logic level of the Serial Data line driven by the slave (ADC).
 *
 * @param bus Pointer to the I2S bus structure.
 * @return 0 or 1.
 */
uint8_t i2s_hal_get_sd(const i2s_bus_t *bus)
{
  if(bus == NULL)
  {
    return 0;
  }
  return bus->sd_rx;
}

/**
 * Sets the driver level for the Bit Clock (BCLK) line.
 *
 * @param bus Pointer to the I2S bus structure.
 * @param level 0 for LOW, 1 for HIGH.
 */
void i2s_hal_set_bclk(i2s_bus_t *bus, uint8_t level)
{
  if(bus == NULL)
  {
    return;
  }

  uint8_t old_bclk = bus->bclk;
  bus->bclk = (level != 0) ? 1 : 0;

  if(old_bclk != bus->bclk)
  {
    bus->clock_cycles++;
    /* edge_type: 1 = rising (0 -> 1), 0 = falling (1 -> 0) */
    mock_codec_on_bclk_edge(bus, bus->bclk);
  }
}

/**
 * Sets the driver level for the Word Select (WS / LRCK) line.
 *
 * @param bus Pointer to the I2S bus structure.
 * @param level 0 for Left Channel, 1 for Right Channel.
 */
void i2s_hal_set_ws(i2s_bus_t *bus, uint8_t level)
{
  if(bus == NULL)
  {
    return;
  }
  bus->ws = (level != 0) ? 1 : 0;
}

/**
 * Sets the master driver level for the Serial Data (SD) line.
 *
 * @param bus Pointer to the I2S bus structure.
 * @param level 0 or 1.
 */
void i2s_hal_set_sd(i2s_bus_t *bus, uint8_t level)
{
  if(bus == NULL)
  {
    return;
  }
  bus->sd_tx = (level != 0) ? 1 : 0;
}

/**
 * Simulated bus timing delay for half-clock cycles and setup/hold intervals.
 */
void i2s_hal_delay(void)
{
  /* Simulated hardware propagation and clock period delay */
}

/* ==============================================================================
 * Mock Audio Codec Peripheral Implementation
 * ============================================================================== */

/**
 * Initializes the mock digital audio codec.
 *
 * @param codec Pointer to the mock codec structure.
 */
void mock_codec_init(mock_i2s_codec_t *codec)
{
  if(codec == NULL)
  {
    return;
  }

  memset(codec, 0, sizeof(*codec));
  codec->rx_channel = I2S_CHANNEL_LEFT;
  codec->rx_delay_pending = true;
  codec->rx_bit_count = 0;
  codec->rx_shift_reg = 0;
  codec->tx_left_pattern = (int16_t)0x1234;
  codec->tx_right_pattern = (int16_t)0x5678;
}

/**
 * Internal handler invoked on every BCLK transition.
 */
static void mock_codec_on_bclk_edge(i2s_bus_t *bus, uint8_t edge_type)
{
  mock_i2s_codec_t *codec = &bus->codec;

  /* RISING EDGE (0 -> 1): Codec samples master data lines */
  if(edge_type == 1)
  {
    uint8_t current_ws = bus->ws;

    /* Detect channel transition */
    if(current_ws != codec->rx_channel)
    {
      codec->rx_channel = current_ws;
      codec->rx_delay_pending = true;
      codec->rx_bit_count = 0;
      codec->rx_shift_reg = 0;
      return;
    }

    /* Handle the 1-BCLK delay slot */
    if(codec->rx_delay_pending)
    {
      codec->rx_delay_pending = false;
      return;
    }

    /* Sample active audio bit (MSB first) */
    if(codec->rx_bit_count < I2S_BITS_PER_SAMPLE)
    {
      codec->rx_shift_reg = (uint16_t)((codec->rx_shift_reg << 1) | (bus->sd_tx & 0x01));
      codec->rx_bit_count++;

      if(codec->rx_bit_count == I2S_BITS_PER_SAMPLE)
      {
        /* Complete 16-bit PCM word received */
        int16_t sample = (int16_t)codec->rx_shift_reg;
        if(codec->rx_channel == I2S_CHANNEL_LEFT)
        {
          codec->last_left_sample = sample;
        }
        else
        {
          codec->last_right_sample = sample;
          codec->total_frames_rx++;
        }
      }
    }
  }
  /* FALLING EDGE (1 -> 0): Codec sets up outgoing SD data for Master RX */
  else
  {
    uint8_t current_ws = bus->ws;

    if(codec->rx_delay_pending)
    {
      /* Prepare MSB during delay slot */
      if(current_ws == I2S_CHANNEL_LEFT)
      {
        codec->tx_shift_reg = (uint16_t)codec->tx_left_pattern;
      }
      else
      {
        codec->tx_shift_reg = (uint16_t)codec->tx_right_pattern;
      }
      codec->tx_bit_count = 0;
      bus->sd_rx = 0;
    }
    else if(codec->tx_bit_count < I2S_BITS_PER_SAMPLE)
    {
      bus->sd_rx = (uint8_t)((codec->tx_shift_reg >> (15 - codec->tx_bit_count)) & 0x01);
      codec->tx_bit_count++;
    }
    else
    {
      bus->sd_rx = 0;
    }
  }
}

/* ==============================================================================
 * Driver Function Declarations & Detailed Protocol Contracts
 * ============================================================================== */

/**
 * Initializes the I2S bus controller and places all physical bus lines into their idle states.
 *
 * Bus State at Idle:
 * - BCLK is driven LOW (0).
 * - WS is driven HIGH (1) or LOW (0) (standard default: HIGH for Right channel idle).
 * - SD is driven LOW (0).
 *
 * @param bus Pointer to the I2S bus structure.
 * @return 0 on success, or -1 on invalid argument (bus == NULL).
 */
int i2s_init(i2s_bus_t *bus)
{
  (void)bus;
  /* TODO: Author Implementation */
  return -1;
}

/**
 * Transmits a single 16-bit signed PCM audio sample on the specified audio channel
 * adhering strictly to the Philips I2S framing standard.
 *
 * Architectural Role:
 * Low-level channel serialization primitive. Transmits one 16-bit word on SD.
 *
 * Timing & Framing Invariants:
 * - Channel Select: WS = 0 for Left Channel, WS = 1 for Right Channel.
 * - WS transition occurs on the FALLING edge of BCLK.
 * - The 1-BCLK Period Delay: Exactly one BCLK clock cycle must elapse between the WS
 *   transition and the transmission of the MSB (bit 15).
 * - Bit Serialization: 16 data bits are shifted out MSB-first (bit 15 down to bit 0).
 * - Line Invariant: SD and WS transitions occur on BCLK FALLING edges, and remain stable
 *   during BCLK RISING edges so the receiver can sample reliably.
 * - After transmitting bit 0, BCLK must return to its idle state (LOW).
 *
 * @param bus Pointer to the I2S bus structure.
 * @param sample 16-bit signed two's complement PCM audio sample.
 * @param channel I2S_CHANNEL_LEFT (0) or I2S_CHANNEL_RIGHT (1).
 * @return 0 on success, or -1 on invalid argument (bus == NULL or invalid channel).
 */
int i2s_transmit_sample(i2s_bus_t *bus, int16_t sample, uint8_t channel)
{
  (void)bus;
  (void)sample;
  (void)channel;
  /* TODO: Author Implementation */
  return -1;
}

/**
 * Transmits one complete stereo audio frame (Left sample followed by Right sample).
 *
 * Architectural Role:
 * Stereo frame orchestrator. Combines Left channel transmission (WS = 0) followed by
 * Right channel transmission (WS = 1).
 *
 * Framing Invariants:
 * - Transmits Left sample first with WS = 0.
 * - Immediately follows with Right sample with WS = 1.
 * - Total frame clocking consists of 2 channel slots (each containing 1 delay cycle + 16 data
 * cycles).
 *
 * @param bus Pointer to the I2S bus structure.
 * @param left_sample 16-bit signed PCM sample for Left audio channel.
 * @param right_sample 16-bit signed PCM sample for Right audio channel.
 * @return 0 on success, or -1 on invalid argument (bus == NULL).
 */
int i2s_transmit_stereo_frame(i2s_bus_t *bus, int16_t left_sample, int16_t right_sample)
{
  (void)bus;
  (void)left_sample;
  (void)right_sample;
  /* TODO: Author Implementation */
  return -1;
}

/**
 * Transmits a contiguous buffer of stereo PCM audio frames.
 *
 * Architectural Role:
 * Audio stream buffer driver. Streams consecutive frames from memory buffers to the codec.
 *
 * Contract:
 * - Iterates frame_count times, calling the stereo frame transmission primitive for each pair
 *   (left_samples[i], right_samples[i]).
 *
 * @param bus Pointer to the I2S bus structure.
 * @param left_samples Pointer to array of Left channel 16-bit PCM samples.
 * @param right_samples Pointer to array of Right channel 16-bit PCM samples.
 * @param frame_count Number of stereo frames to transmit.
 * @return 0 on success, or -1 on invalid argument.
 */
int i2s_transmit_buffer(i2s_bus_t *bus, const int16_t *left_samples, const int16_t *right_samples,
                        size_t frame_count)
{
  (void)bus;
  (void)left_samples;
  (void)right_samples;
  (void)frame_count;
  /* TODO: Author Implementation */
  return -1;
}

/**
 * Receives one complete stereo audio frame from a peripheral (ADC) in master mode.
 *
 * Architectural Role:
 * Audio capture primitive. As the I2S bus master, generates BCLK and WS clocking
 * while sampling the Serial Data (SD) line driven by the connected ADC/codec.
 *
 * Contract:
 * - Clocks Left channel (WS = 0): waits 1-BCLK delay, samples 16 bits (MSB first) into
 * *left_sample.
 * - Clocks Right channel (WS = 1): waits 1-BCLK delay, samples 16 bits (MSB first) into
 * *right_sample.
 * - Data sampling occurs on the RISING edge of BCLK.
 *
 * @param bus Pointer to the I2S bus structure.
 * @param left_sample Destination pointer for captured 16-bit Left channel audio sample.
 * @param right_sample Destination pointer for captured 16-bit Right channel audio sample.
 * @return 0 on success, or -1 on invalid argument.
 */
int i2s_receive_stereo_frame(i2s_bus_t *bus, int16_t *left_sample, int16_t *right_sample)
{
  (void)bus;
  (void)left_sample;
  (void)right_sample;
  /* TODO: Author Implementation */
  return -1;
}

/* ==============================================================================
 * Comprehensive Verification Harness & Test Driver
 * ============================================================================== */

static void test_initialization_and_idle_levels(void)
{
  printf("[TEST 1] Verifying I2S initialization & line idle levels...\n");

  i2s_bus_t bus;
  memset(&bus, 0, sizeof(bus));

  assert(i2s_init(NULL) == -1);

  assert(i2s_init(&bus) == 0);
  assert(bus.bclk == 0);
  assert(bus.sd_tx == 0);

  printf("  -> PASS: Line idle levels verified (BCLK=0, SD=0).\n");
}

static void test_single_channel_transmission(void)
{
  printf("[TEST 2] Verifying single channel transmission & 1-BCLK delay compliance...\n");

  i2s_bus_t bus;
  assert(i2s_init(&bus) == 0);
  mock_codec_init(&bus.codec);

  int16_t left_sample = (int16_t)0x3A5C;
  assert(i2s_transmit_sample(&bus, left_sample, I2S_CHANNEL_LEFT) == 0);

  assert(bus.codec.last_left_sample == left_sample);

  int16_t right_sample = (int16_t)-12345;
  assert(i2s_transmit_sample(&bus, right_sample, I2S_CHANNEL_RIGHT) == 0);

  assert(bus.codec.last_right_sample == right_sample);

  printf("  -> PASS: Left (0x%04X) and Right (%d) sample transmission verified.\n",
         (uint16_t)left_sample, right_sample);
}

static void test_stereo_frame_transmission(void)
{
  printf("[TEST 3] Verifying complete stereo audio frame transmission...\n");

  i2s_bus_t bus;
  assert(i2s_init(&bus) == 0);
  mock_codec_init(&bus.codec);

  int16_t left = (int16_t)0x7FFF;  /* Maximum positive signed 16-bit (+32767) */
  int16_t right = (int16_t)-32768; /* Minimum negative signed 16-bit (-32768) */

  assert(i2s_transmit_stereo_frame(&bus, left, right) == 0);

  assert(bus.codec.last_left_sample == left);
  assert(bus.codec.last_right_sample == right);
  assert(bus.codec.total_frames_rx == 1);
  assert(bus.codec.sync_errors == 0);

  printf("  -> PASS: Stereo frame transmitted successfully (+32767, -32768).\n");
}

static void test_audio_buffer_streaming(void)
{
  printf("[TEST 4] Verifying multi-frame audio buffer streaming (sine wave simulation)...\n");

  i2s_bus_t bus;
  assert(i2s_init(&bus) == 0);
  mock_codec_init(&bus.codec);

  /* Synthesize 8 frames of stereo test PCM audio */
  int16_t left_buffer[8] = {0, 10000, 20000, 30000, 20000, 10000, 0, -10000};
  int16_t right_buffer[8] = {0, -10000, -20000, -30000, -20000, -10000, 0, 10000};

  assert(i2s_transmit_buffer(&bus, left_buffer, right_buffer, 8) == 0);

  assert(bus.codec.total_frames_rx == 8);
  assert(bus.codec.last_left_sample == left_buffer[7]);
  assert(bus.codec.last_right_sample == right_buffer[7]);
  assert(bus.codec.sync_errors == 0);

  printf("  -> PASS: 8 stereo audio frames streamed without synchronization errors.\n");
}

static void test_stereo_frame_reception(void)
{
  printf("[TEST 5] Verifying master stereo frame reception (ADC capture)...\n");

  i2s_bus_t bus;
  assert(i2s_init(&bus) == 0);
  mock_codec_init(&bus.codec);

  bus.codec.tx_left_pattern = (int16_t)0x4321;
  bus.codec.tx_right_pattern = (int16_t)-8765;

  int16_t rx_left = 0;
  int16_t rx_right = 0;

  assert(i2s_receive_stereo_frame(&bus, &rx_left, &rx_right) == 0);

  assert(rx_left == (int16_t)0x4321);
  assert(rx_right == (int16_t)-8765);

  printf("  -> PASS: Stereo frame captured from mock ADC (Left: 0x%04X, Right: %d).\n",
         (uint16_t)rx_left, rx_right);
}

int main(void)
{
  printf("==============================================================================\n");
  printf("Running I2S Bit-Banged Digital Audio Controller & Driver Test Suite\n");
  printf("==============================================================================\n");

  /* Probe for scaffold completion */
  i2s_bus_t probe_bus;
  memset(&probe_bus, 0, sizeof(probe_bus));
  if(i2s_init(&probe_bus) == -1)
  {
    printf("[Exercise Scaffold] i2s_init() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    return 0;
  }

  test_initialization_and_idle_levels();
  test_single_channel_transmission();
  test_stereo_frame_transmission();
  test_audio_buffer_streaming();
  test_stereo_frame_reception();

  printf("==============================================================================\n");
  printf("All I2S Driver Verification Tests Passed Successfully!\n");
  printf("==============================================================================\n");

  return 0;
}
