/*************************************************************************************
 *
 *
 *************************************************************************************/

#include "Code128_Encode.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>


#define CODE128_QUIET_ZONE_LEN 10
#define CODE128_CHAR_LEN       11
#define CODE128_STOP_CODE_LEN  13

#define CODE128_START_CODE_A 103
#define CODE128_START_CODE_B 104
#define CODE128_START_CODE_C 105

#define CODE128_MODE_A    'a'
#define CODE128_MODE_B    'b'
#define CODE128_MODE_C    'c'
#define CODE128_MODE_X    'x'

#define CODE128_MIN_ENCODE_LEN (CODE128_QUIET_ZONE_LEN * 2 + CODE128_CHAR_LEN * 2 + CODE128_STOP_CODE_LEN)


//==============================================================================

static const int code128_pattern[] = {
          // value: pattern,   bar/space widths
    1740, //   0: 11011001100, 212222
    1644, //   1: 11001101100, 222122
    1638, //   2: 11001100110, 222221
    1176, //   3: 10010011000, 121223
    1164, //   4: 10010001100, 121322
    1100, //   5: 10001001100, 131222
    1224, //   6: 10011001000, 122213
    1220, //   7: 10011000100, 122312
    1124, //   8: 10001100100, 132212
    1608, //   9: 11001001000, 221213
    1604, //  10: 11001000100, 221312
    1572, //  11: 11000100100, 231212
    1436, //  12: 10110011100, 112232
    1244, //  13: 10011011100, 122132
    1230, //  14: 10011001110, 122231
    1484, //  15: 10111001100, 113222
    1260, //  16: 10011101100, 123122
    1254, //  17: 10011100110, 123221
    1650, //  18: 11001110010, 223211
    1628, //  19: 11001011100, 221132
    1614, //  20: 11001001110, 221231
    1764, //  21: 11011100100, 213212
    1652, //  22: 11001110100, 223112
    1902, //  23: 11101101110, 312131
    1868, //  24: 11101001100, 311222
    1836, //  25: 11100101100, 321122
    1830, //  26: 11100100110, 321221
    1892, //  27: 11101100100, 312212
    1844, //  28: 11100110100, 322112
    1842, //  29: 11100110010, 322211
    1752, //  30: 11011011000, 212123
    1734, //  31: 11011000110, 212321
    1590, //  32: 11000110110, 232121
    1304, //  33: 10100011000, 111323
    1112, //  34: 10001011000, 131123
    1094, //  35: 10001000110, 131321
    1416, //  36: 10110001000, 112313
    1128, //  37: 10001101000, 132113
    1122, //  38: 10001100010, 132311
    1672, //  39: 11010001000, 211313
    1576, //  40: 11000101000, 231113
    1570, //  41: 11000100010, 231311
    1464, //  42: 10110111000, 112133
    1422, //  43: 10110001110
    1134, //  44: 10001101110
    1496, //  45: 10111011000, 113123
    1478, //  46: 10111000110, 113321
    1142, //  47: 10001110110, 133121
    1910, //  48: 11101110110, 313121
    1678, //  49: 11010001110, 211331
    1582, //  50: 11000101110, 231131
    1768, //  51: 11011101000, 213113
    1762, //  52: 11011100010, 213311
    1774, //  53: 11011101110, 213131
    1880, //  54: 11101011000, 311123
    1862, //  55: 11101000110, 311321
    1814, //  56: 11100010110, 331121
    1896, //  57: 11101101000, 312113
    1890, //  58: 11101100010, 312311
    1818, //  59: 11100011010, 332111
    1914, //  60: 11101111010, 314111
    1602, //  61: 11001000010, 221411
    1930, //  62: 11110001010, 431111
    1328, //  63: 10100110000, 111224
    1292, //  64: 10100001100, 111422
    1200, //  65: 10010110000, 121124
    1158, //  66: 10010000110, 121421
    1068, //  67: 10000101100, 141122
    1062, //  68: 10000100110, 141221
    1424, //  69: 10110010000, 112214
    1412, //  70: 10110000100, 112412
    1232, //  71: 10011010000, 122114
    1218, //  72: 10011000010, 122411
    1076, //  73: 10000110100, 142112
    1074, //  74: 10000110010, 142211
    1554, //  75: 11000010010, 241211
    1616, //  76: 11001010000, 221114
    1978, //  77: 11110111010, 413111
    1556, //  78: 11000010100, 241112
    1146, //  79: 10001111010, 134111
    1340, //  80: 10100111100, 111242
    1212, //  81: 10010111100, 121142
    1182, //  82: 10010011110, 121241
    1508, //  83: 10111100100, 114212
    1268, //  84: 10011110100, 124112
    1266, //  85: 10011110010, 124211
    1956, //  86: 11110100100, 411212
    1940, //  87: 11110010100, 421112
    1938, //  88: 11110010010, 421211
    1758, //  89: 11011011110, 212141
    1782, //  90: 11011110110, 214121
    1974, //  91: 11110110110, 412121
    1400, //  92: 10101111000, 111143
    1310, //  93: 10100011110, 111341
    1118, //  94: 10001011110, 131141
    1512, //  95: 10111101000, 114113
    1506, //  96: 10111100010, 114311
    1960, //  97: 11110101000, 411113
    1954, //  98: 11110100010, 411311
    1502, //  99: 10111011110, 113141
    1518, // 100: 10111101110, 114131
    1886, // 101: 11101011110, 311141
    1966, // 102: 11110101110, 411131
    1668, // 103: 11010000100, 211412
    1680, // 104: 11010010000, 211214
    1692  // 105: 11010011100, 211232
};

