

#ifndef __NETWORK_H__
#define __NETWORK_H__

/*
    Openedges Enlight Network Compiler
*/

#include "enlight_data_type.h"
#include "enlight_network.h"
#ifdef BARE_METAL_FW_DEV
#    include "npu_cmd.h"
#endif


static enlight_act_tensor_t Input_0 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    1228800,
    3,
    4,
    {1, 3, 640, 640},
    128.0000,
    0,
    0,
    "Input_0",
};

static enlight_act_tensor_t Mul_2 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x320000,
    1638400,
    2,
    4,
    {1, 64, 160, 160},
    9.8547,
    0,
    0,
    "Mul_2",
};

static enlight_act_tensor_t Slice_0 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x258000,
    819200,
    2,
    4,
    {1, 32, 160, 160},
    9.8547,
    0,
    0,
    "Slice_0",
};

static enlight_act_tensor_t DummyMulConst_0 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    819200,
    2,
    4,
    {1, 96, 160, 160},
    7.3727,
    0,
    0,
    "DummyMulConst_0",
};

static enlight_act_tensor_t Slice_1 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x258000,
    819200,
    2,
    4,
    {1, 32, 160, 160},
    9.8547,
    0,
    0,
    "Slice_1",
};

static enlight_act_tensor_t DummyMulConst_1 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0xc8000,
    819200,
    2,
    4,
    {1, 96, 160, 160},
    7.3727,
    0,
    0,
    "DummyMulConst_1",
};

static enlight_act_tensor_t Add_0 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x190000,
    819200,
    2,
    4,
    {1, 96, 160, 160},
    7.3727,
    0,
    0,
    "Add_0",
};

static enlight_act_tensor_t Concat_0 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    2457600,
    2,
    4,
    {1, 96, 160, 160},
    7.3727,
    0,
    0,
    "Concat_0",
};

static enlight_act_tensor_t Mul_7 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x258000,
    819200,
    2,
    4,
    {1, 128, 80, 80},
    19.6371,
    0,
    0,
    "Mul_7",
};

static enlight_act_tensor_t Slice_2 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x190000,
    409600,
    2,
    4,
    {1, 64, 80, 80},
    19.6371,
    0,
    0,
    "Slice_2",
};

static enlight_act_tensor_t DummyMulConst_2 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    409600,
    2,
    4,
    {1, 256, 80, 80},
    15.3552,
    0,
    0,
    "DummyMulConst_2",
};

static enlight_act_tensor_t Slice_3 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x190000,
    409600,
    2,
    4,
    {1, 64, 80, 80},
    19.6371,
    0,
    0,
    "Slice_3",
};

static enlight_act_tensor_t DummyMulConst_3 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x64000,
    409600,
    2,
    4,
    {1, 256, 80, 80},
    15.3552,
    0,
    0,
    "DummyMulConst_3",
};

static enlight_act_tensor_t Add_1 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x1f4000,
    409600,
    2,
    4,
    {1, 64, 80, 80},
    17.9485,
    0,
    0,
    "Add_1",
};

static enlight_act_tensor_t DummyMulConst_4 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0xc8000,
    409600,
    2,
    4,
    {1, 256, 80, 80},
    15.3552,
    0,
    0,
    "DummyMulConst_4",
};

static enlight_act_tensor_t Add_2 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x12c000,
    409600,
    2,
    4,
    {1, 256, 80, 80},
    15.3552,
    0,
    0,
    "Add_2",
};

static enlight_act_tensor_t Concat_1 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    1638400,
    2,
    4,
    {1, 256, 80, 80},
    15.3552,
    0,
    0,
    "Concat_1",
};

static enlight_act_tensor_t Mul_12 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x190000,
    819200,
    2,
    4,
    {1, 128, 80, 80},
    23.9738,
    0,
    0,
    "Mul_12",
};

static enlight_act_tensor_t DummyMulConst_5 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x41a000,
    819200,
    2,
    4,
    {1, 384, 80, 80},
    29.4667,
    0,
    0,
    "DummyMulConst_5",
};

