// Copyright 2020 Silicon Laboratories, Inc.
//
//

/***************************************************************************//**
 * @brief RAIL Configuration
 * @details
 *   WARNING: Auto-Generated Radio Config  -  DO NOT EDIT
 *   Radio Configurator Version: 5.0.1
 *   RAIL Adapter Version: 2.4.10
 *   RAIL Compatibility: 2.x
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "em_common.h"
#include "rail_config.h"

uint32_t RAILCb_CalcSymbolRate(RAIL_Handle_t railHandle)
{
  (void) railHandle;
  return 0U;
}

uint32_t RAILCb_CalcBitRate(RAIL_Handle_t railHandle)
{
  (void) railHandle;
  return 0U;
}

void RAILCb_ConfigFrameTypeLength(RAIL_Handle_t railHandle,
                                  const RAIL_FrameType_t *frameType)
{
  (void) railHandle;
  (void) frameType;
}

static const uint8_t generated_irCalConfig[] = {
  25, 63, 1, 6, 4, 16, 1, 0, 0, 1, 1, 6, 0, 16, 39, 0, 0, 5, 0, 1, 1, 0, 0, 0, 0, 0
};

static const uint32_t generated_phyInfo[] = {
  5UL,
  0x0001B4E8UL, // 1.70666666667
  (uint32_t) NULL,
  (uint32_t) generated_irCalConfig,
#ifdef RADIO_CONFIG_ENABLE_TIMING
  (uint32_t) &generated_timing,
#else
  (uint32_t) NULL,
#endif
  0x00000000UL,
  10160000UL,
  38400000UL,
  16000UL,
  (1UL << 8) | 1UL,
  0x02001000UL,
};

const uint32_t generated[] = {
  0x01040FF0UL, (uint32_t) &generated_phyInfo,
     /* 0FF4 */ 0x00000000UL,
     /* 0FF8 */ 0x0003C002UL,
     /* 0FFC */ 0x0003C000UL,
  0x00020004UL, 0x00000000UL,
     /* 0008 */ 0x00000000UL,
  0x00020018UL, 0x0000000FUL,
     /* 001C */ 0x00000000UL,
  0x00070028UL, 0x00000000UL,
     /* 002C */ 0x00000000UL,
     /* 0030 */ 0x00000000UL,
     /* 0034 */ 0x00000000UL,
     /* 0038 */ 0x00000000UL,
     /* 003C */ 0x00000000UL,
     /* 0040 */ 0x00000700UL,
  0x00010048UL, 0x00000000UL,
  0x00020054UL, 0x00000000UL,
     /* 0058 */ 0x00000000UL,
  0x000400A0UL, 0x000040FFUL,
     /* 00A4 */ 0x00000000UL,
     /* 00A8 */ 0x000041FFUL,
     /* 00AC */ 0x00000000UL,
  0x00012000UL, 0x00000700UL,
  0x00012010UL, 0x00000000UL,
  0x00012018UL, 0x00000000UL,
  0x00013008UL, 0x0000AC3FUL,
  0x00023030UL, 0x00101000UL,
     /* 3034 */ 0x00000001UL,
  0x00013040UL, 0x00000000UL,
  0x000140A0UL, 0x0F00277AUL,
  0x000140F4UL, 0x00001020UL,
  0x00024134UL, 0x00000880UL,
     /* 4138 */ 0x000087E6UL,
  0x00024140UL, 0x00880060UL,
     /* 4144 */ 0x1153E6C1UL,
  0x00156014UL, 0x00000010UL,
     /* 6018 */ 0x04000000UL,
     /* 601C */ 0x00020001UL,
     /* 6020 */ 0x00001000UL,
     /* 6024 */ 0x000AD000UL,
     /* 6028 */ 0x03000000UL,
     /* 602C */ 0x00000000UL,
     /* 6030 */ 0x006D7FBCUL,
     /* 6034 */ 0x00000441UL,
     /* 6038 */ 0x0200063AUL,
     /* 603C */ 0x00010012UL,
     /* 6040 */ 0x00000002UL,
     /* 6044 */ 0x00000000UL,
     /* 6048 */ 0x046C0003UL,
     /* 604C */ 0x00000000UL,
     /* 6050 */ 0x00340315UL,
     /* 6054 */ 0x00000000UL,
     /* 6058 */ 0x00000000UL,
     /* 605C */ 0x22140A04UL,
     /* 6060 */ 0x504B4133UL,
     /* 6064 */ 0x00000000UL,
  0x00037014UL, 0x000270FEUL,
     /* 7018 */ 0x00001300UL,
     /* 701C */ 0x81710060UL,
  0x00017028UL, 0x01800000UL,
  0x00027048UL, 0x00003D3CUL,
     /* 704C */ 0x000019BCUL,
  0x00037070UL, 0x00020103UL,
     /* 7074 */ 0x00000112UL,
     /* 7078 */ 0x00552300UL,

  0xFFFFFFFFUL,
};

RAIL_ChannelConfigEntryAttr_t generated_entryAttr = {
  { 0xFFFFFFFFUL }
};

const RAIL_ChannelConfigEntry_t generated_channels[] = {
  {
    .phyConfigDeltaAdd = NULL,
    .baseFrequency = 2400000000,
    .channelSpacing = 100,
    .physicalChannelOffset = 0,
    .channelNumberStart = 0,
    .channelNumberEnd = 20,
    .maxPower = RAIL_TX_POWER_MAX,
    .attr = &generated_entryAttr
  },
};

const RAIL_ChannelConfig_t generated_channelConfig = {
  generated,
  NULL,
  generated_channels,
  1
};
const RAIL_ChannelConfig_t *channelConfigs[] = {
  &generated_channelConfig,
};


//        _  _                          
//       | )/ )         Wireless        
//    \\ |//,' __       Application     
//    (")(_)-"()))=-    Software        
//       (\\            Platform        