const int code128_stop_pattern = 6379; // 1100011101011, 2331112

struct code128_step
{
    int prev_ix;                // Index of previous step, if any
    const char *next_input;     // Remaining input
    unsigned short len;         // The length of the pattern so far (includes this step)
    char mode;                  // State for the current encoding
    signed char code;           // What code should be written for this step
};

struct code128_state {
    struct code128_step *steps;
    int allocated_steps;
    int current_ix;
    int todo_ix;
    int best_ix;

    size_t maxlength;
};

char Prev_Mode = CODE128_MODE_X;    // means no valid mode


//==============================================================================

size_t code128_estimate_len(const char *s)
{
    return CODE128_QUIET_ZONE_LEN
           + CODE128_CHAR_LEN // start code
           + CODE128_CHAR_LEN * (strlen(s) * 11 / 10) // contents + 10% padding
           + CODE128_CHAR_LEN // checksum
           + CODE128_STOP_CODE_LEN
           + CODE128_QUIET_ZONE_LEN;
}

static void code128_append_pattern(int pattern, int pattern_length, char *out)
{
    // All patterns have their first bit set by design
    assert(pattern & (1 << (pattern_length - 1)));

    int i;
    for (i = pattern_length - 1; i >= 0; i--) {
        // cast avoids warning: implicit conversion from 'int' to 'char' changes value from 255 to -1 [-Wconstant-conversion]
        *out++ = (unsigned char)((pattern & (1 << i)) ? 255 : 0);
    }
}

static int code128_append_code(int code, char *out)
{
    assert(code >= 0 && code < (int) (sizeof(code128_pattern) / sizeof(code128_pattern[0])));
    code128_append_pattern(code128_pattern[code], CODE128_CHAR_LEN, out);
    return CODE128_CHAR_LEN;
}

static int code128_append_stop_code(char *out)
{
    code128_append_pattern(code128_stop_pattern, CODE128_STOP_CODE_LEN, out);
    return CODE128_STOP_CODE_LEN;
}

static signed char code128_switch_code(char from_mode, char to_mode)
{
    switch (from_mode) {
    case CODE128_MODE_A:
        switch (to_mode) {
        case CODE128_MODE_B:
            return 100;
        case CODE128_MODE_C:
            return 99;
        }

    case CODE128_MODE_B:
        switch (to_mode) {
        case CODE128_MODE_A:
            return 101;
        case CODE128_MODE_C:
            return 99;
        }

    case CODE128_MODE_C:
        switch (to_mode) {
        case CODE128_MODE_B:
            return 100;
        case CODE128_MODE_A:
            return 101;
        }

    case CODE128_MODE_X:
        switch (to_mode) {
        case CODE128_MODE_A:
            return CODE128_START_CODE_A;
        case CODE128_MODE_B:
            return CODE128_START_CODE_B;
        case CODE128_MODE_C:
            return CODE128_START_CODE_C;
        }
    }

    assert(0); // Invalid mode switch
    return -1;
}

static signed char code128a_ascii_to_code(char value)
{
    if (value >= ' ' && value <= '_')
        return value - ' ';
    else if (value >= 0 && value < ' ')
        return value + 64;
    else if (value == CODE128_FNC1)
        return 102;
    else if (value == CODE128_FNC2)
        return 97;
    else if (value == CODE128_FNC3)
        return 96;
    else if (value == CODE128_FNC4)
        return 101;
    else
        return -1;
}

static signed char code128b_ascii_to_code(char value)
{
    if (value >= 32) // value <= 127 is implied
        return value - 32;
    else if (value == CODE128_FNC1)
        return 102;
    else if (value == CODE128_FNC2)
        return 97;
    else if (value == CODE128_FNC3)
        return 96;
    else if (value == CODE128_FNC4)
        return 100;
    else
        return -1;
}