static enlight_act_tensor_t Mul_14 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    409600,
    2,
    4,
    {1, 256, 40, 40},
    20.2336,
    0,
    0,
    "Mul_14",
};

static enlight_act_tensor_t Slice_4 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x64000,
    204800,
    2,
    4,
    {1, 128, 40, 40},
    20.2336,
    0,
    0,
    "Slice_4",
};

static enlight_act_tensor_t DummyMulConst_6 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x4e2000,
    204800,
    2,
    4,
    {1, 512, 40, 40},
    15.2680,
    0,
    0,
    "DummyMulConst_6",
};

static enlight_act_tensor_t Slice_5 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x64000,
    204800,
    2,
    4,
    {1, 128, 40, 40},
    20.2336,
    0,
    0,
    "Slice_5",
};

static enlight_act_tensor_t DummyMulConst_7 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x514000,
    204800,
    2,
    4,
    {1, 512, 40, 40},
    15.2680,
    0,
    0,
    "DummyMulConst_7",
};

static enlight_act_tensor_t Add_3 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    204800,
    2,
    4,
    {1, 128, 40, 40},
    18.3144,
    0,
    0,
    "Add_3",
};

static enlight_act_tensor_t DummyMulConst_8 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x546000,
    204800,
    2,
    4,
    {1, 512, 40, 40},
    15.2680,
    0,
    0,
    "DummyMulConst_8",
};

static enlight_act_tensor_t Add_4 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x578000,
    204800,
    2,
    4,
    {1, 512, 40, 40},
    15.2680,
    0,
    0,
    "Add_4",
};

static enlight_act_tensor_t Concat_2 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x4e2000,
    819200,
    2,
    4,
    {1, 512, 40, 40},
    15.2680,
    0,
    0,
    "Concat_2",
};

static enlight_act_tensor_t Mul_19 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    409600,
    2,
    4,
    {1, 256, 40, 40},
    31.2093,
    0,
    0,
    "Mul_19",
};

static enlight_act_tensor_t DummyMulConst_9 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x190000,
    409600,
    2,
    4,
    {1, 768, 40, 40},
    34.4791,
    0,
    0,
    "DummyMulConst_9",
};

static enlight_act_tensor_t Mul_21 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x23f000,
    204800,
    2,
    4,
    {1, 512, 20, 20},
    30.0311,
    0,
    0,
    "Mul_21",
};

static enlight_act_tensor_t Slice_6 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x4e2000,
    102400,
    2,
    4,
    {1, 256, 20, 20},
    30.0311,
    0,
    0,
    "Slice_6",
};

static enlight_act_tensor_t DummyMulConst_10 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x1f4000,
    102400,
    2,
    4,
    {1, 768, 20, 20},
    19.2373,
    0,
    0,
    "DummyMulConst_10",
};

static enlight_act_tensor_t Slice_7 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x4e2000,
    102400,
    2,
    4,
    {1, 256, 20, 20},
    30.0311,
    0,
    0,
    "Slice_7",
};

static enlight_act_tensor_t DummyMulConst_11 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x20d000,
    102400,
    2,
    4,
    {1, 768, 20, 20},
    19.2373,
    0,
    0,
    "DummyMulConst_11",
};

static enlight_act_tensor_t Add_5 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x226000,
    102400,
    2,
    4,
    {1, 768, 20, 20},
    19.2373,
    0,
    0,
    "Add_5",
};

static enlight_act_tensor_t Concat_3 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x1f4000,
    307200,
    2,
    4,
    {1, 768, 20, 20},
    19.2373,
    0,
    0,
    "Concat_3",
};

static enlight_act_tensor_t Conv2d_25 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    102400,
    2,
    4,
    {1, 1024, 20, 20},
    10.0329,
    0,
    0,
    "Conv2d_25",
};

static enlight_act_tensor_t MaxPool2d_0 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x19000,
    102400,
    2,
    4,
    {1, 1024, 20, 20},
    10.0329,
    0,
    0,
    "MaxPool2d_0",
};

static enlight_act_tensor_t MaxPool2d_1 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x32000,
    102400,
    2,
    4,
    {1, 1024, 20, 20},
    10.0329,
    0,
    0,
    "MaxPool2d_1",
};

