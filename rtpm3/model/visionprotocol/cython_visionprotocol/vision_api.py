'''
 * Copyright Telechips Inc.
 *
 * TCC Version 1.0
 *
 * This source code contains confidential information of Telechips.
 *
 * Any unauthorized use without a written permission of Telechips including not
 * limited to re-distribution in source or binary form is strictly prohibited.
 *
 * This source code is provided "AS IS" and nothing contained in this source code
 * shall constitute any express or implied warranty of any kind, including without
 * limitation, any warranty of merchantability, fitness for a particular purpose
 * or non-infringement of any patent, copyright or other third party intellectual
 * property right.
 * No warranty is made, express or implied, regarding the information's accuracy,
 * completeness, or performance.
 *
 * In no event shall Telechips be liable for any claim, damages or other
 * liability arising from, out of or in connection with this source code or
 * the use in the source code.
 *
 * This source code is provided subject to the terms of a Mutual Non-Disclosure
 * Agreement between Telechips and Company.
'''

from model.visionprotocol.cython_visionprotocol import visionprotocol as vpm
import ctypes
import numpy as np
import time

# Define ctypes equivalents of C types and enums
uint64_t = ctypes.c_ulonglong
uint32_t = ctypes.c_ulong
uint16_t = ctypes.c_ushort
uint8_t = ctypes.c_ubyte

BLOCKING = -1
SYS_DEFAULT_BUFSIZE = 0

# vision_api function return value
VISION_SUCCESS          = 0
VISION_ERROR            = -1
ARGUMENT_ERROR          = -2
CONNECTION_ERROR        = -3
AUTHENTICATION_ERROR    = -4
QUEUE_OVERFLOW          = -5
QUEUE_UNDERFLOW         = -6
TIMEOUT_ERROR           = -7
MEMORY_ALLOCATION_ERROR = -8
BUF_REGISTRATION_ERROR  = -9
RECONNECTING            = -10

class OperationMode(ctypes.c_int):
    MESSAGE_MODE = 0
    STREAM_MODE = 1

class Interface(ctypes.c_int):
    INTERFACE_ETHERNET = 0
    INTERFACE_PCIE = 1

class Role(ctypes.c_int):
    SERVER = 0
    CLIENT = 1

class ActivationState(ctypes.c_int):
    OFF = 0
    ON = 1

class QueueInfo(ctypes.Structure):
    _fields_ = [
        ("maxDataSize", uint32_t),
        ("numQ", uint8_t)
    ]

class NetworkBufSize(ctypes.Structure):
    _fields_ = [
        ("sendBufSize", uint32_t),
        ("recvBufSize", uint32_t)
    ]

class vision_user_config_t(ctypes.Structure):
    _fields_ = [
        ("netInterface", uint8_t),
        ("netRole", uint8_t),
        ("netBuf", NetworkBufSize),
        ("ip", ctypes.c_char * 16),
        ("port", uint16_t),
        ("opMode", uint8_t),
        ("reconnection", uint8_t),
        ("streamZeroCopy", uint8_t),
        ("sendQ", QueueInfo),
        ("recvQ", QueueInfo)
    ]

class VisionStreamInfo(ctypes.Structure):
    _fields_ = [
        ("id", ctypes.c_uint32),
        ("pBuffer", ctypes.POINTER(ctypes.c_uint8)),
        ("length", ctypes.c_uint32),
        ("seqNum", ctypes.c_uint64),
        ("timestamp", ctypes.c_uint64)
    ]

class VisionMessage(ctypes.Structure):
    _fields_ = [
        ("id", ctypes.c_uint32),
        ("length", ctypes.c_uint32),
        ("seqNum", ctypes.c_uint64),
        ("timestamp", ctypes.c_uint64)
    ]

