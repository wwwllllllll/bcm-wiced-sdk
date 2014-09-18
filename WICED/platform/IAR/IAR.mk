#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := common_IAR

$(NAME)_SOURCES = stdio_IAR.c \
                  mem_IAR.c

# These need to be forced into the final ELF since they are not referenced otherwise
$(NAME)_LINK_FILES := stdio_IAR.o

GLOBAL_INCLUDES += .