static enlight_act_tensor_t MaxPool2d_2 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x4b000,
    102400,
    2,
    4,
    {1, 1024, 20, 20},
    10.0329,
    0,
    0,
    "MaxPool2d_2",
};

static enlight_act_tensor_t Concat_4 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    409600,
    2,
    4,
    {1, 1024, 20, 20},
    10.0329,
    0,
    0,
    "Concat_4",
};

static enlight_act_tensor_t Mul_25 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x5f5000,
    204800,
    2,
    4,
    {1, 512, 20, 20},
    36.5681,
    0,
    0,
    "Mul_25",
};

static enlight_act_tensor_t Upsample_0 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x4e2000,
    819200,
    2,
    4,
    {1, 512, 40, 40},
    36.5681,
    0,
    0,
    "Upsample_0",
};

static enlight_act_tensor_t DummyMulConst_12 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0xc8000,
    819200,
    2,
    4,
    {1, 768, 40, 40},
    34.4791,
    0,
    0,
    "DummyMulConst_12",
};

static enlight_act_tensor_t Concat_5 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0xc8000,
    1228800,
    2,
    4,
    {1, 768, 40, 40},
    34.4791,
    0,
    0,
    "Concat_5",
};

static enlight_act_tensor_t Mul_26 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    409600,
    2,
    4,
    {1, 256, 40, 40},
    25.0986,
    0,
    0,
    "Mul_26",
};

static enlight_act_tensor_t Slice_8 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x12c000,
    204800,
    2,
    4,
    {1, 128, 40, 40},
    25.0986,
    0,
    0,
    "Slice_8",
};

static enlight_act_tensor_t DummyMulConst_14 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x96000,
    204800,
    2,
    4,
    {1, 384, 40, 40},
    23.9171,
    0,
    0,
    "DummyMulConst_14",
};

static enlight_act_tensor_t Slice_9 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x12c000,
    204800,
    2,
    4,
    {1, 128, 40, 40},
    25.0986,
    0,
    0,
    "Slice_9",
};

static enlight_act_tensor_t DummyMulConst_15 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0xc8000,
    204800,
    2,
    4,
    {1, 384, 40, 40},
    23.9171,
    0,
    0,
    "DummyMulConst_15",
};

static enlight_act_tensor_t Mul_28 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0xfa000,
    204800,
    2,
    4,
    {1, 384, 40, 40},
    23.9171,
    0,
    0,
    "Mul_28",
};

static enlight_act_tensor_t Concat_6 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x96000,
    614400,
    2,
    4,
    {1, 384, 40, 40},
    23.9171,
    0,
    0,
    "Concat_6",
};

static enlight_act_tensor_t Mul_29 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x4e2000,
    409600,
    2,
    4,
    {1, 256, 40, 40},
    34.3886,
    0,
    0,
    "Mul_29",
};

static enlight_act_tensor_t Upsample_1 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x96000,
    1638400,
    2,
    4,
    {1, 256, 80, 80},
    34.3886,
    0,
    0,
    "Upsample_1",
};

static enlight_act_tensor_t DummyMulConst_16 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x28a000,
    1638400,
    2,
    4,
    {1, 384, 80, 80},
    29.4667,
    0,
    0,
    "DummyMulConst_16",
};

static enlight_act_tensor_t Concat_7 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x28a000,
    2457600,
    2,
    4,
    {1, 384, 80, 80},
    29.4667,
    0,
    0,
    "Concat_7",
};

static enlight_act_tensor_t Mul_30 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x1c2000,
    819200,
    2,
    4,
    {1, 128, 80, 80},
    24.7750,
    0,
    0,
    "Mul_30",
};

static enlight_act_tensor_t Slice_10 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x28a000,
    409600,
    2,
    4,
    {1, 64, 80, 80},
    24.7750,
    0,
    0,
    "Slice_10",
};

static enlight_act_tensor_t DummyMulConst_18 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x96000,
    409600,
    2,
    4,
    {1, 192, 80, 80},
    21.8536,
    0,
    0,
    "DummyMulConst_18",
};

