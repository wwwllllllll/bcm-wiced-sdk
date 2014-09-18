#!/usr/bin/perl

#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

printf("/* \n");
printf(" * The MAC address of the Wi-Fi device may be configured in one of several places as \n");
printf(" * described in the document WICED-AN800-R Factory Programming Application Note.     \n");
printf(" * Please read this document for further information.                                \n");
printf(" */\n\n");

$a = rand()*256;
$b = rand()*256;
$c = rand()*256;
printf("#define NVRAM_GENERATED_MAC_ADDRESS   \"macaddr=02:0A:F7:%.2x:%.2x:%.2x\"\n", $a, $b, $c);
printf("#define DCT_GENERATED_MAC_ADDRESS     \"\\x02\\x0A\\xF7\\x%.2x\\x%.2x\\x%.2x\"", $c, $a, $b);