static signed char code128c_ascii_to_code(const char *values)
{
    if (values[0] == CODE128_FNC1)
        return 102;

    if (values[0] >= '0' && values[0] <= '9' &&
            values[1] >= '0' && values[1] <= '9') {
        char code = 10 * (values[0] - '0') + (values[1] - '0');
        return code;
    }

    return -1;
}

static int code128_do_a_step(struct code128_step *base, int prev_ix, int ix)
{
    struct code128_step *previous_step = &base[prev_ix];
    struct code128_step *step = &base[ix];

    char value = *previous_step->next_input;
    // NOTE: Currently we can't encode NULL
    if (value == 0)
        return 0;

    step->code = code128a_ascii_to_code(value);
    if (step->code < 0)
        return 0;

    step->prev_ix = prev_ix;
    step->next_input = previous_step->next_input + 1;
    step->mode = CODE128_MODE_A;
    step->len = previous_step->len + CODE128_CHAR_LEN;
    if (step->mode != previous_step->mode)
        step->len += CODE128_CHAR_LEN; // Need to switch modes

    return 1;
}

static int code128_do_b_step(struct code128_step *base, int prev_ix, int ix)
{
    struct code128_step *previous_step = &base[prev_ix];
    struct code128_step *step = &base[ix];

    char value = *previous_step->next_input;
    // NOTE: Currently we can't encode NULL
    if (value == 0)
        return 0;

    step->code = code128b_ascii_to_code(value);
    if (step->code < 0)
        return 0;

    step->prev_ix = prev_ix;
    step->next_input = previous_step->next_input + 1;
    step->mode = CODE128_MODE_B;
    step->len = previous_step->len + CODE128_CHAR_LEN;
    if (step->mode != previous_step->mode)
        step->len += CODE128_CHAR_LEN; // Need to switch modes

    return 1;
}

static int code128_do_c_step(struct code128_step *base, int prev_ix, int ix)
{
    struct code128_step *previous_step = &base[prev_ix];
    struct code128_step *step = &base[ix];

    char value = *previous_step->next_input;
    // NOTE: Currently we can't encode NULL
    if (value == 0)
        return 0;

    step->code = code128c_ascii_to_code(previous_step->next_input);
    if (step->code < 0)
        return 0;

    step->prev_ix = prev_ix;
    step->next_input = previous_step->next_input + 1;

    // Mode C consumes 2 characters for codes 0-99
    if (step->code < 100)
        step->next_input++;

    step->mode = CODE128_MODE_C;
    step->len = previous_step->len + CODE128_CHAR_LEN;
    if (step->mode != previous_step->mode)
        step->len += CODE128_CHAR_LEN; // Need to switch modes

    return 1;
}

static struct code128_step *code128_alloc_step(struct code128_state *state)
{
    if (state->todo_ix >= state->allocated_steps) {
        state->allocated_steps += 1024;
        state->steps = (struct code128_step *) realloc(state->steps, state->allocated_steps * sizeof(struct code128_step));
    }

    struct code128_step *step = &state->steps[state->todo_ix];

    memset(step, 0, sizeof(*step));
    return step;
}

static void code128_do_step(struct code128_state *state)
{
    struct code128_step *step = &state->steps[state->current_ix];
    if (*step->next_input == 0) {
        // Done, so see if we have a new shortest encoding.
        if ((step->len < state->maxlength) ||
                (state->best_ix < 0 && step->len == state->maxlength)) {
            state->best_ix = state->current_ix;

            // Update maxlength to avoid considering anything longer
            state->maxlength = step->len;
        }
        return;
    }

    // Don't try if we're already at or beyond the max acceptable
    // length;
    if (step->len >= state->maxlength)
        return;
    char mode = step->mode;

    code128_alloc_step(state);
    int mode_c_worked = 0;

    // Always try mode C
    if (code128_do_c_step(state->steps, state->current_ix, state->todo_ix)) {
        state->todo_ix++;
        code128_alloc_step(state);
        mode_c_worked = 1;
    }

    if (mode == CODE128_MODE_A) {
        // If A works, stick with A. There's no advantage to switching
        // to B proactively if A still works.
        if (code128_do_a_step(state->steps, state->current_ix, state->todo_ix) ||
                code128_do_b_step(state->steps, state->current_ix, state->todo_ix))
            state->todo_ix++;
    } else if (mode == CODE128_MODE_B) {
        // The same logic applies here. There's no advantage to switching
        // proactively to A if B still works.
        if (code128_do_b_step(state->steps, state->current_ix, state->todo_ix) ||
                code128_do_a_step(state->steps, state->current_ix, state->todo_ix))
            state->todo_ix++;
    } else if (!mode_c_worked) {
        // In mode C. If mode C worked and we're in mode C, trying anything
        // else is pointless since the mode C encoding will be shorter and
        // there won't be any mode switches.

        // If we're leaving mode C, though, try both in case one ends up
        // better than the other.
        if (code128_do_a_step(state->steps, state->current_ix, state->todo_ix)) {
            state->todo_ix++;
            code128_alloc_step(state);
        }
        if (code128_do_b_step(state->steps, state->current_ix, state->todo_ix))
            state->todo_ix++;
    }
}

