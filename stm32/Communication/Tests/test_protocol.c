#include "protocol.h"

#include <stdio.h>
#include <string.h>

#define MAX_CAPTURED_FRAMES 16U

#define CHECK(condition)                                                     \
    do                                                                       \
    {                                                                        \
        if (!(condition))                                                    \
        {                                                                    \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);      \
            g_failures++;                                                    \
        }                                                                    \
    } while (0)

static uint8_t g_frames[MAX_CAPTURED_FRAMES][PROTO_UART_MAX_DATA];
static uint8_t g_lengths[MAX_CAPTURED_FRAMES];
static uint16_t g_frame_count;
static int g_failures;

static void capture_frame(const uint8_t *data, uint8_t len)
{
    if (g_frame_count < MAX_CAPTURED_FRAMES)
    {
        memcpy(g_frames[g_frame_count], data, len);
        g_lengths[g_frame_count] = len;
    }
    g_frame_count++;
}

static void reset_capture(void)
{
    g_frame_count = 0U;
    memset(g_frames, 0, sizeof(g_frames));
    memset(g_lengths, 0, sizeof(g_lengths));
}

static void feed(Proto_UartParser *parser, const uint8_t *data, uint16_t len)
{
    uint16_t i;

    for (i = 0U; i < len; i++)
    {
        Proto_UartParserFeed(parser, data[i]);
    }
}

static void check_frame(uint16_t index, const uint8_t *expected, uint8_t len)
{
    CHECK(index < g_frame_count);
    if (index < g_frame_count)
    {
        CHECK(g_lengths[index] == len);
        CHECK(memcmp(g_frames[index], expected, len) == 0);
    }
}

static void test_crc_and_examples(void)
{
    static const uint8_t off_payload[] = {0x01U, 0x00U};
    static const uint8_t half_payload[] = {0x01U, 0x05U};
    static const uint8_t full_payload[] = {0x01U, 0x0AU};
    static const uint8_t voltage_payload[] =
        {0x02U, 0x00U, 0x08U, 0x72U, 0x06U};

    /* CRC is calculated over LEN + DATA, so the first byte here is LEN. */
    static const uint8_t off_crc_input[] = {0x02U, 0x01U, 0x00U};
    static const uint8_t half_crc_input[] = {0x02U, 0x01U, 0x05U};
    static const uint8_t full_crc_input[] = {0x02U, 0x01U, 0x0AU};
    static const uint8_t voltage_crc_input[] =
        {0x05U, 0x02U, 0x00U, 0x08U, 0x72U, 0x06U};

    static const uint8_t off_frame[] =
        {0xAAU, 0x55U, 0x02U, 0x01U, 0x00U, 0xC3U};
    static const uint8_t half_frame[] =
        {0xAAU, 0x55U, 0x02U, 0x01U, 0x05U, 0xD8U};
    static const uint8_t full_frame[] =
        {0xAAU, 0x55U, 0x02U, 0x01U, 0x0AU, 0xF5U};
    static const uint8_t voltage_frame[] =
        {0xAAU, 0x55U, 0x05U, 0x02U, 0x00U, 0x08U,
         0x72U, 0x06U, 0x82U};

    uint8_t out[PROTO_UART_MAX_FRAME];
    uint8_t total;

    CHECK(Proto_Crc8(off_crc_input, 3U) == 0xC3U);
    CHECK(Proto_Crc8(half_crc_input, 3U) == 0xD8U);
    CHECK(Proto_Crc8(full_crc_input, 3U) == 0xF5U);
    CHECK(Proto_Crc8(voltage_crc_input, 6U) == 0x82U);

    total = Proto_BuildUartFrame(off_payload, 2U, out);
    CHECK(total == sizeof(off_frame));
    CHECK(memcmp(out, off_frame, sizeof(off_frame)) == 0);

    total = Proto_BuildUartFrame(half_payload, 2U, out);
    CHECK(total == sizeof(half_frame));
    CHECK(memcmp(out, half_frame, sizeof(half_frame)) == 0);

    total = Proto_BuildUartFrame(full_payload, 2U, out);
    CHECK(total == sizeof(full_frame));
    CHECK(memcmp(out, full_frame, sizeof(full_frame)) == 0);

    total = Proto_BuildUartFrame(voltage_payload, 5U, out);
    CHECK(total == sizeof(voltage_frame));
    CHECK(memcmp(out, voltage_frame, sizeof(voltage_frame)) == 0);

    CHECK(Proto_BuildUartFrame(off_payload, 0U, out) == 0U);
    CHECK(Proto_BuildUartFrame(off_payload, 9U, out) == 0U);
}

