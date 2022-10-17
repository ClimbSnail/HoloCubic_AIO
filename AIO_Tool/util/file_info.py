# -*- coding: utf-8 -*-
################################################################################
#
# Author: ClimbSnail(HQ)
# original source is here.
#   https://github.com/ClimbSnail/HoloCubic_AIO_Tool
# 
#
################################################################################

from util.common import *
from ctypes import *
import struct


class FileSystem(MsgHead):

    def __init__(self, action_type=AT.AT_FREE_STATUS):
        MsgHead.__init__(self, MT.MODULE_TYPE_C_FILE_MANAGER, MT.MODULE_TYPE_CUBIC_FILE_MANAGER)  # 一定要初始化父类
        self.action_type = action_type
        self.fmt = self.fmt + "1B"

    def __dir__(self):
        super_param = super().__dir__()
        return super_param + ["action_type"]


class DirCreate(FileSystem):

    def __init__(self, dir_path=""):
        FileSystem.__init__(self, AT.AT_DIR_CREATE)  # 一定要初始化父类
        self.dir_path = bytes(dir_path, encoding='utf8')
        self.fmt = self.fmt + "99s"
        print("self.fmt = ", end="")
        print(self.fmt)

    def __dir__(self):
        super_param = super().__dir__()
        return super_param + ["dir_path"]


class DirRemove(FileSystem):

    def __init__(self, dir_path=""):
        FileSystem.__init__(self, AT.AT_DIR_REMOVE)  # 一定要初始化父类
        self.dir_path = bytes(dir_path, encoding='utf8')
        self.fmt = self.fmt + "99s"
        print("self.fmt = ", end="")
        print(self.fmt)

    def __dir__(self):
        super_param = super().__dir__()
        return super_param + ["dir_path"]


class DirRename(FileSystem):

    def __init__(self, dir_cur_name="", dir_new_name=""):
        FileSystem.__init__(self, AT.AT_DIR_RENAME)  # 一定要初始化父类
        self.dir_cur_name = bytes(dir_cur_name, encoding='utf8')
        self.dir_new_name = bytes(dir_new_name, encoding='utf8')
        self.fmt = self.fmt + "99s99s"
        print("self.fmt = ", end="")
        print(self.fmt)

    def __dir__(self):
        super_param = super().__dir__()
        return super_param + ["dir_cur_name", "dir_new_name"]


class DirList(FileSystem):

    def __init__(self, dir_path="", dir_info=""):
        FileSystem.__init__(self, AT.AT_DIR_LIST)  # 一定要初始化父类
        self.dir_path = bytes(dir_path, encoding='utf8')
        self.dir_info = bytes(dir_info, encoding='utf8')
        self.fmt = self.fmt + "99s%ds" % len(self.dir_info)
        print("self.fmt = ", end="")
        print(self.fmt)

    def decode(self, network_data, byteOrder='!'):
        """
        消息的解码
        """
        size = super().decode(network_data, byteOrder)
        # 处理不定长的数据
        self.dir_info = network_data[size:]

    def __dir__(self):
        super_param = super().__dir__()
        return super_param + ["dir_path", "dir_info"]


class FileCreate(FileSystem):

    def __init__(self, file_name, file_size):
        FileSystem.__init__(self, AT.AT_FILE_CREATE)  # 一定要初始化父类
        self.file_name = bytes(file_name, encoding='utf8')
        self.file_size = file_size
        self.fmt = self.fmt + "99s1H"
        print("self.fmt = ", end="")
        print(self.fmt)

    def __dir__(self):
        super_param = super().__dir__()
        return super_param + ["file_name", "file_size"]


class FileWrite(FileSystem):

    def __init__(self, data=""):
        FileSystem.__init__(self, AT.AT_FILE_WRITE)  # 一定要初始化父类
        self.data = bytes(data, encoding='utf8')
        self.fmt = self.fmt + "%ds" % len(self.data)
        print("self.fmt = ", end="")
        print(self.fmt)

    def decode(self, network_data, byteOrder='!'):
        """
        消息的解码
        """
        size = super().decode(network_data, byteOrder)
        # 处理不定长的数据
        self.data = network_data[size:]

    def __dir__(self):
        super_param = super().__dir__()
        return super_param + ["data"]