class VisionProtocolModule():
    def __init__(self, config):
        super(VisionProtocolModule, self).__init__()

        self.handle = 0
        ret = self.Initialization(config)
        if(ret == VISION_SUCCESS):
            self.sbuf = [None] * 0xFF
            self.rbuf = [None] * 0xFF
            if(config.opMode == OperationMode.STREAM_MODE):
                for i in range(config.sendQ.numQ):
                    self.sbuf[i] = np.array(bytearray(config.sendQ.maxDataSize), dtype=np.uint8)
                    ret, addr = self.RegisterStreamSendBuffer(self.sbuf[i], config.sendQ.maxDataSize)
                    if ret == VISION_SUCCESS:
                        print(f"[SUCCESS][RegisterSendBuf] idx: {i}, addr: {addr}")
                    else:
                        print(f"[ERROR][RegisterSendBuf] idx: {i}")
                
                for i in range(config.recvQ.numQ):
                    self.rbuf[i] = np.array(bytearray(config.recvQ.maxDataSize), dtype=np.uint8)
                    ret, addr = self.RegisterStreamRecvBuffer(self.rbuf[i], config.recvQ.maxDataSize)
                    if ret == VISION_SUCCESS:
                        print(f"[SUCCESS][RegisterRecvBuf] idx: {i}, addr: {addr}")
                    else:
                        print(f"[ERROR][RegisterRecvBuf] idx: {i}")

            ret = self.Connection()
            if(ret < 0):
                print("[ERROR] Connection failed. The program will now terminate")
                exit(0)
    
    def Initialization(self, config):
        self.handle = np.array(bytearray(1), dtype=np.uint64)
        ret = vpm.Vision_API_Initialization_Cython(config, self.handle)
        print("Initialization :", ret, "   handle :", self.handle)
        return ret
    
    def Deinitialization(self):
        ret = vpm.Vision_API_Deinitialization_Cython(self.handle)
        print("Deinitialization :", ret)
        return ret
    
    def RegisterStreamSendBuffer(self, buf, length):
        ret, addr = vpm.Vision_API_RegisterStreamSendBuffer_Cython(self.handle, buf, length)
        return ret, addr
    
    def RegisterStreamRecvBuffer(self, buf, length):
        ret, addr = vpm.Vision_API_RegisterStreamRecvBuffer_Cython(self.handle, buf, length)
        return ret, addr

    def Connection(self):
        ret = vpm.Vision_API_Connection_Cython(self.handle)
        print("Connection :", ret)
        return ret
    
    def Disconnection(self):
        ret = vpm.Vision_API_Disconnection_Cython(self.handle)
        print("Disconnection :", ret)
        return ret

    def SendMessage(self, packet, length, timeout=BLOCKING):
        ret = vpm.Vision_API_SendMessage_Cython(self.handle, packet, length, timeout)
        return ret

    def RecvMessage(self, packet, length, timeout=BLOCKING):
        ret = vpm.Vision_API_RecvMessage_Cython(self.handle, packet, length, timeout)
        return ret
    
    def PeekMessage(self, packet, length):
        ret = vpm.Vision_API_PeekMessage_Cython(self.handle, packet, length)
        return ret

    def GetStreamSendBuffer(self, timeout=BLOCKING):
        ret, x_addr, pIndex = vpm.Vision_API_GetStreamSendBuffer_Cython(self.handle, timeout)
        if ret == 0:
            x_ptr = ctypes.cast(x_addr, ctypes.POINTER(VisionStreamInfo))
            pStreamInfo = x_ptr.contents
        else:
            return ret, 0, 0
        return ret, pStreamInfo, pIndex

    def SendStream(self, pStreamInfo, pIndex):
        x_addr = ctypes.addressof(pStreamInfo)
        ret = vpm.Vision_API_SendStream_Cython(self.handle, x_addr, pIndex)
        return ret
    
    def RecvStream(self, timeout=BLOCKING):
        ret, x_addr, pIndex = vpm.Vision_API_RecvStream_Cython(self.handle, timeout)
        if ret == 0:
            x_ptr = ctypes.cast(x_addr, ctypes.POINTER(VisionStreamInfo))
            pStreamInfo = x_ptr.contents
            return ret, pStreamInfo, pIndex
        else:
            return ret, 0, 0

    def PeekStream(self):
        ret, x_addr, pIndex = vpm.Vision_API_PeekStream_Cython(self.handle)
        if ret == 0:
            x_ptr = ctypes.cast(x_addr, ctypes.POINTER(VisionStreamInfo))
            pStreamInfo = x_ptr.contents
            return ret, pStreamInfo, pIndex
        else:
            return ret, 0, 0
    
    def ReleaseStreamRecvBuffer(self, pStreamInfo, pIndex):
        x_addr = ctypes.addressof(pStreamInfo)
        ret = vpm.Vision_API_ReleaseStreamRecvBuffer_Cython(self.handle, x_addr, pIndex)
        return ret

    def buf2ndarray(self, pBuffer, len):
        # Cast the address to uint8
        x_ptr = ctypes.cast(pBuffer, ctypes.POINTER(ctypes.c_uint8))
        pBuffer = np.ctypeslib.as_array(x_ptr, shape=(len,))
        pBuffer_Addr = ctypes.addressof(x_ptr.contents)
        return pBuffer, pBuffer_Addr