static void test_payload_decoders(void)
{
    static const uint8_t brightness_0[] = {0x01U, 0x00U};
    static const uint8_t brightness_10[] = {0x01U, 0x0AU};
    static const uint8_t brightness_11[] = {0x01U, 0x0BU};
    static const uint8_t brightness_wrong_cmd[] = {0x02U, 0x05U};
    static const uint8_t brightness_wrong_len[] = {0x01U, 0x05U, 0x00U};
    static const uint8_t voltage[] = {0x02U, 0x00U, 0x08U, 0x72U, 0x06U};
    static const uint8_t voltage_bad_adc[] = {0x02U, 0x00U, 0x10U, 0x72U, 0x06U};

    uint8_t level;
    uint16_t adc;
    uint16_t mv;

    CHECK(Proto_DecodeBrightness(brightness_0, 2U, &level) == 1U);
    CHECK(level == 0U);
    CHECK(Proto_DecodeBrightness(brightness_10, 2U, &level) == 1U);
    CHECK(level == 10U);
    CHECK(Proto_DecodeBrightness(brightness_11, 2U, &level) == 0U);
    CHECK(Proto_DecodeBrightness(brightness_wrong_cmd, 2U, &level) == 0U);
    CHECK(Proto_DecodeBrightness(brightness_wrong_len, 3U, &level) == 0U);

    CHECK(Proto_DecodeVoltage(voltage, 5U, &adc, &mv) == 1U);
    CHECK(adc == 2048U);
    CHECK(mv == 1650U);
    CHECK(Proto_DecodeVoltage(voltage_bad_adc, 5U, &adc, &mv) == 0U);

    CHECK(Proto_AdcToMillivolts(0U) == 0U);
    CHECK(Proto_AdcToMillivolts(1U) == 1U);
    CHECK(Proto_AdcToMillivolts(2048U) == 1650U);
    CHECK(Proto_AdcToMillivolts(4095U) == 3300U);
    CHECK(Proto_AdcToMillivolts(5000U) == 3300U);
}

static void test_all_brightness_levels(void)
{
    uint8_t level;
    uint8_t payload[2];
    uint8_t frame[PROTO_UART_MAX_FRAME];
    uint8_t total;
    Proto_UartParser parser;
    uint16_t i;

    for (i = 0U; i <= PROTO_BRIGHTNESS_MAX; i++)
    {
        payload[0] = PROTO_CMD_BRIGHTNESS;
        payload[1] = (uint8_t)i;
        total = Proto_BuildUartFrame(payload, 2U, frame);
        CHECK(total == 6U);

        Proto_UartParserInit(&parser, capture_frame);
        reset_capture();
        feed(&parser, frame, total);
        CHECK(g_frame_count == 1U);
        CHECK(Proto_DecodeBrightness(g_frames[0], g_lengths[0], &level) == 1U);
        CHECK(level == (uint8_t)i);
    }
}

static void test_crc_error_recovery(void)
{
    static const uint8_t frame[] =
        {0xAAU, 0x55U, 0x02U, 0x01U, 0x05U, 0xD8U};
    uint8_t corrupted[sizeof(frame)];
    Proto_UartParser parser;
    uint16_t position;

    for (position = 2U; position < sizeof(frame); position++)
    {
        memcpy(corrupted, frame, sizeof(frame));
        corrupted[position] ^= 0xFFU;

        Proto_UartParserInit(&parser, capture_frame);
        reset_capture();
        feed(&parser, corrupted, sizeof(corrupted));
        CHECK(g_frame_count == 0U);

        feed(&parser, frame, sizeof(frame));
        CHECK(g_frame_count == 1U);
        CHECK(g_lengths[0] == 2U);
        CHECK(g_frames[0][0] == 0x01U);
        CHECK(g_frames[0][1] == 0x05U);
    }
}