class FileRead(FileSystem):

    def __init__(self, data=""):
        FileSystem.__init__(self, AT.AT_FILE_READ)  # 一定要初始化父类
        self.data = bytes(data, encoding='utf8')
        self.fmt = self.fmt + "%ds" % len(self.data)
        print("self.fmt = ", end="")
        print(self.fmt)

    def decode(self, network_data, byteOrder='!'):
        """
        消息的解码
        """
        size = super().decode(network_data, byteOrder)
        # 处理不定长的数据
        self.data = network_data[size:]

    def __dir__(self):
        super_param = super().__dir__()
        return super_param + ["data"]


class FileRemove(FileSystem):

    def __init__(self, file_name):
        FileSystem.__init__(self, AT.AT_FILE_REMOVE)  # 一定要初始化父类
        self.file_name = bytes(file_name, encoding='utf8')
        self.fmt = self.fmt + "99s"
        print("self.fmt = ", end="")
        print(self.fmt)

    def __dir__(self):
        super_param = super().__dir__()
        return super_param + ["file_name"]


class FileRename(FileSystem):

    def __init__(self, file_name):
        FileSystem.__init__(self, AT.AT_DIR_RENAME)  # 一定要初始化父类
        self.dir_cur_name = bytes(file_name, encoding='utf8')
        self.dir_new_name = bytes(file_name, encoding='utf8')
        self.fmt = self.fmt + "99s99s"
        print("self.fmt = ", end="")
        print(self.fmt)

    def __dir__(self):
        super_param = super().__dir__()
        return super_param + ["dir_cur_name", "dir_new_name"]


class FileGetInfo(FileSystem):

    def __init__(self, file_name, file_info=""):
        FileSystem.__init__(self, AT.AT_DIR_LIST)  # 一定要初始化父类
        self.file_name = bytes(file_name, encoding='utf8')
        self.file_info = bytes(file_info, encoding='utf8')
        self.fmt = self.fmt + "99s%ds" % len(self.file_info)
        print("self.fmt = ", end="")
        print(self.fmt)

    def decode(self, network_data, byteOrder='!'):
        """
        消息的解码
        """
        size = super().decode(network_data, byteOrder)
        # 处理不定长的数据
        self.dir_info = network_data[size:]

    def __dir__(self):
        super_param = super().__dir__()
        return super_param + ["file_name", "file_info"]

######################################################################################
# class FileSystem_TT(Structure):
#     # _fields_ = [
#     #         # ("msg_head", MsgHead),
#     #         ("action_type",c_byte)
#     #         ]
#     _fields_ = MsgHead_TT._fields_
#     _fields_.extend([("action_type",c_byte)]) 


# class FileCreate(Structure):
#     _fields_ = FileSystem._fields_
#     extend_param = [
#             ("file_system", FileSystem),
#             ("file_name",c_byte*99),
#             ("file_size",c_uint)
#             ]
#     _fields_.extend(extend_param)


# class FileWrite(Structure):
#     _fields_ = [
#             ("file_system", FileSystem),
#             ("file_data",c_byte*65536)
#             ]


# class FileRead(Structure):
#     _fields_ = [
#             ("file_system", FileSystem),
#             ("file_data",c_byte*65536)
#             ]


# class FileRemove(Structure):
#     _fields_ = [
#             ("file_system", FileSystem),
#             ("file_name",c_byte*99)
#             ]


# class FileRename(Structure):
#     _fields_ = [
#             ("file_system", FileSystem),
#             ("file_cur_name", c_byte*99),
#             ("file_new_name", c_byte*99)
#             ]


# class FileGetInfo(Structure):
#     _fields_ = [
#             ("file_system", FileSystem),
#             ("file_name",c_byte*99),
#             ("file_info",c_byte*99)
#             ]


# class DirCreate(Structure):
#     _fields_ = [
#             ("file_system", FileSystem),
#             ("file_name",c_byte*99)
#             ]


# class DirRemove(Structure):
#     _fields_ = [
#             ("file_system", FileSystem),
#             ("file_name",c_byte*99)
#             ]


# class DirRename(Structure):
#     _fields_ = [
#             ("file_system", FileSystem),
#             ("dir_cur_name", c_byte*99),
#             ("dir_new_name", c_byte*99)
#             ]


# class DirList(Structure):
#     _fields_ = [
#             ("file_system", FileSystem),
#             ("file_name", c_byte*99),
#             ("dir_info", c_byte*3000)
#             ]
