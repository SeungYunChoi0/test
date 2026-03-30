# SPDX-License-Identifier: Apache-2.0

###################################################################################################
#
#   FileName : ruls.mk
#
#   Copyright (c) Telechips Inc.
#
#   Description :
#
#
###################################################################################################

MCU_BSP_APP_SAMPLE_I2C_TEST_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DMCU_BSP_SUPPORT_TEST_APP_I2C=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_I2C_TEST_PATH)
VPATH += $(MCU_BSP_DEV_DRIVERS_I2C_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_I2C_TEST_PATH)
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_I2C_PATH)
# Sources
SRCS += i2c_test.c
SRCS += lcd1602.c
SRCS += lcd.c
