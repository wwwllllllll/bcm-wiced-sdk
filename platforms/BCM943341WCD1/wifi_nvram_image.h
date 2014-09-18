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
 *  NVRAM variables which define BCM43341 Parameters for 43341BO-P203 card on a BCM9WCD2WLRFFAD + BCM9WCD2EVAL1
 *
 *  from bcm943341wlagb_p203.txt
 *
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

static const char wifi_nvram_image[] =
        //# NVRAM file for BCM943341WLAGB_2 P203 board
        "manfid=0x2d0"                                                       "\x00"
        "prodid=0x0653"                                                      "\x00"
        "vendid=0x14e4"                                                      "\x00"
        "devid=0x4386"                                                       "\x00"
        "boardtype=0x0653"                                                   "\x00"
        "boardrev=0x1203"                                                    "\x00"
        "boardnum=22"                                                        "\x00"
        NVRAM_GENERATED_MAC_ADDRESS                                          "\x00"
        "sromrev=3"                                                          "\x00"
        //#boardflags:
        //# bit 19 3tswitch:   2.4GHz FEM: SP3T switch share with BT
        //# bit 16     nopa:   no external pa
        //#            keep original 0x200
        "boardflags=0x0090201"                                               "\x00"
        "xtalfreq=37400"                                                     "\x00"
        "nocrc=1"                                                            "\x00"
        "ag0=255"                                                            "\x00"
        "aa2g=1"                                                             "\x00"
        "aa5g=1"                                                             "\x00"
        "ccode=ALL"                                                          "\x00"
        "pa0itssit=0x20"                                                     "\x00"
        //#PA parameters for 2.4GHz
        "pa0b0=6747"                                                         "\x00"
        "pa0b1=-808"                                                         "\x00"
        "pa0b2=-178"                                                         "\x00"
        "tssifloor2g=69"                                                     "\x00"
        //# rssi params for 2.4GHz
        "rssismf2g=0xf"                                                      "\x00"
        "rssismc2g=0x8"                                                      "\x00"
        "rssisav2g=0x1"                                                      "\x00"
        "cckPwrOffset=3"                                                     "\x00"
        //# rssi params for 5GHz
        "rssismf5g=0xf"                                                      "\x00"
        "rssismc5g=0x7"                                                      "\x00"
        //#rssisav5g=0x1
        "rssisav5g=0x3"                                                      "\x00"
        //#PA parameters for lower a-band
        "pa1lob0=5659"                                                       "\x00"
        "pa1lob1=-693"                                                       "\x00"
        "pa1lob2=-178"                                                       "\x00"
        "tssifloor5gl=93"                                                    "\x00"
        //#PA parameters for midband
        "pa1b0=5172"                                                         "\x00"
        "pa1b1=-671"                                                         "\x00"
        "pa1b2=-212"                                                         "\x00"
        "tssifloor5gm=77"                                                    "\x00"
        //#PA paramasdeters for high band
        "pa1hib0=5320"                                                       "\x00"
        "pa1hib1=-663"                                                       "\x00"
        "pa1hib2=-179"                                                       "\x00"
        "tssifloor5gh=74"                                                    "\x00"
        "rxpo5g=0"                                                           "\x00"
        "maxp2ga0=0x4E"                                                      "\x00"
        //#  19.5dBm max; 18dBm target
        //#Per rate power back-offs for g band, in .5 dB steps. Set it once you have the right numbers.
        "cck2gpo=0x0000"                                                     "\x00"
        "ofdm2gpo=0x42000000"                                                "\x00"
        //# R54 16dBm; R48 17dBm; others 18dBm
        "mcs2gpo0=0x2222"                                                    "\x00"
        //# M0~ M4 17dBm
        "mcs2gpo1=0x7662"                                                    "\x00"
        //# M5M6 15dBm; M7 14.5dBm
        //#max power for 5G
        "maxp5ga0=0x46"                                                      "\x00"
        //# 16dBm target; 17.5dBm Max
        "maxp5gla0=0x46"                                                     "\x00"
        "maxp5gha0=0x46"                                                     "\x00"
        //#Per rate power back-offs for a band, in .5 dB steps. Set it once you have the right numbers.
        "ofdm5gpo=0x52222222"                                                "\x00"
        //# R54 13.5dBm
        "ofdm5glpo=0x52222222"                                               "\x00"
        "ofdm5ghpo=0x52222222"                                               "\x00"
        "mcs5gpo0=0x0000"                                                    "\x00"
        //# M0~M4 16dBm (1dB higher than ofdm)
        "mcs5gpo1=0x8550"                                                    "\x00"
        //# M5M6 13.5dBm; M7 12dBm
        "mcs5glpo0=0x0000"                                                   "\x00"
        "mcs5glpo1=0x8550"                                                   "\x00"
        "mcs5ghpo0=0x0000"                                                   "\x00"
        "mcs5ghpo1=0x8550"                                                   "\x00"
        //# Parameters for DAC2x mode and ALPF bypass
        //# RF SW Truth Table: ctrl0 for BT_TX; ctrl1 or 5G Tx; ctrl2 for 5G Rx; Ctrl3 for 2G Tx; Ctrl4 for 2G Rx
        "swctrlmap_2g=0x00080008,0x00100010,0x00080008,0x011010,0x11f"       "\x00"
        "swctrlmap_5g=0x00020002,0x00040004,0x00020002,0x011010,0x2fe"       "\x00"
        "gain=32"                                                            "\x00"
        "triso2g=8"                                                          "\x00"
        "triso5g=8"                                                          "\x00"
        //#tx parameters
        "loflag=0"                                                           "\x00"
        "iqlocalidx5g=40"                                                    "\x00"
        "dlocalidx5g=70"                                                     "\x00"
        "iqcalidx5g=50"                                                      "\x00"
        "lpbckmode5g=1"                                                      "\x00"
        "txiqlopapu5g=0"                                                     "\x00"
        "txiqlopapu2g=0"                                                     "\x00"
        "dlorange_lowlimit=5"                                                "\x00"
        "txalpfbyp=1"                                                        "\x00"
        "txalpfpu=1"                                                         "\x00"
        "dacrate2xen=1"                                                      "\x00"
        "papden2g=1"                                                         "\x00"
        "papden5g=1"                                                         "\x00"
        //#rx parameters
        "gain_settle_dly_2g=4"                                               "\x00"
        "gain_settle_dly_5g=4"                                               "\x00"
        "noise_cal_po_2g=-1"                                                 "\x00"
        "noise_cal_po_40_2g=-1"                                              "\x00"
        "noise_cal_high_gain_2g=73"                                          "\x00"
        "noise_cal_nf_substract_val_2g=346"                                  "\x00"
        "noise_cal_po_5g=-1"                                                 "\x00"
        "noise_cal_po_40_5g=-1"                                              "\x00"
        "noise_cal_high_gain_5g=73"                                          "\x00"
        "noise_cal_nf_substract_val_5g=346"                                  "\x00"
        "cckpapden=0"                                                        "\x00"
        "paparambwver=1"                                                     "\x00"
        "\x00\x00";


#ifdef __cplusplus
} /* extern "C" */
#endif

#else /* ifndef INCLUDED_NVRAM_IMAGE_H_ */

#error Wi-Fi NVRAM image included twice

#endif /* ifndef INCLUDED_NVRAM_IMAGE_H_ */