size_t code128_encode_raw(const char *s, char *out, size_t maxlength)
{
    unsigned int i, sum, encoded_len = 0;
    signed char encoded_code;
    char codes[MAX_CODE_NUM];

    if(maxlength > MAX_CODE_NUM)
        return 0;

    Prev_Mode = CODE128_MODE_X;    // means no valid mode

    while((*s != 0) && (encoded_len < maxlength))  // leave space for code
    {
        // Try mode C first
        encoded_code = code128c_ascii_to_code(s);

        if(0)//(encoded_code >= 0)  // for compatible with UHF_EPD_Demo barcode format
        {
            if(Prev_Mode != CODE128_MODE_C)
            {
                codes[encoded_len++] = code128_switch_code(Prev_Mode, CODE128_MODE_C);

                Prev_Mode = CODE128_MODE_C;
            }

            codes[encoded_len++] = encoded_code;

            if(encoded_code < 100)
                s += 2;
            else
                s += 1;
        }
        else if(Prev_Mode == CODE128_MODE_A)
        {
            // Stay with mode A
            encoded_code = code128a_ascii_to_code(*s);

            if(encoded_code >= 0)
            {
                codes[encoded_len++] = encoded_code;

                s += 1;
            }
            else
            {
                // Switch to mode B
                encoded_code = code128b_ascii_to_code(*s);

                if(encoded_code >= 0)
                {
                    codes[encoded_len++] = code128_switch_code(Prev_Mode, CODE128_MODE_B);
                    codes[encoded_len++] = encoded_code;

                    Prev_Mode = CODE128_MODE_B;

                    s += 1;
                }
                else
                {
                    // Skip encoding
                    s += 1;
                }
            }
        }
        else if((Prev_Mode == CODE128_MODE_B) || (Prev_Mode == CODE128_MODE_C) || (Prev_Mode == CODE128_MODE_X))
        {
            // Stay with mode B or Switch to mode B first from mode C
            encoded_code = code128b_ascii_to_code(*s);

            if(encoded_code >= 0)
            {
                if(Prev_Mode != CODE128_MODE_B)
                {
                    codes[encoded_len++] = code128_switch_code(Prev_Mode, CODE128_MODE_B);

                    Prev_Mode = CODE128_MODE_B;
                }

                codes[encoded_len++] = encoded_code;

                s += 1;
            }
            else
            {
                // Switch to mode A
                encoded_code = code128a_ascii_to_code(*s);

                if(encoded_code >= 0)
                {
                    codes[encoded_len++] = code128_switch_code(Prev_Mode, CODE128_MODE_A);
                    codes[encoded_len++] = encoded_code;

                    Prev_Mode = CODE128_MODE_A;

                    s += 1;
                }
                else
                {
                    // Skip encoding
                    s += 1;
                }
            }
        }
    }

    if(encoded_len > 0)
    {
        // Fill in 0xFF or 0x00 based on each bit in codes
        for(i = 0; i < encoded_len; i++)
            out += code128_append_code(codes[i], out);

        // Compute the checksum
        sum = codes[0];

        for(i = 1; i < encoded_len; i++)
            sum += codes[i] * i;

        out += code128_append_code(sum % 103, out);
    }

    return encoded_len;     // without Checksum code, STOP code and QUIET zone
}

/**
 * @brief Encode the GS1 string
 *
 * This converts [FNC1] sequences to raw FNC1 characters and
 * removes spaces before encoding the barcodes.
 *
 * @return the length of barcode data in bytes
 */
size_t code128_encode_gs1(const char *s, char *out, size_t maxlength)
{
    char raw[strlen(s) + 1];

    char *p = raw;
    for (; *s != '\0'; s++) {
        if (strncmp(s, "[FNC1]", 6) == 0) {
            *p++ = CODE128_FNC1;
            s += 5;
        } else if (*s != ' ') {
            *p++ = *s;
        }
    }
    *p = '\0';

    return code128_encode_raw(raw, out, maxlength);
}
