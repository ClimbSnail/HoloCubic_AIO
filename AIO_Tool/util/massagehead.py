# -*- coding: utf-8 -*-
################################################################################
#
# Author: ClimbSnail(HQ)
# original source is here.
#   https://github.com/ClimbSnail/HoloCubic_AIO_Tool
# 
#
################################################################################

from ctypes import *
from util.common import *
import struct

# 模块名 M_
M_ALL = "M_ALL"
M_ENGINE = "M_ENGINE"
M_DOWNLOAD_DEBUG = "M_DOWNLOAD_DEBUG"
M_SETTING = "M_SETTING"
M_FILE_MANAGER = "M_FILE_MANAGER"
M_PICTURE = "M_PICTURE"
M_VIDEO_TOOL = "M_VIDEO_TOOL"
M_SRCEEN_SHARE = "M_SRCEEN_SHARE"
M_HELP = "M_HELP"

# 动作类型 A_
A_CLOSE_UART = "A_CLOSE_UART"
A_OPEN_UART = "A_OPEN_UART"


class ModuleType:
    class ConstError(TypeError): pass

    # 模块名 未知
    MODULE_TYPE_UNKNOW = 0

    # 模块名 Holocubic
    MODULE_TYPE_CUBIC_FILE_MANAGER = 1

    # 上位机控制器
    MODULE_TYPE_C_FILE_MANAGER = 2

    # Holocubic的settings模块
    MODULE_TYPE_CUBIC_SETTINGS = 3

    # 上位机控制器的settings模块
    MODULE_TYPE_TOOL_SETTINGS = 4

    def __setattr__(self, name, value):
        raise self.ConstError(f"Can't rebind const {name}")


MT = ModuleType()  # 模块类型


class ActionType:
    class ConstError(TypeError): pass

    # 未知类型
    AT_UNKNOWN = 0
    AT_FREE_STATUS = 1

    # 目录操作
    AT_DIR_CREATE = 2  # 创建
    AT_DIR_REMOVE = 3  # 删除
    AT_DIR_RENAME = 4  # 重命名
    AT_DIR_LIST = 5  # 列举目录文件

    # 文件操作
    AT_FILE_CREATE = 6  # 创建
    AT_FILE_WRITE = 7  # 文件信息流写
    AT_FILE_READ = 8  # 文件信息读
    AT_FILE_REMOVE = 9  # 删除
    AT_FILE_RENAME = 10  # 重命名
    AT_FILE_GET_INFO = 11  # 查询文件大小

    AT_SETTING_SET = 12 # 设置属性
    AT_SETTING_GET = 13 # 获取属性

    def __setattr__(self, name, value):
        raise self.ConstError(f"Can't rebind const {name}")


AT = ActionType()

class ValueType:
    class ConstError(TypeError): pass

    # 值类型 未知
    VALUE_TYPE_UNKNOWN = 0

    # int
    VALUE_TYPE_INT = 1

    # uchar
    VALUE_TYPE_UCHAR = 2

    # String
    VALUE_TYPE_STRING = 3

    def __setattr__(self, name, value):
        raise self.ConstError(f"Can't rebind const {name}")


VT = ValueType()  # Setting中值的类型


class MsgHead_TT(Structure):
    _fields_ = [
        ("header_mark", c_byte * 2),
        ("from_who", c_byte),
        ("to_who", c_byte),
        ("msg_len", c_uint),
    ]


class MsgHead():
    """
    网络通信的消息头
    """

    def __init__(self, from_who=0, to_who=0, action_type=AT.AT_UNKNOWN):
        # self.header_mark = b'\x23\x23'  # 两个#号
        self.header_mark = 8995  # 两个#号
        self.msg_len = 0
        self.from_who = from_who
        self.to_who = to_who
        self.action_type = action_type
        # fmt 以为 format 规定以上四个参数的所占字节数
        self.fmt = "1H1H1B1B1B"

    def __dir__(self):
        # 定义类中哪些是需要发送的数据
        return ["header_mark", "msg_len", "from_who", "to_who", "action_type"]

    def decode(self, network_data, byteOrder='!'):
        """
        消息的解码，子类可以继承无需重写
        """
        members = [attr for attr in self.__dir__()
                   if not callable(getattr(self, attr))
                   and not attr.startswith("__")
                   and not attr.startswith("fmt")]
        # 获取当前实例化的对象大小（可能是当前类，也可能是它的子类）
        size = struct.Struct(self.fmt).size
        # 以下的 self.fmt 可能包含了子类的一部分，并非一定等于 __init__ 中的 self.fmt
        get_data = struct.unpack(byteOrder + self.fmt, network_data[: size])

        # 解析解码后的参数
        for (attr, value) in zip(members, get_data):
            setattr(self, attr, value)
        return size  # 返回解码完成的数据大小

    def encode(self, byteOrder='='):
        """
        消息的编码，子类可以继承可以不重写
        """
        # self.msg_len = struct.Struct(self.fmt).size - 7
        # 获取当前实例化的参数
        members = [attr for attr in self.__dir__() if not callable(getattr(self, attr))]
        # 解析得到所有的参数 一定要在self.msg_len赋值后操作
        params = [getattr(self, param) for param in members]
        return struct.pack(byteOrder + self.fmt, *params)


class SettingMsg(MsgHead):

    def __init__(self, action_type=AT.AT_SETTING_GET):
        MsgHead.__init__(self, MT.MODULE_TYPE_TOOL_SETTINGS, MT.MODULE_TYPE_CUBIC_SETTINGS, action_type)  # 一定要初始化父类
        # 以下数据直接使用空格隔开
        self.prefs_name = b'';
        self.key = b'';
        self.type = b'';
        self.value = b'';
        # self.fmt = self.fmt + "1B"

    def decode(self, network_data, byteOrder='!'):
        """
        消息的解码
        """
        size = super().decode(network_data, byteOrder)
        # 处理不定长的数据
        self.left_info = network_data[size:]
        print(self.left_info)
        # # 获取当前实例化的对象大小（可能是当前类，也可能是它的子类）
        # size = struct.Struct(self.fmt).size
        # # 以下的 self.fmt 可能包含了子类的一部分，并非一定等于 __init__ 中的 self.fmt
        # get_data = struct.unpack(byteOrder + self.fmt, network_data[: size])

        # # 解析解码后的参数
        # for (attr, value) in zip(members, get_data):
        #     setattr(self, attr, value)
        return size  # 返回解码完成的数据大小

    def encode(self, byteOrder='='):
        """
        消息的编码，子类可以继承可以不重写
        """
        info = self.prefs_name + b'\x00' + self.key + b'\x00' \
            + self.type + b'\x00' + self.value + b'\r\n'
        self.msg_len = struct.Struct(self.fmt).size + len(info)

        data = super().encode(byteOrder)
        data = data + info
        return data

    def __dir__(self):
        super_param = super().__dir__()
        return super_param


# 结构体转字典
def dump_dict(obj):
    info = {}
    # 通过_fields_获取每一个字段
    # 检查每个字段的类型，根据不同类型分别处理
    # 支持递归迭代
    for k, v in obj._fields_:
        av = getattr(obj, k)
        if type(v) == type(Structure):
            print(av)
            # av = av.dump_dict()
        elif type(v) == type(Array):
            av = cast(av, c_char_p).value.decode()
        else:
            pass
        info[k] = av
    return info