static void test_all_adc_values(void)
{
    uint8_t payload[5];
    uint16_t adc;
    uint16_t mv;
    uint16_t decoded_adc;
    uint16_t decoded_mv;

    for (adc = 0U; adc <= 4095U; adc++)
    {
        mv = Proto_AdcToMillivolts(adc);
        CHECK(mv <= 3300U);
        CHECK(Proto_EncodeVoltage(adc, mv, payload) == 5U);
        CHECK(Proto_DecodeVoltage(payload, 5U, &decoded_adc, &decoded_mv) == 1U);
        CHECK(decoded_adc == adc);
        CHECK(decoded_mv == mv);
    }
}

static void test_parser_full_and_split(void)
{
    static const uint8_t frame[] =
        {0xAAU, 0x55U, 0x02U, 0x01U, 0x05U, 0xD8U};
    static const uint8_t expected[] = {0x01U, 0x05U};
    Proto_UartParser parser;
    uint16_t i;

    Proto_UartParserInit(&parser, capture_frame);
    reset_capture();
    feed(&parser, frame, sizeof(frame));
    CHECK(g_frame_count == 1U);
    check_frame(0U, expected, 2U);

    Proto_UartParserInit(&parser, capture_frame);
    reset_capture();
    for (i = 0U; i < sizeof(frame); i++)
    {
        Proto_UartParserFeed(&parser, frame[i]);
        if (i + 1U < sizeof(frame))
        {
            CHECK(g_frame_count == 0U);
        }
    }
    CHECK(g_frame_count == 1U);
    check_frame(0U, expected, 2U);
}

static void test_parser_multiple_frames(void)
{
    static const uint8_t stream[] =
    {
        0xAAU, 0x55U, 0x02U, 0x01U, 0x00U, 0xC3U,
        0xAAU, 0x55U, 0x02U, 0x01U, 0x05U, 0xD8U,
        0xAAU, 0x55U, 0x02U, 0x01U, 0x0AU, 0xF5U
    };
    static const uint8_t expected_0[] = {0x01U, 0x00U};
    static const uint8_t expected_5[] = {0x01U, 0x05U};
    static const uint8_t expected_10[] = {0x01U, 0x0AU};
    Proto_UartParser parser;

    Proto_UartParserInit(&parser, capture_frame);
    reset_capture();
    feed(&parser, stream, sizeof(stream));
    CHECK(g_frame_count == 3U);
    check_frame(0U, expected_0, 2U);
    check_frame(1U, expected_5, 2U);
    check_frame(2U, expected_10, 2U);
}

static void test_parser_resync(void)
{
    static const uint8_t bad_crc_then_good[] =
    {
        0xAAU, 0x55U, 0x02U, 0x01U, 0x05U, 0x00U,
        0xAAU, 0x55U, 0x02U, 0x01U, 0x0AU, 0xF5U
    };
    static const uint8_t truncated_then_good[] =
    {
        0xAAU, 0x55U, 0x02U, 0x01U,
        0xAAU, 0x55U, 0x02U, 0x01U, 0x0AU, 0xF5U
    };
    static const uint8_t bad_len_then_good[] =
    {
        0x00U, 0xFFU, 0xAAU, 0x55U, 0x09U, 0x11U, 0x22U,
        0xAAU, 0x55U, 0x02U, 0x01U, 0x0AU, 0xF5U
    };
    static const uint8_t aa_aa_55[] =
    {
        0xAAU, 0xAAU, 0x55U, 0x02U, 0x01U, 0x05U, 0xD8U
    };
    static const uint8_t expected_10[] = {0x01U, 0x0AU};
    static const uint8_t expected_5[] = {0x01U, 0x05U};
    Proto_UartParser parser;

    Proto_UartParserInit(&parser, capture_frame);
    reset_capture();
    feed(&parser, bad_crc_then_good, sizeof(bad_crc_then_good));
    CHECK(g_frame_count == 1U);
    check_frame(0U, expected_10, 2U);

    Proto_UartParserInit(&parser, capture_frame);
    reset_capture();
    feed(&parser, truncated_then_good, sizeof(truncated_then_good));
    CHECK(g_frame_count == 1U);
    check_frame(0U, expected_10, 2U);

    Proto_UartParserInit(&parser, capture_frame);
    reset_capture();
    feed(&parser, bad_len_then_good, sizeof(bad_len_then_good));
    CHECK(g_frame_count == 1U);
    check_frame(0U, expected_10, 2U);

    Proto_UartParserInit(&parser, capture_frame);
    reset_capture();
    feed(&parser, aa_aa_55, sizeof(aa_aa_55));
    CHECK(g_frame_count == 1U);
    check_frame(0U, expected_5, 2U);
}

