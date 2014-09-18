/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 * BCM94390 variables
 */

#ifndef INCLUDED_NVRAM_IMAGE_H_
#define INCLUDED_NVRAM_IMAGE_H_

#include <string.h>
#include <stdint.h>
#include "../generated_mac_address.txt"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Character array of NVRAM image
 */

// Sample variables file for BCM94390 module based on SPIL SB-WM-N03 SiP (2.4GHz ONLY)
// To be used with BCM9WCD3EVAL1 eval board
static const char wifi_nvram_image[] =
        "NVRAMRev=$Rev: 387583 $"                                                                      "\x00"
        "sromrev=11"                                                                                   "\x00"
        "boardrev=0x1102"                                                                              "\x00"
        "boardtype=0x0704"                                                                             "\x00"
        "boardflags=0x00480c01"                                                                        "\x00"
        "# iLNA bypass disabled:"                                                                      "\x00"
        "boardflags2=0x00002000"                                                                       "\x00"
        "# for iLNA bypass actiavted - boardflags2=0xc0000000"                                         "\x00"
        "boardflags3=0x8101188"                                                                        "\x00"
        "#boardnum=57410"                                                                              "\x00"
        NVRAM_GENERATED_MAC_ADDRESS                                                                    "\x00"
        "ccode=0"                                                                                      "\x00"
        "regrev=0"                                                                                     "\x00"
        "antswitch=0"                                                                                  "\x00"
        "pdgain2g=7"                                                                                   "\x00"
        "pdgain5g=0"                                                                                   "\x00"
        "tworangetssi2g=0"                                                                             "\x00"
        "tworangetssi5g=0"                                                                             "\x00"
        "femctrl=7"                                                                                    "\x00"
        "pcieingress_war=15"                                                                           "\x00"
        "vendid=0x14e4"                                                                                "\x00"
        "devid=0x43ae"                                                                                 "\x00"
        "manfid=0x2d0"                                                                                 "\x00"
        "#prodid=0x052e"                                                                               "\x00"
        "nocrc=1"                                                                                      "\x00"
        "otpimagesize=502"                                                                             "\x00"
        "xtalfreq=37400"                                                                               "\x00"
        "extpagain2g=2"                                                                                "\x00"
        "pdetrange2g=2"                                                                                "\x00"
        "extpagain5g=2"                                                                                "\x00"
        "pdetrange5g=2"                                                                                "\x00"
        "#gainctrlsph=1"                                                                               "\x00"
        "rxgains2gelnagaina0=0"                                                                        "\x00"
        "rxgains2gtrisoa0=5"                                                                           "\x00"
        "rxgains2gtrelnabypa0=0"                                                                       "\x00"
        "rxgains5gelnagaina0=0"                                                                        "\x00"
        "rxgains5gtrisoa0=14"                                                                          "\x00"
        "rxgains5gtrelnabypa0=0"                                                                       "\x00"
        "#pdgain5g=10"                                                                                 "\x00"
        "#pdgain2g=10"                                                                                 "\x00"
        "rxchain=1"                                                                                    "\x00"
        "txchain=1"                                                                                    "\x00"
        "aa2g=1"                                                                                       "\x00"
        "aa5g=1"                                                                                       "\x00"
        "tssipos5g=0"                                                                                  "\x00"
        "tssipos2g=0"                                                                                  "\x00"
        "pa2ga0=-144,6727,-776"                                                                        "\x00"
        "pa2gccka0=-111,7930,-899"                                                                     "\x00"
        "pa5ga0=0xFF61,0x163C,0xFD55,0xFF5D,0x1671,0xFD4F,0xFF5F,0x16CA,0xFD45,0xFF60,0x1676,0xFD4D"   "\x00"
        "pdoffset40ma0=0"                                                                              "\x00"
        "pdoffset80ma0=0"                                                                              "\x00"
        "maxp2ga0=76"                                                                                  "\x00"
        "cckbw202gpo=0x0000"                                                                           "\x00"
        "cckbw20ul2gpo=0x4444"                                                                         "\x00"
        "mcsbw202gpo=0x99866666"                                                                       "\x00"
        "dot11agofdmhrbw202gpo=0x5533"                                                                 "\x00"
        "ofdmlrbw202gpo=0x0000"                                                                        "\x00"
        "tssifloor2g=500"                                                                              "\x00"
        "cckdigfilttype=2"                                                                             "\x00"
        "swctrlmap_2g=0x12221222,0x11211121,0x11211121,0x0,0x0"                                        "\x00"
        "swctrlmapext_2g=0x00000000,0x00000000,0x00000000,0x000000,0x000"                              "\x00"
        "swctrlmap_5g=0x00000000,0x00000000,0x00000000,0x0,0x0"                                        "\x00"
        "swctrlmapext_5g=0x00000000,0x00000000,0x00000000,0x000000,0x000"                              "\x00"
        "paparambwver=1"                                                                               "\x00"
        "\x00\x00";

#ifdef __cplusplus
} /*extern "C" */
#endif

#else /* ifndef INCLUDED_NVRAM_IMAGE_H_ */

#error Wi-Fi NVRAM image included twice

#endif /* ifndef INCLUDED_NVRAM_IMAGE_H_ */
