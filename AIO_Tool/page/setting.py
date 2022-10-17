# -*- coding: utf-8 -*-
################################################################################
#
# Author: ClimbSnail(HQ)
# original source is here.
#   https://github.com/ClimbSnail/HoloCubic_AIO_Tool
# 
#
################################################################################

import tkinter as tk
import util.tkutils as tku
from tkinter import ttk
import json
import codecs
import threading
import util.common as common
import util.massagehead as mh
import serial   # pip install pyserial
import time
import struct
import traceback


class Setting(object):
    """
    参数设置类
    """

    def __init__(self, father, engine, lock=None):
        """
        DownloadDebug初始化
        :param father:父类窗口
        :param engine:引擎对象，用于推送与其他控件的请求
        :param lock:线程锁
        :return:None
        """
        self.m_engine = engine  # 负责各个组件之间数据调度的引擎
        self.__father = father  # 保存父窗口
        self.cfg_name = "cubictool.json"  # 配置文件路径
        self.data_info = None   # 存放数据的
        self.receive_thread = None # 串口接收线程
        self.ser = None    # 串口对象

        fp = codecs.open(self.cfg_name, "r", "utf8")
        self.data_info = json.load(fp)
        fp.close()
        # print(self.data_info)

        # 创建配置文件
        # self.createConfig(self.cfg_name)

        # 串口连接
        self.uart_father = tk.Frame(self.__father, bg=self.__father["bg"])
        self.uart_father.place(x=self.__father.winfo_width() + 250, y=10)
        self.connect_uart(self.uart_father)
        self.uart_father.update()

        # wifi设置框
        # 使用LabelFrame控件 框出连接相关的控件
        self.wifi_grid_frame = tk.LabelFrame(self.__father, text="WIFI设置",
                                               labelanchor="nw", bg="white")
        # self.wifi_grid_frame.place(anchor="ne", relx=1.0, rely=0.0)
        # self.wifi_grid_frame.grid(row=0, column=1)
        self.wifi_grid_frame.place(x=self.__father.winfo_width() + 10, y=10)
        self.create_wifi(self.wifi_grid_frame)
        self.wifi_grid_frame.update()
    
    def createConfig(self, filename):
        """
        创建配置文件
        :param filename: 文件路径
        :return: True/False
        """
        # 数据格式 {"ssid_1": {"namespace":"sys", "type":"String", "info":"提示信息"}}
        data = {"ssid_1": {"name":"ssid_1", "namespace":"sys", "type":"String", "info":"提示信息"},
                "password_1": {"name":"password_1", "namespace":"sys", "type":"String", "info":"提示信息"},
                "ssid_2": {"name":"ssid_2", "namespace":"sys", "type":"String", "info":"提示信息"},
                "password_2": {"name":"password_2", "namespace":"sys", "type":"String", "info":"提示信息"},

                "backLight": {"name":"backLight", "namespace":"sys", "type":"UChar", "info":"提示信息"},
                "rotation": {"name":"rotation", "namespace":"sys", "type":"UChar", "info":"提示信息"},
                "auto_mpu": {"name":"auto_mpu", "namespace":"sys", "type":"UChar", "info":"提示信息"},

                "cityname": {"name":"cityname", "namespace":"zhixin", "type":"String", "info":"提示信息"},
                "language": {"name":"language", "namespace":"zhixin", "type":"String", "info":"提示信息"},
                "weather_key": {"name":"weather_key", "namespace":"zhixin", "type":"String", "info":"提示信息"},

                "tianqi_aid": {"name":"tianqi_aid", "namespace":"tianqi", "type":"String", "info":"提示信息"},
                "tianqi_as": {"name":"tianqi_as", "namespace":"tianqi", "type":"String", "info":"提示信息"},
                "tianqi_addr": {"name":"tianqi_addr", "namespace":"tianqi", "type":"String", "info":"提示信息"},

                "bili_uid": {"name":"bili_uid", "namespace":"other", "type":"String", "info":"提示信息"}
                }
        fp = codecs.open(self.cfg_name, "w", "utf8")
        json.dump(data, fp, indent=2)
        fp.close()
        return True

    def connect_uart(self, father):
        """
        创建串口连接控件
        :param father: 父类窗口
        :return: None
        """
        # 获取可用COM口名字
        com_obj_list = list(serial.tools.list_ports.comports())
        com_tuple = [com_obj[0] for com_obj in com_obj_list]
        if len(com_tuple) == 0:
            com_tuple = [""]

        border_padx = 15  # 两个控件的间距
        # 窗口
        com_frame = tk.Frame(father, bg=father["bg"])
        self.m_com_label = tk.Label(com_frame, text="端口号",
                                    # font=self.my_ft1,
                                    bg=father['bg'])
        self.m_com_label.pack(side=tk.LEFT, padx=border_padx)

        self.m_com_select = ttk.Combobox(com_frame, width=8, state='readonly')
        self.m_com_select["value"] = tuple(com_tuple)
        self.m_com_select.bind("<FocusOut>", self.com_pull_down)

        # 设置默认值，即默认下拉框中的内容
        self.m_com_select.current(0)
        self.m_com_select.pack(side=tk.LEFT, padx=border_padx)
        com_frame.pack(side=tk.LEFT, pady=5)

        # 波特率
        baud_frame = tk.Frame(father, bg=father["bg"])
        self.m_baud_label = tk.Label(baud_frame, text="波特率",
                                     # font=self.my_ft1,
                                     bg=father['bg'])
        self.m_baud_label.pack(side=tk.LEFT, padx=border_padx)
        self.m_baud_select = ttk.Combobox(baud_frame, width=8, state='readonly')
        self.m_baud_select["value"] = ('9600', '38400', '57600', '115200',
                                       '230400', '460800', '576000', '921600', '1152000')
        # 设置默认值，即默认下拉框中的内容
        self.m_baud_select.current(3)
        self.m_baud_select.pack(side=tk.LEFT, padx=border_padx)
        baud_frame.pack(side=tk.LEFT, pady=5)

        # 启停按钮
        botton_frame = tk.Frame(father, bg=father["bg"])
        self.m_connect_button = tk.Button(botton_frame, text="打开串口", fg='black',
                                          command=self.com_connect, width=12, height=1)
        self.m_connect_button.pack(side=tk.LEFT, fill=tk.X, padx=5)

        botton_frame.pack(side=tk.LEFT, pady=5)


    def com_pull_down(self, event):
        """
        comm口下拉框被点击的时候 触发端口扫描
        """
        # 获取可用COM口名字
        com_obj_list = list(serial.tools.list_ports.comports())
        com_tuple = [com_obj[0] for com_obj in com_obj_list]
        if len(com_tuple) == 0:
            com_tuple = [""]
        # 获取当前下拉框的值
        choose_com = self.m_com_select.get()
        choose_index = 0
        if choose_com in com_tuple:
            choose_index = com_tuple.index(choose_com)
        # 设置下拉框里的列表
        self.m_com_select["value"] = tuple(com_tuple)
        # 更改下拉框中的内容
        self.m_com_select.current(choose_index)

    def com_connect(self):
        global STRGLO, BOOL

        # 先关闭下载页的串口
        self.m_engine.OnThreadMessage(mh.M_SETTING, mh.M_DOWNLOAD_DEBUG, mh.A_CLOSE_UART, None)
        
        if self.m_connect_button["text"] == "打开串口":

            port = self.m_com_select.get().strip()
            baud = self.m_baud_select.get().strip()
            if self.ser != None:
                self.ser.close()  # 关闭串口
            self.ser = serial.Serial(port, baud, timeout=10)

            # 判断是否打开成功
            if self.ser.is_open:
                BOOL = True  # 读取标志位
                self.receive_thread = threading.Thread(target=self.read_data,
                                                    args=(self.ser,))
                self.receive_thread.start()

                self.m_connect_button["text"] = "关闭串口"
                self.m_com_select["state"] = tk.DISABLED
                self.m_baud_select["state"] = tk.DISABLED
        else:
            self.m_connect_button["text"] = "打开串口"
            self.m_com_select["state"] = tk.NORMAL
            self.m_baud_select["state"] = tk.NORMAL

            if self.ser != None:
                self.ser.close()  # 关闭串口
                del self.ser
                self.ser = None
                # 杀线程
                common._async_raise(self.receive_thread)
                self.receive_thread = None
                self.print_log("Receive_thread stop")
                STRGLO = ""  # 读取的数据
                BOOL = False  # 读取标志位
        
    
    def read_data(self, ser):
        global STRGLO, BOOL
        pass
        # 循环接收数据，此为死循环，可用线程实现
        self.print_log("Receive_thread start")
        while BOOL:
            if ser.in_waiting:
                STRGLO = ser.read(ser.in_waiting)
                print("Receive---> ", STRGLO)
                time.sleep(0.2)
    
    def print_log(self, msg):
        print(msg)
        self.set_param("ssid_1", "12345678")

    
    # 帧格式为 
    # 帧头0x2323（2字节）+ 帧长度（2字节）+ 发送者（2字节）
    #                  + 接收者（2字节）+ 消息类型（2字节）
    #                   + 消息数据（帧长度-10）+ 帧尾/r/n（2字节）
    def set_param(self, key, value):
        """
        设置参数
        :param key: 设置的key
        :param value: 值
        :return: None
        """
        value_type = {"String": mh.VT.VALUE_TYPE_STRING, 
                    "UChar": mh.VT.VALUE_TYPE_UCHAR, 
                    "Int": mh.VT.VALUE_TYPE_INT
                    }
        try:
            info = self.data_info[key]
            print(info)
            send_data = mh.SettingMsg()
            send_data.action_type = mh.AT.AT_SETTING_SET
            send_data.prefs_name = bytes(info["namespace"], encoding='utf8')
            send_data.key = bytes(key, encoding='utf8')
            send_data.type = value_type[info["type"]].to_bytes(1, byteorder='little', signed=True)
            print(send_data.type)
            send_data.value = bytes(value, encoding='utf8')
            print(send_data.encode())
            if self.ser != None:
                self.ser.write(send_data.encode())
        except Exception as err:
            print(str(traceback.format_exc()))
            print(err)
        

    # 帧格式为 
    # 帧头0x2323（2字节）+ 帧长度（2字节）+ 发送者（2字节）
    #                  + 接收者（2字节）+ 消息类型（2字节）
    #                   + 消息数据（帧长度-10）+ 帧尾/r/n（2字节）
    def get_param(self, key):
        """
        获取参数
        :param key: 
        :return: string(value)
        """
        value_type = {"String": mh.VT.VALUE_TYPE_STRING, 
                    "UChar": mh.VT.VALUE_TYPE_UCHAR, 
                    "Int": mh.VT.VALUE_TYPE_INT
                    }
        try:
            info = self.data_info[key]
            print(info)
            send_data = mh.SettingMsg()
            send_data.action_type = mh.AT.AT_SETTING_GET
            send_data.prefs_name = bytes(info["namespace"], encoding='utf8')
            send_data.key = bytes(key, encoding='utf8')
            send_data.type = value_type[info["type"]].to_bytes(1, byteorder='little', signed=True)
            print(send_data.type)
            print("send_data --> ", send_data.encode('>'))
            if self.ser != None:
                self.ser.write(send_data.encode('>'))
        except Exception as err:
            print(str(traceback.format_exc()))
            print(err)
        
        # print("get_param--> ")
        # if self.ser != None and self.ser.in_waiting:
        #     STRGLO = self.ser.read(self.ser.in_waiting)
        #     print("read---> ", STRGLO)
            

    def create_wifi(self, father):
        """
        创建WIFI控件
        :param father: 父类窗口
        :return: None
        """
        # 选择按钮
        get_botton = tk.Button(father, text="获取", fg='black',
                                command=lambda: self.get_param("ssid"),
                                width=6, height=1)
        get_botton.pack(side=tk.RIGHT, fill=tk.X, padx=5)

    def __del__(self):
        """"
        资源释放 查杀线程
        """
        if self.ser != None:
            self.ser.close()  # 关闭串口
            self.ser = None
        if self.receive_thread != None:
            # 杀线程
            common._async_raise(self.receive_thread)
        
        
