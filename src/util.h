#pragma once

#define COMBINE_UINT8_2(hi, lo) (((uint16_t)hi) << 8) | ((uint16_t)(lo))
#define COMBINE_UINT8_3(hi, mi, lo)                                                                \
    (((uint32_t)hi) << 16) | ((uint32_t)(mi) << 8) | ((uint32_t)(lo))