static enlight_act_tensor_t Slice_11 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x28a000,
    409600,
    2,
    4,
    {1, 64, 80, 80},
    24.7750,
    0,
    0,
    "Slice_11",
};

static enlight_act_tensor_t DummyMulConst_19 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0xfa000,
    409600,
    2,
    4,
    {1, 192, 80, 80},
    21.8536,
    0,
    0,
    "DummyMulConst_19",
};

static enlight_act_tensor_t Mul_32 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x15e000,
    409600,
    2,
    4,
    {1, 192, 80, 80},
    21.8536,
    0,
    0,
    "Mul_32",
};

static enlight_act_tensor_t Concat_8 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x96000,
    1228800,
    2,
    4,
    {1, 192, 80, 80},
    21.8536,
    0,
    0,
    "Concat_8",
};

static enlight_act_tensor_t Mul_33 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x1c2000,
    819200,
    2,
    4,
    {1, 128, 80, 80},
    25.9953,
    0,
    0,
    "Mul_33",
};

static enlight_act_tensor_t Conv2d_45 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    409600,
    4,
    4,
    {1, 63, 80, 80},
    13.4121,
    0,
    1,
    "Conv2d_45",
};

static enlight_act_tensor_t Conv2d_43 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x64000,
    409600,
    4,
    4,
    {1, 64, 80, 80},
    7.0627,
    0,
    1,
    "Conv2d_43",
};

static enlight_act_tensor_t Conv2d_44 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0xc8000,
    204800,
    4,
    4,
    {1, 1, 80, 80},
    5.1448,
    0,
    1,
    "Conv2d_44",
};

static enlight_act_tensor_t Mul_34 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    204800,
    2,
    4,
    {1, 384, 40, 40},
    31.1857,
    0,
    0,
    "Mul_34",
};

static enlight_act_tensor_t DummyMulConst_17 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x32000,
    409600,
    2,
    4,
    {1, 384, 40, 40},
    31.1857,
    0,
    0,
    "DummyMulConst_17",
};

static enlight_act_tensor_t Concat_9 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    614400,
    2,
    4,
    {1, 384, 40, 40},
    31.1857,
    0,
    0,
    "Concat_9",
};

static enlight_act_tensor_t Mul_41 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0xc8000,
    409600,
    2,
    4,
    {1, 256, 40, 40},
    26.2007,
    0,
    0,
    "Mul_41",
};

static enlight_act_tensor_t Slice_12 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x96000,
    204800,
    2,
    4,
    {1, 128, 40, 40},
    26.2007,
    0,
    0,
    "Slice_12",
};

static enlight_act_tensor_t DummyMulConst_20 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    204800,
    2,
    4,
    {1, 384, 40, 40},
    21.3962,
    0,
    0,
    "DummyMulConst_20",
};

static enlight_act_tensor_t Slice_13 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x96000,
    204800,
    2,
    4,
    {1, 128, 40, 40},
    26.2007,
    0,
    0,
    "Slice_13",
};

static enlight_act_tensor_t DummyMulConst_21 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x32000,
    204800,
    2,
    4,
    {1, 384, 40, 40},
    21.3962,
    0,
    0,
    "DummyMulConst_21",
};

static enlight_act_tensor_t Mul_43 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x64000,
    204800,
    2,
    4,
    {1, 384, 40, 40},
    21.3962,
    0,
    0,
    "Mul_43",
};

static enlight_act_tensor_t Concat_10 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    614400,
    2,
    4,
    {1, 384, 40, 40},
    21.3962,
    0,
    0,
    "Concat_10",
};

static enlight_act_tensor_t Mul_44 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x96000,
    409600,
    2,
    4,
    {1, 256, 40, 40},
    22.3367,
    0,
    0,
    "Mul_44",
};

static enlight_act_tensor_t Conv2d_59 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0xfa000,
    102400,
    4,
    4,
    {1, 63, 40, 40},
    10.2911,
    0,
    1,
    "Conv2d_59",
};

static enlight_act_tensor_t Conv2d_57 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x113000,
    102400,
    4,
    4,
    {1, 64, 40, 40},
    5.5143,
    0,
    1,
    "Conv2d_57",
};

