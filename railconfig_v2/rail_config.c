// Copyright 2020 Silicon Laboratories, Inc.
//
//

/***************************************************************************//**
 * @brief RAIL Configuration
 * @details
 *   WARNING: Auto-Generated Radio Config  -  DO NOT EDIT
 *   Radio Configurator Version: 5.6.0
 *   RAIL Adapter Version: 2.4.13
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
#include "em_device.h"
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
  25, 68, 6, 6, 4, 16, 0, 1, 1, 2, 1, 6, 0, 16, 39, 0, 0, 5, 0, 1, 1, 0, 0, 0, 0, 0
};

static const uint32_t generated_phyInfo[] = {
  5UL,
  0x000A3D70UL, // 10.24
  (uint32_t) NULL,
  (uint32_t) generated_irCalConfig,
#ifdef RADIO_CONFIG_ENABLE_TIMING
  (uint32_t) &generated_timing,
#else
  (uint32_t) NULL,
#endif
  0x00000000UL,
  10080000UL,
  38400000UL,
  16000UL,
  (1UL << 8) | 1UL,
  0x0C004639UL,
};

uint32_t generated[] = {
  0x01041FF0UL, 0x003F003FUL,
     /* 1FF4 */ 0x00000000UL,
     /* 1FF8 */ (uint32_t) &generated_phyInfo,
     /* 1FFC */ 0x00000000UL,
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
  0x00013008UL, 0x0100AC13UL,
  0x00023030UL, 0x00104639UL,
     /* 3034 */ 0x00000013UL,
  0x00013040UL, 0x00000000UL,
  0x000140A0UL, 0x0F0027AAUL,
  0x000140B8UL, 0x0023C000UL,
  0x000140F4UL, 0x00001020UL,
  0x00024134UL, 0x00000880UL,
     /* 4138 */ 0x000087F6UL,
  0x00024140UL, 0x00880020UL,
     /* 4144 */ 0x4D52E6C1UL,
  0x00044160UL, 0x00000000UL,
     /* 4164 */ 0x00000000UL,
     /* 4168 */ 0x00000006UL,
     /* 416C */ 0x00000006UL,
  0x00086014UL, 0x00000010UL,
     /* 6018 */ 0x04000000UL,
     /* 601C */ 0x00020001UL,
     /* 6020 */ 0x00001000UL,
     /* 6024 */ 0x000AD000UL,
     /* 6028 */ 0x03000000UL,
     /* 602C */ 0x00000000UL,
     /* 6030 */ 0x00000000UL,
  0x00066050UL, 0x006D7FBCUL,
     /* 6054 */ 0x00000441UL,
     /* 6058 */ 0x0000031CUL,
     /* 605C */ 0x00010012UL,
     /* 6060 */ 0x00000002UL,
     /* 6064 */ 0x00000000UL,
  0x000C6078UL, 0x046C0003UL,
     /* 607C */ 0x00000000UL,
     /* 6080 */ 0x0034035FUL,
     /* 6084 */ 0x00000000UL,
     /* 6088 */ 0x00000000UL,
     /* 608C */ 0x22140A04UL,
     /* 6090 */ 0x4F4A4132UL,
     /* 6094 */ 0x00000000UL,
     /* 6098 */ 0x00000000UL,
     /* 609C */ 0x00000000UL,
     /* 60A0 */ 0x00000000UL,
     /* 60A4 */ 0x00000000UL,
  0x000760E4UL, 0x84000080UL,
     /* 60E8 */ 0x00000000UL,
     /* 60EC */ 0x07830464UL,
     /* 60F0 */ 0x3AC81388UL,
     /* 60F4 */ 0x000A2090UL,
     /* 60F8 */ 0x00206100UL,
     /* 60FC */ 0x123556B7UL,
  0x00036104UL, 0x0010BB42UL,
     /* 6108 */ 0x29043020UL,
     /* 610C */ 0x0040BB88UL,
  0x00016120UL, 0x00000000UL,
  0x00016128UL, 0x0C660664UL,
  0x000C6130UL, 0x00FA53E8UL,
     /* 6134 */ 0x00000000UL,
     /* 6138 */ 0x00000000UL,
     /* 613C */ 0x00000000UL,
     /* 6140 */ 0x00000000UL,
     /* 6144 */ 0x00000000UL,
     /* 6148 */ 0x00000000UL,
     /* 614C */ 0x00000001UL,
     /* 6150 */ 0x00000000UL,
     /* 6154 */ 0x00000000UL,
     /* 6158 */ 0x00000000UL,
     /* 615C */ 0x00000000UL,
  0x00077014UL, 0x000270FEUL,
     /* 7018 */ 0x00001300UL,
     /* 701C */ 0x82EA0060UL,
     /* 7020 */ 0x00000000UL,
     /* 7024 */ 0x00000082UL,
     /* 7028 */ 0x00000000UL,
     /* 702C */ 0x000000D5UL,
  0x00027048UL, 0x0000383EUL,
     /* 704C */ 0x000025BCUL,
  0x00037070UL, 0x00120103UL,
     /* 7074 */ 0x0008302DUL,
     /* 7078 */ 0x006D8480UL,

  0xFFFFFFFFUL,
};

RAIL_ChannelConfigEntryAttr_t generated_entryAttr = {
  { 0xFFFFFFFFUL }
};

const RAIL_ChannelConfigEntry_t generated_channels[] = {
  {
    .phyConfigDeltaAdd = NULL,
    .baseFrequency = 434000000,
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