static void test_embedded_header_and_generic_length(void)
{
    static const uint8_t data[] =
        {0x42U, 0xAAU, 0x55U, 0x99U, 0x00U, 0xFFU, 0x12U, 0x34U};
    uint8_t frame[PROTO_UART_MAX_FRAME];
    uint8_t total;
    Proto_UartParser parser;

    total = Proto_BuildUartFrame(data, sizeof(data), frame);
    CHECK(total == 12U);

    Proto_UartParserInit(&parser, capture_frame);
    reset_capture();
    feed(&parser, frame, total);
    CHECK(g_frame_count == 1U);
    check_frame(0U, data, sizeof(data));
}

static void test_end_to_end_flow(void)
{
    uint8_t level;
    uint8_t duty;
    uint8_t pc_frame[PROTO_UART_MAX_FRAME];
    uint8_t voltage_payload[5];
    uint8_t response_frame[PROTO_UART_MAX_FRAME];
    uint8_t pc_data[2];
    uint16_t adc;
    uint16_t mv;
    uint16_t i;
    uint8_t total;
    Proto_UartParser parser;

    pc_data[0] = PROTO_CMD_BRIGHTNESS;
    pc_data[1] = 7U;
    total = Proto_BuildUartFrame(pc_data, 2U, pc_frame);
    CHECK(total == 6U);

    Proto_UartParserInit(&parser, capture_frame);
    reset_capture();
    for (i = 0U; i < total; i++)
    {
        Proto_UartParserFeed(&parser, pc_frame[i]);
    }
    CHECK(g_frame_count == 1U);
    CHECK(Proto_DecodeBrightness(g_frames[0], g_lengths[0], &level) == 1U);
    CHECK(level == 7U);

    duty = (uint8_t)(level * 10U);
    CHECK(duty == 70U);

    adc = 2048U;
    mv = Proto_AdcToMillivolts(adc);
    CHECK(Proto_EncodeVoltage(adc, mv, voltage_payload) == 5U);
    CHECK(voltage_payload[0] == 0x02U);
    CHECK(voltage_payload[1] == 0x00U);
    CHECK(voltage_payload[2] == 0x08U);
    CHECK(voltage_payload[3] == 0x72U);
    CHECK(voltage_payload[4] == 0x06U);

    total = Proto_BuildUartFrame(voltage_payload, 5U, response_frame);
    CHECK(total == 9U);

    Proto_UartParserInit(&parser, capture_frame);
    reset_capture();
    for (i = 0U; i < total; i++)
    {
        Proto_UartParserFeed(&parser, response_frame[i]);
    }
    CHECK(g_frame_count == 1U);
    CHECK(Proto_DecodeVoltage(g_frames[0], g_lengths[0], &adc, &mv) == 1U);
    CHECK(adc == 2048U);
    CHECK(mv == 1650U);
}

int main(void)
{
    test_crc_and_examples();
    test_payload_decoders();
    test_all_brightness_levels();
    test_crc_error_recovery();
    test_all_adc_values();
    test_parser_full_and_split();
    test_parser_multiple_frames();
    test_parser_resync();
    test_embedded_header_and_generic_length();
    test_end_to_end_flow();

    if (g_failures == 0)
    {
        printf("PASS: all protocol tests passed.\n");
        return 0;
    }

    printf("FAILED: %d assertion(s).\n", g_failures);
    return 1;
}
