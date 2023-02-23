# -*- coding: utf-8 -*-
################################################################################
#
# Author: ClimbSnail(HQ)
# original source is here.
#   https://github.com/ClimbSnail/HoloCubic_AIO_Tool
# 
#
################################################################################

from util.massagehead import *
import binascii
import ctypes
import inspect
import traceback
import re

VERSION = "Ver1.4.7"
ROOT_PATH = "OutFile"
CACHE_PATH = "Cache"

# 字节序定义
byteOrders = {'Native order': '@',  # 本机（默认）
              'Native standard': '=',  # 本机
              'Little-endian': '<',  # 小端
              'Big-endian': '>',  # 大端
              'Network order': '!'}  # network(大端)


# 关于struct格式串字节大小 https://blog.csdn.net/qq_30638831/article/details/80421019

def getSendInfo(info):
    """
    打印网络数据流, 
    :param info: ctypes.create_string_buffer()
    :return : str
    """
    info = binascii.hexlify(info)
    print(info)
    re_obj = re.compile('.{1,2}')  # 匹配任意字符1-2次
    t = ' '.join(re_obj.findall(str(info).upper()))
    return t


def _async_raise(thread_obj):
    """
    释放进程
    :param thread: 进程对象
    :param exctype:
    :return:
    """
    try:
        tid = thread_obj.ident
        tid = ctypes.c_long(tid)
        exctype = SystemExit
        """raises the exception, performs cleanup if needed"""
        if not inspect.isclass(exctype):
            exctype = type(exctype)
        res = ctypes.pythonapi.PyThreadState_SetAsyncExc(tid, ctypes.py_object(exctype))
        if res == 0:
            raise ValueError("invalid thread id")
        elif res != 1:
            # """if it returns a number greater than one, you're in trouble,
            # and you should call it again with exc=NULL to revert the effect"""
            ctypes.pythonapi.PyThreadState_SetAsyncExc(tid, None)
            raise SystemError("PyThreadState_SetAsyncExc failed")
    except Exception as err:
        print(err)
