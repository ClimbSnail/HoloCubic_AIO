# -*- coding: utf-8 -*-
################################################################################
#
# Author: ClimbSnail(HQ)
# original source is here.
#   https://github.com/ClimbSnail/HoloCubic_AIO_Tool
# 
#
################################################################################

from util.widget_base import EntryWithPlaceholder

import os
import time
import serial   # pip install pyserial
import tkinter as tk
import util.tkutils as tku
from tkinter import ttk
from tkinter import filedialog
from tool_esptoolpy import esptool
import util.massagehead as mh
import threading
import util.common as common
import requests
import re

STRGLO = ""  # 读取的数据
BOOL = True  # 读取标志位
VERSION_INFO_URL = "https://gitee.com/ClimbSnailQ/HoloCubic_AIO/blob/main/AIO_Firmware_PIO/src/common.h"
# VERSION_INFO_URL = "https://github.com/ClimbSnail/HoloCubic_AIO/blob/main/AIO_Firmware_PIO/src/common.h"

class DownloadDebug(object):
    """
    菜单栏类
    """

    def __init__(self, father, engine, lock=None):
        """
        DownloadDebug初始化
        :param father:父类窗口
        :param engine:引擎对象，用于推送与其他控件的请求
        :param lock:线程锁
        :return:None
        """
        self.__engine = engine  # 负责各个组件之间数据调度的引擎
        self.__father = father  # 保存父窗口
        self.ser = None  # 串口
        self.receive_thread = None  # 串口接收线程对象
        self.download_thread = None  # 下载线程对象
        self.progress_bar_thread = None  # 进度条线程对象
        self.clean_flash_thread = None # 清空flash动作线程对象

        # 连接器相关控件
        # 使用LabelFrame控件 框出连接相关的控件
        self.connor_grid_frame = tk.LabelFrame(self.__father, text="串口设置",
                                               labelanchor="nw", bg="white")
        # self.connor_grid_frame.place(anchor="ne", relx=1.0, rely=0.0)
        # self.connor_grid_frame.grid(row=0, column=1)
        self.connor_grid_frame.place(x=self.__father.winfo_width() + 10, y=10)
        self.create_com(self.connor_grid_frame)
        self.connor_grid_frame.update()

        # 连接器相关控件
        cur_dir = os.getcwd()
        # 固件下载框默认值
        self.__pre_down_param_list = [
            {"bin_addr": "0x1000", "bin_path": os.path.join(cur_dir, 'bootloader_qio_80m.bin'),
             "placeholder": "选择Bootloader的bin文件"},

            {"bin_addr": "0x8000", "bin_path": os.path.join(cur_dir, 'partitions.bin'),
             "placeholder": "选择partitions的bin文件"},

            {"bin_addr": "0xe000", "bin_path": os.path.join(cur_dir, 'boot_app0.bin'),
             "placeholder": "选择boot_app0的bin文件"},

            {"bin_addr": "0x10000", "bin_path": "",
             "placeholder": "选择user_data的bin文件(AIO固件必选项)"}
        ]
        # 使用LabelFrame控件 框出刷写的控件
        self.connor_firmware_frame = tk.LabelFrame(self.__father, text="固件刷写",
                                                   labelanchor="nw", bg="white")
        self.connor_firmware_frame.place(x=200, y=10)
        self.connor_firmware_frame.update()
        self.init_firmware(self.connor_firmware_frame)

        # 连接器相关控件
        # 使用LabelFrame控件 框出日志相关的控件
        self.connor_log_frame = tk.LabelFrame(self.__father, text="操作日志",
                                              labelanchor="nw", bg="white")
        self.connor_log_frame.place(x=685, y=10)
        self.connor_log_frame.update()
        self.init_log(self.connor_log_frame)

        # 连接器相关控件
        # 使用LabelFrame控件 框出连接相关的控件
        self.connor_info_frame = tk.LabelFrame(self.__father, text="串口接收",
                                               labelanchor="nw", bg="white")
        self.connor_info_frame.place(x=self.__father.winfo_width() + 10, y=240)
        self.connor_info_frame.update()
        self.init_serial_receive(self.connor_info_frame)

        self.display_version();
        self.get_version_thread = threading.Thread(target=self.display_version)
        self.get_version_thread.start()

    def api(self, action, param=None):
        """
        下载模块对外的api接口
        """
        if action == mh.A_CLOSE_UART:  # 关闭串口
            self.canle_download_firmware()
            self.m_connect_button["text"] == "关闭串口"
            self.com_connect()

    def display_version(self):
        self.m_version_info["state"] = tk.DISABLED
        try:
            response = requests.get(VERSION_INFO_URL, timeout=3) # , verify=False
            version_info = re.findall(r'AIO_VERSION \"\d{1,2}\.\d{1,2}\.\d{1,2}\"', response.text)
            self.m_version_var.set("v" + version_info[0].split("\"")[1])
        except Exception as err:
            print(err)
            self.m_version_var.set("未知")

    def init_firmware(self, father):
        """
        固件操作
        """
        border_padx = 5  # 两个控件的间距
        border_pady = 3  # 两行控件的间距

        firmware_num = 4
        firmware_frame = [None for cnt in range(firmware_num)]
        self.__firmware_enable_val = [None for cnt in range(firmware_num)]
        self.__firmware_enable = [None for cnt in range(firmware_num)]
        self.__firmware_addr_val = [None for cnt in range(firmware_num)]
        self.__firmware_addr_entry = [None for cnt in range(firmware_num)]
        self.__firmware_path_val = [None for cnt in range(firmware_num)]
        self.__firmware_path_entry = [None for cnt in range(firmware_num)]
        self.__firmware_choose_botton = [None for cnt in range(firmware_num)]

        for pos in range(firmware_num):
            firmware_frame[pos] = tk.Frame(father, bg=father["bg"])
            # 勾选框的键值对象
            self.__firmware_enable_val[pos] = tk.IntVar()
            self.__firmware_enable_val[pos].set(1)
            # 勾选框
            self.__firmware_enable[pos] = tk.Checkbutton(firmware_frame[pos], text="", bg=father["bg"],
                                                         variable=self.__firmware_enable_val[pos],
                                                         onvalue=1, offvalue=0, height=1,
                                                         width=1)
            self.__firmware_enable[pos].pack(side=tk.LEFT)
            # 创建地址输入框
            self.__firmware_addr_val[pos] = tk.StringVar()
            self.__firmware_addr_entry[pos] = tk.Entry(firmware_frame[pos], width=9,
                                                       highlightcolor="LightGrey",
                                                       textvariable=self.__firmware_addr_val[pos])

            self.__firmware_addr_val[pos].set(self.__pre_down_param_list[pos]["bin_addr"])
            self.__firmware_addr_entry[pos].pack(side=tk.LEFT, padx=border_padx)
            # 创建路径输入框
            self.__firmware_path_val[pos] = tk.StringVar()
            self.__firmware_path_entry[pos] = EntryWithPlaceholder(firmware_frame[pos], width=40,
                                                                   highlightcolor="LightGrey",
                                                                   placeholder=self.__pre_down_param_list[pos][
                                                                       "placeholder"],
                                                                   placeholder_color="grey",
                                                                   textvariable=self.__firmware_path_val[pos])

            self.__firmware_path_val[pos].set(self.__pre_down_param_list[pos]["bin_path"])
            self.__firmware_path_entry[pos].pack(side=tk.LEFT, padx=border_padx)
            self.__firmware_path_entry[pos].refresh()
            # 选择按钮
            self.__firmware_choose_botton[pos] = tk.Button(firmware_frame[pos], text="选择", fg='black',
                                                           command=lambda: self.choose_file(pos.copy()), width=6, height=1)

            self.__firmware_choose_botton[pos].pack(side=tk.RIGHT, fill=tk.X, padx=5)
            firmware_frame[pos].pack(side=tk.TOP, pady=border_pady)

        # 由于"选择"按钮中用的是循环，参数pos不生效
        self.__firmware_choose_botton[0].config(command=lambda: self.choose_file(0))
        self.__firmware_choose_botton[1].config(command=lambda: self.choose_file(1))
        self.__firmware_choose_botton[2].config(command=lambda: self.choose_file(2))
        self.__firmware_choose_botton[3].config(command=lambda: self.choose_file(3))
        
        # version_info_frame = tk.Frame(father, bg=father["bg"])
        # 版本信息
        # version_text = tk.Label(version_info_frame, text="AIO最新版本", bg=version_info_frame['bg'])
        # version_text.pack(side=tk.LEFT)
        # # 创建宽输入框
        # self.m_version_var = tk.StringVar()
        # self.m_version_info = EntryWithPlaceholder(version_info_frame, width=6, highlightcolor="LightGrey",
        #                                           placeholder="未知", placeholder_color="grey",
        #                                           textvariable=self.m_version_var)
        # self.m_version_info.pack(side=tk.LEFT, padx=5)
        # version_info_frame.pack(side=tk.TOP, pady=5)

        # botton组件
        botton_group_frame = tk.Frame(father, bg=father["bg"])

        version_text = tk.Label(botton_group_frame, text="AIO最新版本", bg=botton_group_frame['bg'])
        version_text.pack(side=tk.LEFT)
        # 创建宽输入框
        self.m_version_var = tk.StringVar()
        self.m_version_info = EntryWithPlaceholder(botton_group_frame, width=6, highlightcolor="LightGrey",
                                                  placeholder="未知", placeholder_color="grey",
                                                  textvariable=self.m_version_var)
        self.m_version_info.pack(side=tk.LEFT, padx=5)

        # 清空按钮
        self.m_clean_flash_botton = tk.Button(botton_group_frame, text="清空芯片", fg='black',
                                              command=self.clean_flash, width=8, height=1)
        self.m_clean_flash_botton.pack(side=tk.LEFT, fill=tk.X, padx=border_padx)
        # 下载按钮
        self.m_download_botton = tk.Button(botton_group_frame, text="刷写固件", fg='black',
                                           command=self.down_and_canle, width=8, height=1)

        self.m_download_botton.pack(side=tk.RIGHT, fill=tk.X, padx=0)
        botton_group_frame.pack(side=tk.TOP, pady=5)

        # 设置下载进度条组件
        progress_frame = tk.Frame(father, bg=father["bg"])
        self.progress_bar = tk.Canvas(progress_frame, width=450, height=15, bg="white")
        # 进度条的矩形框
        self.progress_bar_circle = self.progress_bar.create_rectangle(3, 3, 450, 14, outline="green", width=1)
        self.progress_bar_fill = self.progress_bar.create_rectangle(3, 3, 20, 14, outline="", width=1, fill="green")
        self.progress_bar.coords(self.progress_bar_fill, (3, 3, 0, 25))
        self.progress_bar.pack(side=tk.TOP, pady=0)
        progress_frame.pack(side=tk.TOP, pady=0)

    def schedule_display(self, all_time, update_interval):
        """
        进度条处理动画，原则上启动一个线程来执行本函数
        all_time：进度条的总时间(s)
        update_interval：更新时间间隔(s)
        """
        cycle_number = int(all_time / update_interval)
        self.print_log("all_time: " + str(all_time))
        for num in range(cycle_number - 1):
            self.progress_bar.coords(self.progress_bar_fill, (3, 3, (num / cycle_number) * 440, 14))
            self.__father.update()
            time.sleep(update_interval)

    def choose_file(self, num):
        """
        点击"选择"文件触发的函数
        :pos: 为触发”选择“按钮的编号
        :return:
        """
        # 打开文件对话框 获取文件路径
        # defaultextension 为选取保存类型中的拓展名为文件名
        # filetypes为文件拓展名
        filepath = filedialog.askopenfilename(
            title='选择一个bin文件',
            defaultextension=".espace",
            filetypes=[('BIN', '.bin .Bin')])
        if filepath == None or filepath == "":
            return None
        else:
            self.__firmware_path_entry[num].delete(0, tk.END)  # 清空文本框
            self.__firmware_path_entry[num].insert(tk.END, filepath)

    def clean_flash(self):
        """
        擦除闪存
        """
        def clean_func():
            self.m_clean_flash_botton["text"] = "清空中"
            self.m_connect_button["state"] = tk.DISABLED
            self.m_reboot_button["state"] = tk.DISABLED
            # self.m_clean_flash_botton["state"] = tk.DISABLED
            self.m_download_botton["state"] = tk.DISABLED
            self.print_log("清空芯片中.....")
            self.__father.update()

            cmd = ['erase_flash']
            esptool.main(cmd)
            self.print_log("清空芯片成功！")
            # 更新按钮状态
            self.m_clean_flash_botton["text"] = "清空芯片"
            self.m_connect_button["state"] = tk.NORMAL
            self.m_reboot_button["state"] = tk.NORMAL
            # self.m_clean_flash_botton["state"] = tk.NORMAL
            self.m_download_botton["state"] = tk.NORMAL
            self.__father.update()

        if self.m_clean_flash_botton["text"] == "清空芯片":
            self.clean_flash_thread = threading.Thread(target=clean_func,)
            self.clean_flash_thread.start()
        else:
            # 杀线程
            common._async_raise(self.clean_flash_thread)
            self.clean_flash_thread = None
            self.m_clean_flash_botton["text"] = "清空芯片"
            self.m_connect_button["state"] = tk.NORMAL
            self.m_reboot_button["state"] = tk.NORMAL
            # self.m_clean_flash_botton["state"] = tk.NORMAL
            self.m_download_botton["state"] = tk.NORMAL
            self.__father.update()

    def down_and_canle(self):
        """
        下载与取消按钮
        """
        if self.m_download_botton["text"] == "刷写固件":
            self.download_firmware()
        elif self.m_download_botton["text"] == "取消下载":
            self.canle_download_firmware()

    def canle_download_firmware(self):
        """
        取消下载固件
        """
        if self.download_thread != None:
            try:
                # 杀线程
                common._async_raise(self.download_thread)
                self.download_thread = None
            except Exception as err:
                print(err)

        self.m_download_botton["text"] = "刷写固件"
        self.m_connect_button["state"] = tk.NORMAL
        self.m_reboot_button["state"] = tk.NORMAL
        self.m_clean_flash_botton["state"] = tk.NORMAL
        self.m_download_botton["state"] = tk.NORMAL

        if self.progress_bar_thread != None:
            try:
                # 杀线程
                common._async_raise(self.progress_bar_thread)
                self.progress_bar_thread = None
            except Exception as err:
                print(err)

        # 复位进度条
        self.progress_bar.coords(self.progress_bar_fill, (3, 3, 0, 25))

    def download_firmware(self):
        """
        下载固件
        """
        self.m_download_botton["text"] = "取消下载"
        self.m_connect_button["state"] = tk.DISABLED
        self.m_reboot_button["state"] = tk.DISABLED
        self.m_clean_flash_botton["state"] = tk.DISABLED
        self.m_download_botton["state"] = tk.NORMAL
        self.print_log("刷写中.....")
        down_flag, param = self.get_download_param()
        if down_flag == "disable":
            self.print_log("参数错误，刷写终止！")
            self.canle_download_firmware()
            return None

        # cmd = ['--port', param["port"],
        #        '--baud', param["baud"],
        #        'write_flash', '-fm', 'dio', '-fs', '4MB'
        #        ]
        cmd = ['--port', param["port"],
               '--baud', param["baud"],
               'write_flash', '-fs', '4MB'
               ]

        all_time = 0  # 粗略认为连接并复位芯片需要0.5s钟
        speed = int(param["baud"])

        del param["port"]
        del param["baud"]

        for key, value in param.items():
            cmd.append(key)
            cmd.append(value)
            try:
                # 波特率中10或11个10个比特能传输一个字节，这里同意取10
                all_time = all_time + os.path.getsize(value) * 10 / speed
            except Exception as err:
                print(err)

        self.progress_bar_thread = threading.Thread(target=self.schedule_display,
                                                    args=(all_time, 0.1,))
        # 进度条进程要在下载进程之前启动（为了在下载失败时可以立即查杀进度条进程）
        self.download_thread = threading.Thread(target=self.down_action,
                                                args=(cmd,))
        self.progress_bar_thread.start()
        self.download_thread.start()

    def down_action(self, cmd):
        cmd_str = ' '.join(cmd)
        self.print_log("-" * 15)
        self.print_log(cmd_str)
        self.print_log("-" * 15)
        self.__father.update()

        ret = esptool.main(cmd)

        self.m_download_botton["text"] = "刷写固件"
        self.m_connect_button["state"] = tk.NORMAL
        self.m_reboot_button["state"] = tk.NORMAL
        self.m_clean_flash_botton["state"] = tk.NORMAL
        self.m_download_botton["state"] = tk.NORMAL

        if self.progress_bar_thread != None:
            try:
                # 杀线程
                common._async_raise(self.progress_bar_thread)
                self.progress_bar_thread = None
            except Exception as err:
                print(err)

        # 复位进度条
        self.progress_bar.coords(self.progress_bar_fill, (3, 3, 0, 25))

        self.print_log("刷写固件成功！")

    def init_log(self, father):
        """
        初始化日志打印框
        """
        info_width = 39
        info_height = 15

        # 滑动条
        self.m_log_scrollbar = tk.Scrollbar(father, width=12)
        # 信息文本框
        self.m_log = tk.Text(father, width=info_width,
                             height=info_height,
                             yscrollcommand=self.m_log_scrollbar.set,
                             state=tk.DISABLED)
        # 两个控件关联
        self.m_log_scrollbar.config(command=self.m_log.yview)
        self.m_log.pack(side=tk.LEFT, fill=tk.Y, padx=3, pady=3)

        # 清空按键
        m_clear = tk.Button(father, text="X", command=self.clear_log,
                            width=1, height=2, font=('Helvetica', '4'))
        m_clear.pack(side=tk.BOTTOM, fill=tk.X, pady=1)

        self.m_log_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

    def print_log(self, msg):
        msg = str(msg) + "\n"
        self.m_log.config(state=tk.NORMAL)
        self.m_log.insert(tk.END, msg)
        self.m_log.config(state=tk.DISABLED)
        self.m_log.yview_moveto(1)

    def clear_log(self):
        """
        清空日志按钮
        :return: None
        """
        self.m_log.config(state=tk.NORMAL)
        self.m_log.delete(1.0, tk.END)
        self.m_log.config(state=tk.DISABLED)

    def init_serial_receive(self, father):

        # father.update()
        info_width = 135
        info_height = 27

        # 滑动条
        self.m_scrollbar = tk.Scrollbar(father, width=12, orient="vertical")
        # 信息文本框
        self.m_msg = tk.Text(father, width=info_width,
                             height=info_height,
                             yscrollcommand=self.m_scrollbar.set,
                             state=tk.DISABLED)
        # 两个控件关联
        self.m_scrollbar.config(command=self.m_msg.yview)
        self.m_msg.pack(side=tk.LEFT, fill=tk.Y, padx=3, pady=3)

        # 清空按键
        self.m_clear = tk.Button(father, text="X", command=self.clear_msg,
                                 width=1, height=2, font=('Helvetica', '4'))
        self.m_clear.pack(side=tk.BOTTOM, fill=tk.X, pady=1)

        self.m_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

    def clear_msg(self):
        """
        清空日志按钮
        :return: None
        """
        self.m_msg.config(state=tk.NORMAL)
        self.m_msg.delete(1.0, tk.END)
        self.m_msg.config(state=tk.DISABLED)

    def create_com(self, father):
        """
        创建Comm相关控件
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
        self.m_com_select.pack(side=tk.RIGHT, padx=border_padx)
        com_frame.pack(side=tk.TOP, pady=5)

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
        self.m_baud_select.current(7)
        self.m_baud_select.pack(side=tk.RIGHT, padx=border_padx)
        baud_frame.pack(side=tk.TOP, pady=5)

        # 数据位
        data_bit_frame = tk.Frame(father, bg=father["bg"])
        self.m_data_bit_label = tk.Label(data_bit_frame, text="数据位",
                                         # font=self.my_ft1,
                                         bg=father['bg'])
        self.m_data_bit_label.pack(side=tk.LEFT, padx=border_padx)
        self.m_data_bit_select = ttk.Combobox(data_bit_frame, width=8, state='readonly')
        self.m_data_bit_select["value"] = ('5', '6', '7', '8')
        # 设置默认值，即默认下拉框中的内容
        self.m_data_bit_select.current(3)
        self.m_data_bit_select.pack(side=tk.RIGHT, padx=border_padx)
        data_bit_frame.pack(side=tk.TOP, pady=5)

        # 校验位
        check_bit_frame = tk.Frame(father, bg=father["bg"])
        self.m_check_bit_label = tk.Label(check_bit_frame, text="校验位",
                                          # font=self.my_ft1,
                                          bg=father['bg'])
        self.m_check_bit_label.pack(side=tk.LEFT, padx=border_padx)
        self.m_check_bit_select = ttk.Combobox(check_bit_frame, width=8, state='readonly')
        self.m_check_bit_select["value"] = ('无校验', '奇校验', '偶校验', '0校验', '1校验')
        # 设置默认值，即默认下拉框中的内容
        self.m_check_bit_select.current(0)
        self.m_check_bit_select.pack(side=tk.RIGHT, padx=border_padx)
        check_bit_frame.pack(side=tk.TOP, pady=5)

        # 停止位
        stop_bit_frame = tk.Frame(father, bg=father["bg"])
        self.m_stop_bit_label = tk.Label(stop_bit_frame, text="停止位",
                                         # font=self.my_ft1,
                                         bg=father['bg'])
        self.m_stop_bit_label.pack(side=tk.LEFT, padx=border_padx)
        self.m_stop_bit_select = ttk.Combobox(stop_bit_frame, width=8, state='readonly')
        self.m_stop_bit_select["value"] = ('1位', '1.5位', '2位')
        # 设置默认值，即默认下拉框中的内容
        self.m_stop_bit_select.current(0)
        self.m_stop_bit_select.pack(side=tk.RIGHT, padx=border_padx)
        stop_bit_frame.pack(side=tk.TOP, pady=5)

        # 启停按钮
        botton_frame = tk.Frame(father, bg=father["bg"])
        self.m_connect_button = tk.Button(botton_frame, text="打开串口", fg='black',
                                          command=self.com_connect, width=12, height=1)
        self.m_connect_button.pack(side=tk.LEFT, fill=tk.X, padx=5)
        # 重启按钮
        self.m_reboot_button = tk.Button(botton_frame, text="重启", fg='black',
                                         command=self.esp_reboot, width=8, height=1)
        self.m_reboot_button.pack(side=tk.RIGHT, fill=tk.X, padx=5)
        self.m_reboot_button["state"] = tk.NORMAL

        botton_frame.pack(side=tk.TOP, pady=5)

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

        if self.m_connect_button["text"] == "打开串口":

            down_flag, param = self.get_download_param()
            if self.ser != None:
                self.ser.close()  # 关闭串口
            self.ser = serial.Serial(param["port"], param["baud"], timeout=10)

            # 判断是否打开成功
            if self.ser.is_open:
                BOOL = True  # 读取标志位
                self.receive_thread = threading.Thread(target=self.read_data,
                                                       args=(self.ser,))
                self.receive_thread.start()

                self.m_connect_button["text"] = "关闭串口"
                self.m_com_select["state"] = tk.DISABLED
                self.m_baud_select["state"] = tk.DISABLED
                self.m_data_bit_select["state"] = tk.DISABLED
                self.m_check_bit_select["state"] = tk.DISABLED
                self.m_stop_bit_select["state"] = tk.DISABLED
                self.m_reboot_button["state"] = tk.DISABLED
                self.m_clean_flash_botton["state"] = tk.DISABLED
                self.m_download_botton["state"] = tk.DISABLED
        else:
            self.m_connect_button["text"] = "打开串口"
            self.m_com_select["state"] = tk.NORMAL
            self.m_baud_select["state"] = tk.NORMAL
            self.m_data_bit_select["state"] = tk.NORMAL
            self.m_check_bit_select["state"] = tk.NORMAL
            self.m_stop_bit_select["state"] = tk.NORMAL
            self.m_reboot_button["state"] = tk.NORMAL
            self.m_clean_flash_botton["state"] = tk.NORMAL
            self.m_download_botton["state"] = tk.NORMAL

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
        # 循环接收数据，此为死循环，可用线程实现
        self.print_log("Receive_thread start")
        while BOOL:
            if ser.in_waiting:
                STRGLO = ser.read(ser.in_waiting).decode("utf8")
                self.m_msg.config(state=tk.NORMAL)
                self.m_msg.insert(tk.END, STRGLO)
                self.m_msg.config(state=tk.DISABLED)
                self.m_msg.yview_moveto(1)
                time.sleep(0.1)

    def get_download_param(self):
        """
        获取下载参数
        """
        data_map = {}
        firmware_num = 4  # 四个下载项
        for pos in range(firmware_num):
            firmware_addr = self.__firmware_addr_val[pos].get().strip()
            firmware_path = self.__firmware_path_entry[pos].get().strip()
            if self.__firmware_enable_val[pos].get() == 1 and firmware_addr != "" and firmware_path != "":
                data_map[firmware_addr] = firmware_path

        if data_map == {}:  # 无下载内容
            down_flag = "disable"
        else:
            down_flag = "enable"

        data_map["port"] = self.m_com_select.get().strip()
        data_map["baud"] = self.m_baud_select.get().strip()
        if data_map["port"] == "" or data_map["baud"] == "":
            # 串口信息为空
            down_flag = "disable"

        return down_flag, data_map

    def esp_reboot(self):
        """
        重启芯片
        """
        self.m_connect_button["state"] = tk.DISABLED
        self.m_reboot_button["state"] = tk.DISABLED
        self.m_clean_flash_botton["state"] = tk.DISABLED
        self.m_download_botton["state"] = tk.DISABLED
        down_flag, param = self.get_download_param()
        self.print_log("已发送重启指令！")
        self.__father.update()
        # cmd = ['--port', param["port"], '--baud', param["baud"],
        #        '--after', 'hard_reset', "read_mac"]
        # esptool.main(cmd)
        
        port = serial.Serial(param["port"], param["baud"], timeout=10)
        port.setRTS(True)  # EN->LOW
        port.setDTR(port.dtr)
        time.sleep(0.05)
        port.setRTS(False)
        port.setDTR(port.dtr)
        port.close()  # 关闭串口
        del port
        self.m_connect_button["state"] = tk.NORMAL
        self.m_reboot_button["state"] = tk.NORMAL
        self.m_clean_flash_botton["state"] = tk.NORMAL
        self.m_download_botton["state"] = tk.NORMAL

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
        if self.download_thread != None:
            # 杀线程
            common._async_raise(self.download_thread)
        if self.progress_bar_thread != None:
            # 杀线程
            common._async_raise(self.progress_bar_thread)