static enlight_act_tensor_t Mul_47 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    204800,
    2,
    4,
    {1, 128, 40, 40},
    21.2285,
    0,
    0,
    "Mul_47",
};

static enlight_act_tensor_t Conv2d_58 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x12c000,
    51200,
    4,
    4,
    {1, 1, 40, 40},
    3.3314,
    0,
    1,
    "Conv2d_58",
};

static enlight_act_tensor_t Mul_45 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x5aa000,
    102400,
    2,
    4,
    {1, 768, 20, 20},
    30.5862,
    0,
    0,
    "Mul_45",
};

static enlight_act_tensor_t DummyMulConst_13 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x5c3000,
    204800,
    2,
    4,
    {1, 768, 20, 20},
    30.5862,
    0,
    0,
    "DummyMulConst_13",
};

static enlight_act_tensor_t Concat_11 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x5aa000,
    307200,
    2,
    4,
    {1, 768, 20, 20},
    30.5862,
    0,
    0,
    "Concat_11",
};

static enlight_act_tensor_t Mul_52 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x64000,
    204800,
    2,
    4,
    {1, 512, 20, 20},
    28.2829,
    0,
    0,
    "Mul_52",
};

static enlight_act_tensor_t Slice_14 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x4b000,
    102400,
    2,
    4,
    {1, 256, 20, 20},
    28.2829,
    0,
    0,
    "Slice_14",
};

static enlight_act_tensor_t DummyMulConst_22 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    102400,
    2,
    4,
    {1, 768, 20, 20},
    21.0407,
    0,
    0,
    "DummyMulConst_22",
};

static enlight_act_tensor_t Slice_15 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x4b000,
    102400,
    2,
    4,
    {1, 256, 20, 20},
    28.2829,
    0,
    0,
    "Slice_15",
};

static enlight_act_tensor_t DummyMulConst_23 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x19000,
    102400,
    2,
    4,
    {1, 768, 20, 20},
    21.0407,
    0,
    0,
    "DummyMulConst_23",
};

static enlight_act_tensor_t Mul_53 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x64000,
    102400,
    2,
    4,
    {1, 256, 20, 20},
    26.0920,
    0,
    0,
    "Mul_53",
};

static enlight_act_tensor_t Mul_54 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x32000,
    102400,
    2,
    4,
    {1, 768, 20, 20},
    21.0407,
    0,
    0,
    "Mul_54",
};

static enlight_act_tensor_t Concat_12 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    307200,
    2,
    4,
    {1, 768, 20, 20},
    21.0407,
    0,
    0,
    "Concat_12",
};

static enlight_act_tensor_t Mul_55 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x4b000,
    204800,
    2,
    4,
    {1, 512, 20, 20},
    15.2674,
    0,
    0,
    "Mul_55",
};

static enlight_act_tensor_t Mul_58 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    25600,
    2,
    4,
    {1, 63, 20, 20},
    10.2460,
    0,
    0,
    "Mul_58",
};

static enlight_act_tensor_t Conv2d_71 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x138800,
    25600,
    4,
    4,
    {1, 63, 20, 20},
    5.6841,
    0,
    1,
    "Conv2d_71",
};

static enlight_act_tensor_t Mul_56 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    25600,
    2,
    4,
    {1, 64, 20, 20},
    11.6470,
    0,
    0,
    "Mul_56",
};

static enlight_act_tensor_t Conv2d_69 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x13ec00,
    25600,
    4,
    4,
    {1, 64, 20, 20},
    3.9199,
    0,
    1,
    "Conv2d_69",
};

static enlight_act_tensor_t Mul_57 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x0,
    51200,
    2,
    4,
    {1, 128, 20, 20},
    23.1493,
    0,
    0,
    "Mul_57",
};

static enlight_act_tensor_t Conv2d_70 = {
    ENLIGHT_DTYPE_INT8,
    (void*)0x0,
    0x145000,
    12800,
    4,
    4,
    {1, 1, 20, 20},
    1.1914,
    0,
    1,
    "Conv2d_70",
};

