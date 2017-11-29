// Copyright 2017 Silicon Laboratories, Inc.
//
//

/***************************************************************************//**
 * @file rail_config.c
 * @brief RAIL Configuration
 * @copyright Copyright 2015 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/
// =============================================================================
//
//  WARNING: Auto-Generated Radio Config  -  DO NOT EDIT
//  Radio Configurator Version: 2.10.0
//  RAIL Adapter Version: 2.0.4
//  RAIL Compatibility: 1.x
//
// =============================================================================
#include "em_common.h"
#include "rail_config.h"

const uint32_t generated_phyInfo[] = {
  0UL,
  0x0009C09C, // 9.75238095238
};

const uint32_t generated[] = {
  0x01010FF0UL, (uint32_t) &generated_phyInfo,
  0x01010FF4UL, 0x00000000UL,
  0x01010FF8UL, 0x0003C002UL,
  0x01010FFCUL, 0x0003C002UL,
  0x00010004UL, 0x00000000UL,
  0x00010008UL, 0x00000000UL,
  0x00010018UL, 0x0000000FUL,
  0x0001001CUL, 0x00000000UL,
  0x00010028UL, 0x00000000UL,
  0x0001002CUL, 0x00000000UL,
  0x00010030UL, 0x00000000UL,
  0x00010034UL, 0x00000000UL,
  0x00010038UL, 0x00000000UL,
  0x0001003CUL, 0x00000000UL,
  0x00010040UL, 0x00000700UL,
  0x00010048UL, 0x00000000UL,
  0x00010054UL, 0x00000000UL,
  0x00010058UL, 0x00000000UL,
  0x000100A0UL, 0x00004CFFUL,
  0x000100A4UL, 0x00000000UL,
  0x000100A8UL, 0x00004DFFUL,
  0x000100ACUL, 0x00000000UL,
  0x00012000UL, 0x00000744UL,
  0x00012010UL, 0x00000000UL,
  0x00012018UL, 0x0000A001UL,
  0x00013008UL, 0x0000AC3FUL,
  0x00013030UL, 0x00101555UL,
  0x00013034UL, 0x00000001UL,
  0x00013040UL, 0x00000000UL,
  0x000140A0UL, 0x0F00277AUL,
  0x000140F4UL, 0x00001020UL,
  0x00014134UL, 0x00000880UL,
  0x00014138UL, 0x000087E6UL,
  0x00014140UL, 0x008800E0UL,
  0x00014144UL, 0x1153E6C1UL,
  0x00016014UL, 0x00000010UL,
  0x00016018UL, 0x04000000UL,
  0x0001601CUL, 0x0002000FUL,
  0x00016020UL, 0x000000C8UL,
  0x00016024UL, 0x000CD000UL,
  0x00016028UL, 0x03000000UL,
  0x0001602CUL, 0x00000000UL,
  0x00016030UL, 0x00DA7FBCUL,
  0x00016034UL, 0x00000399UL,
  0x00016038UL, 0x020006F8UL,
  0x0001603CUL, 0x00140012UL,
  0x00016040UL, 0x0000B16FUL,
  0x00016044UL, 0x00000000UL,
  0x00016048UL, 0x13C00714UL,
  0x0001604CUL, 0x00000000UL,
  0x00016050UL, 0x002E02D5UL,
  0x00016054UL, 0x00000000UL,
  0x00016058UL, 0x00000000UL,
  0x0001605CUL, 0x261C140DUL,
  0x00016060UL, 0x403D372FUL,
  0x00016064UL, 0x00000000UL,
  0x00017014UL, 0x000270F8UL,
  0x00017018UL, 0x00000300UL,
  0x0001701CUL, 0x82710060UL,
  0x00017028UL, 0x01800000UL,
  0x00017048UL, 0x00003D3CUL,
  0x0001704CUL, 0x000019BCUL,
  0x00017070UL, 0x00020105UL,
  0x00017074UL, 0x00000113UL,
  0x00017078UL, 0x00552300UL,

  0xFFFFFFFFUL,
};

const uint32_t *configList[] = {
  generated,
};

const char *configNames[] = {
  "generated",
};

RAIL_ChannelConfigEntry_t generated_channels[] = {
  { 0, 20, 1000, 2395000000 },
};

const RAIL_ChannelConfig_t generated_channelConfig = {
  generated_channels,
  1,
};

const RAIL_ChannelConfig_t *channelConfigs[] = {
  &generated_channelConfig,
};

RAIL_FrameType_t *frameTypeConfigList[] = {
  NULL
};

const uint8_t irCalConfig[] = {
  24, 63, 1, 6, 4, 16, 1, 0, 0, 1, 1, 6, 0, 16, 39, 0, 0, 5, 0, 1, 1, 0, 0, 0, 0
};


//        _  _                          
//       | )/ )         Wireless        
//    \\ |//,' __       Application     
//    (")(_)-"()))=-    Software        
//       (\\            Platform        
