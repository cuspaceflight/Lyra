#pragma once

#define COMBINE_UINT8_2(a, b) ((uint16_t)(a) << 8) | ((uint16_t)(b))
#define COMBINE_UINT8_3(a, b, c) ((uint32_t)(a) << 16) | ((uint32_t)(b) << 8) | ((uint32_t)(c))