/* Buf description of Activation and Output*/
static enlight_act_tensor_t* all_output_tensors[101] = {
    &Input_0,
    &Mul_2,
    &Slice_0,
    &DummyMulConst_0,
    &Slice_1,
    &DummyMulConst_1,
    &Add_0,
    &Concat_0,
    &Mul_7,
    &Slice_2,
    &DummyMulConst_2,
    &Slice_3,
    &DummyMulConst_3,
    &Add_1,
    &DummyMulConst_4,
    &Add_2,
    &Concat_1,
    &Mul_12,
    &DummyMulConst_5,
    &Mul_14,
    &Slice_4,
    &DummyMulConst_6,
    &Slice_5,
    &DummyMulConst_7,
    &Add_3,
    &DummyMulConst_8,
    &Add_4,
    &Concat_2,
    &Mul_19,
    &DummyMulConst_9,
    &Mul_21,
    &Slice_6,
    &DummyMulConst_10,
    &Slice_7,
    &DummyMulConst_11,
    &Add_5,
    &Concat_3,
    &Conv2d_25,
    &MaxPool2d_0,
    &MaxPool2d_1,
    &MaxPool2d_2,
    &Concat_4,
    &Mul_25,
    &Upsample_0,
    &DummyMulConst_12,
    &Concat_5,
    &Mul_26,
    &Slice_8,
    &DummyMulConst_14,
    &Slice_9,
    &DummyMulConst_15,
    &Mul_28,
    &Concat_6,
    &Mul_29,
    &Upsample_1,
    &DummyMulConst_16,
    &Concat_7,
    &Mul_30,
    &Slice_10,
    &DummyMulConst_18,
    &Slice_11,
    &DummyMulConst_19,
    &Mul_32,
    &Concat_8,
    &Mul_33,
    &Conv2d_45,
    &Conv2d_43,
    &Conv2d_44,
    &Mul_34,
    &DummyMulConst_17,
    &Concat_9,
    &Mul_41,
    &Slice_12,
    &DummyMulConst_20,
    &Slice_13,
    &DummyMulConst_21,
    &Mul_43,
    &Concat_10,
    &Mul_44,
    &Conv2d_59,
    &Conv2d_57,
    &Mul_47,
    &Conv2d_58,
    &Mul_45,
    &DummyMulConst_13,
    &Concat_11,
    &Mul_52,
    &Slice_14,
    &DummyMulConst_22,
    &Slice_15,
    &DummyMulConst_23,
    &Mul_53,
    &Mul_54,
    &Concat_12,
    &Mul_55,
    &Mul_58,
    &Conv2d_71,
    &Mul_56,
    &Conv2d_69,
    &Mul_57,
    &Conv2d_70,
};


static enlight_custom_postproc_t post_ext_param = {
    {&Conv2d_45,&Conv2d_43,&Conv2d_44,&Conv2d_59,&Conv2d_57,&Conv2d_58,&Conv2d_71,&Conv2d_69,&Conv2d_70,},
    9,
};

static enlight_buf_size_t network_buf_size = {
    //code_buf_size
    0x19600, 
    //wght_buf_size
    0xb40080, 
    //input_buf_size
    0x12c000, 
    //output_buf_size
    0x148200, 
    //work_buf_size
    0x627000, 
};

enlight_network_t network_instance = {
    //network_name
    "hand640_qt.enlight",
    //post_proc
    ENLIGHT_POST_CUSTOM,
    //batch_size
    1,
    //img_sizes
    {3, 640, 640},
    //buf_sizes
    &network_buf_size,
    //post_proc_extension
    (void*)&post_ext_param,
    //output_tensors_num
    9,
    //output_tensors
    {&Conv2d_45,&Conv2d_43,&Conv2d_44,&Conv2d_59,&Conv2d_57,&Conv2d_58,&Conv2d_71,&Conv2d_69,&Conv2d_70,},
    //Conv MAC num
    15654504480,

    //For Debug only
    //all_tensors_num,
    1519,
    //all_tensors
    all_output_tensors,

    // For toolkit developer
#ifdef BARE_METAL_FW_DEV
    //npu_cmd_codes
    npu_cmd,
#endif
};

#endif /*__NETWORK_H__*/