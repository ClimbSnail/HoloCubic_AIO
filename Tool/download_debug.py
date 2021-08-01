import os
import serial
import tkinter as tk
import tkutils as tku
from tkinter import ttk
from tool_esptoolpy import esptool
import threading
import ctypes
import inspect

STRGLO = ""  # 读取的数据
BOOL = True  # 读取标志位


def _async_raise(thread):
    """
    释放进程
    :param thread: 进程对象
    :param exctype:
    :return:
    """
    try:
        tid = thread.ident
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


class EntryWithPlaceholder(tk.Entry):
    def __init__(self, master=None, *, placeholder="PLACEHOLDER",
                 placeholder_color='grey', **attribute):
        super().__init__(master, attribute)

        self.placeholder = placeholder.strip()
        self.placeholder_color = placeholder_color
        self.default_fg_color = self['fg']

        self.bind("<FocusIn>", self.foc_in)
        self.bind("<FocusOut>", self.foc_out)

    def refresh(self):
        """
        若输入框带有其他默认值（非提示词）时 需要手动调用刷新
        """
        if not self.get():
            self.insert(0, self.placeholder)
            self['fg'] = self.placeholder_color

    def foc_in(self, *args):
        if self.placeholder == super().get():
            if self['fg'] == self.placeholder_color:
                self.delete('0', 'end')
                self['fg'] = self.default_fg_color

    def foc_out(self, *args):
        self.refresh()


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
        self.m_engine = engine  # 负责各个组件之间数据调度的引擎
        self.m_father = father  # 保存父窗口
        self.ser = None  # 串口

        # 连接器相关控件
        # 使用LabelFrame控件 框出连接相关的控件
        self.connor_grid_frame = tk.LabelFrame(self.m_father, text="串口设置",
                                               labelanchor="nw", bg="white")
        # self.connor_grid_frame.place(anchor="ne", relx=1.0, rely=0.0)
        # self.connor_grid_frame.grid(row=0, column=1)
        self.connor_grid_frame.place(x=self.m_father.winfo_width() + 10, y=10)
        self.connor_grid_frame.update()
        self.create_com(self.connor_grid_frame)

        # 连接器相关控件
        # 使用LabelFrame控件 框出连接相关的控件
        self.connor_firmware_frame = tk.LabelFrame(self.m_father, text="固件刷写",
                                                   labelanchor="nw", bg="white")
        self.connor_firmware_frame.place(x=200, y=10)
        self.connor_firmware_frame.update()
        self.init_firmware(self.connor_firmware_frame)

        # 连接器相关控件
        # 使用LabelFrame控件 框出连接相关的控件
        self.connor_log_frame = tk.LabelFrame(self.m_father, text="操作日志",
                                              labelanchor="nw", bg="white")
        self.connor_log_frame.place(x=685, y=10)
        self.connor_log_frame.update()
        self.init_log(self.connor_log_frame)

        # 连接器相关控件
        # 使用LabelFrame控件 框出连接相关的控件
        self.connor_info_frame = tk.LabelFrame(self.m_father, text="串口接收",
                                               labelanchor="nw", bg="white")
        self.connor_info_frame.place(x=self.m_father.winfo_width() + 10, y=240)
        self.connor_info_frame.update()
        self.init_serial_receive(self.connor_info_frame)

    def init_firmware(self, father):
        """
        固件操作
        """
        border_padx = 10  # 两个控件的间距

        # bootloader文件
        boot_frame = tk.Frame(father, bg=father["bg"])
        # 创建地址输入框
        self.m_boot_addr_val = tk.StringVar()
        self.m_boot_addr_entry = tk.Entry(boot_frame, width=10,
                                          highlightcolor="LightGrey", textvariable=self.m_boot_addr_val)
        self.m_boot_addr_entry.pack(side=tk.LEFT, padx=border_padx)
        # 创建路径输入框
        self.m_boot_path_val = tk.StringVar()
        self.m_boot_path_entry = EntryWithPlaceholder(boot_frame, width=40, highlightcolor="LightGrey",
                                                      placeholder="选择Bootloader的bin文件", placeholder_color="grey",
                                                      textvariable=self.m_boot_path_val)
        self.m_boot_path_entry.pack(side=tk.LEFT, padx=border_padx)
        # 原视频输入按钮
        self.boot_choose_botton = tk.Button(boot_frame, text="选择bin", fg='black',
                                            command=self.choose_boot_file, width=8, height=1)

        self.boot_choose_botton.pack(side=tk.RIGHT, fill=tk.X, padx=5)
        boot_frame.pack(side=tk.TOP, pady=5)

        # partitions文件
        partitions_frame = tk.Frame(father, bg=father["bg"])
        # 创建地址输入框
        self.m_partitions_addr_val = tk.StringVar()
        self.m_partitions_addr_entry = tk.Entry(partitions_frame, width=10, highlightcolor="LightGrey",
                                                textvariable=self.m_partitions_addr_val)
        self.m_partitions_addr_entry.pack(side=tk.LEFT, padx=border_padx)
        # 创建路径输入框
        self.m_partitions_path_val = tk.StringVar()
        self.m_partitions_path_entry = EntryWithPlaceholder(partitions_frame, width=40, highlightcolor="LightGrey",
                                                            placeholder="选择partitions的bin文件", placeholder_color="grey",
                                                            textvariable=self.m_partitions_path_val)
        self.m_partitions_path_entry.pack(side=tk.LEFT, padx=border_padx)
        # 原视频输入按钮
        self.partitions_choose_botton = tk.Button(partitions_frame, text="选择bin", fg='black',
                                                  command=self.choose_partitions_file, width=8, height=1)

        self.partitions_choose_botton.pack(side=tk.RIGHT, fill=tk.X, padx=5)
        partitions_frame.pack(side=tk.TOP, pady=5)

        # boot_app0文件
        boot_app0_frame = tk.Frame(father, bg=father["bg"])
        # 创建地址输入框
        self.m_boot_app0_addr_val = tk.StringVar()
        self.m_boot_app0_addr_entry = tk.Entry(boot_app0_frame, width=10,
                                               highlightcolor="LightGrey", textvariable=self.m_boot_app0_addr_val)
        self.m_boot_app0_addr_entry.pack(side=tk.LEFT, padx=border_padx)
        # 创建路径输入框
        self.m_boot_app0_path_val = tk.StringVar()
        self.m_boot_app0_path_entry = EntryWithPlaceholder(boot_app0_frame, width=40, highlightcolor="LightGrey",
                                                           placeholder="选择boot_app0的bin文件(核心固件)",
                                                           placeholder_color="grey",
                                                           textvariable=self.m_boot_app0_path_val)
        self.m_boot_app0_path_entry.pack(side=tk.LEFT, padx=border_padx)
        # 原视频输入按钮
        self.boot_app0_choose_botton = tk.Button(boot_app0_frame, text="选择bin", fg='black',
                                                 command=self.choose_boot_app0_file, width=8, height=1)

        self.boot_app0_choose_botton.pack(side=tk.RIGHT, fill=tk.X, padx=5)
        boot_app0_frame.pack(side=tk.TOP, pady=5)

        # user_data文件
        user_data_frame = tk.Frame(father, bg=father["bg"])
        # 创建地址输入框
        self.m_user_data_addr_val = tk.StringVar()
        self.m_user_data_addr_entry = tk.Entry(user_data_frame, width=10, highlightcolor="LightGrey",
                                               textvariable=self.m_user_data_addr_val)
        self.m_user_data_addr_entry.pack(side=tk.LEFT, padx=border_padx)
        # 创建路径输入框
        self.m_user_data_path_val = tk.StringVar()
        self.m_user_data_path_entry = EntryWithPlaceholder(user_data_frame, width=40, highlightcolor="LightGrey",
                                                           placeholder="选择user_data的bin文件", placeholder_color="grey",
                                                           textvariable=self.m_user_data_path_val)
        self.m_user_data_path_entry.pack(side=tk.LEFT, padx=border_padx)
        # 原视频输入按钮
        self.user_data_choose_botton = tk.Button(user_data_frame, text="选择bin", fg='black',
                                                 command=self.choose_user_data_file, width=8, height=1)

        self.user_data_choose_botton.pack(side=tk.RIGHT, fill=tk.X, padx=5)
        user_data_frame.pack(side=tk.TOP, pady=5)

        # botton组件
        botton_group_frame = tk.Frame(father, bg=father["bg"])
        # 清空按钮
        self.m_clean_flash_botton = tk.Button(botton_group_frame, text="清空芯片", fg='black',
                                              command=self.clean_flash, width=8, height=1)

        self.m_clean_flash_botton.pack(side=tk.LEFT, fill=tk.X, padx=border_padx)
        # 下载按钮
        self.m_download_botton = tk.Button(botton_group_frame, text="刷写固件", fg='black',
                                           command=self.download_firmware, width=8, height=1)

        self.m_download_botton.pack(side=tk.RIGHT, fill=tk.X, padx=5)
        botton_group_frame.pack(side=tk.TOP, pady=5)

        # 设置默认值
        self.default_firmware()

    def choose_bin_file(self, event):
        """
        点击"bin"文件触发的函数
        :return:
        """
        widget = event.widget  # 当前的组件
        value = widget.get()  # 选中的值
        print(value)

    def choose_boot_file(self):
        """
        点击"boot0"文件触发的函数
        :return:
        """
        # 打开文件对话框 获取文件路径
        # defaultextension 为选取保存类型中的拓展名为文件名
        # filetypes为文件拓展名
        filepath = tk.filedialog.askopenfilename(
            title='选择一个bin文件',
            defaultextension=".espace",
            filetypes=[('BIN', '.bin .Bin')])
        if filepath == None or filepath == "":
            return None
        else:
            self.m_boot_path_entry.delete(0, tk.END)  # 清空文本框
            self.m_boot_path_entry.insert(tk.END, filepath)

    def choose_partitions_file(self):
        """
        点击"partitions"文件触发的函数
        :return:
        """
        # 打开文件对话框 获取文件路径
        # defaultextension 为选取保存类型中的拓展名为文件名
        # filetypes为文件拓展名
        filepath = tk.filedialog.askopenfilename(
            title='选择一个bin文件',
            defaultextension=".espace",
            filetypes=[('BIN', '.bin .Bin')])
        if filepath == None or filepath == "":
            return None
        else:
            self.m_partitions_path_entry.delete(0, tk.END)  # 清空文本框
            self.m_partitions_path_entry.insert(tk.END, filepath)

    def choose_boot_app0_file(self):
        """
        点击"boot0"文件触发的函数
        :return:
        """
        # 打开文件对话框 获取文件路径
        # defaultextension 为选取保存类型中的拓展名为文件名
        # filetypes为文件拓展名
        filepath = tk.filedialog.askopenfilename(
            title='选择一个bin文件',
            defaultextension=".espace",
            filetypes=[('BIN', '.bin .Bin')])
        if filepath == None or filepath == "":
            return None
        else:
            self.m_boot_app0_path_entry.delete(0, tk.END)  # 清空文本框
            self.m_boot_app0_path_entry.insert(tk.END, filepath)

    def choose_user_data_file(self):
        """
        点击"user_data"文件触发的函数
        :return:
        """
        # 打开文件对话框 获取文件路径
        # defaultextension 为选取保存类型中的拓展名为文件名
        # filetypes为文件拓展名
        filepath = tk.filedialog.askopenfilename(
            title='选择一个bin文件',
            defaultextension=".espace",
            filetypes=[('BIN', '.bin .Bin')])
        if filepath == None or filepath == "":
            return None
        else:
            self.m_user_data_path_entry.delete(0, tk.END)  # 清空文本框
            self.m_user_data_path_entry.insert(tk.END, filepath)

    def clean_flash(self):
        """
        擦除闪存
        """
        self.m_clean_flash_botton["text"] = "清空中"
        self.m_clean_flash_botton["state"] = tk.DISABLED
        self.print_log("清空芯片中.....")
        self.m_father.update()
        cmd = ['erase_flash']
        esptool.main(cmd)
        self.m_clean_flash_botton["text"] = "清空芯片"
        self.m_clean_flash_botton["state"] = tk.NORMAL
        self.print_log("清空芯片成功！")

    def download_firmware(self):
        """
        下载固件
        """
        self.m_download_botton["text"] = "刷写中"
        self.m_download_botton["state"] = tk.DISABLED
        self.print_log("刷写中.....")
        param = self.get_download_param()
        cmd = ['--port', param["port"],
               '--baud', param["baud"],
               'write_flash', '-fm', 'dio', '-fs', '4MB',
               param["bootloader_addr"], param["bootloader_path"],
               param["partitions_addr"], param["partitions_path"],
               param["boot_app0_addr"], param["boot_app0_path"],
               param["user_data_addr"], param["user_data_path"]
               ]

        run_thread = threading.Thread(target=self.down,
                                        args=(cmd, ))
        run_thread.start()

    def down(self, cmd):

        cmd_str = ' '.join(cmd)
        self.print_log("-" * 15)
        self.print_log(cmd_str)
        self.print_log("-" * 15)
        self.m_father.update()

        esptool.main(cmd)
        self.m_download_botton["text"] = "刷写固件"
        self.m_download_botton["state"] = tk.NORMAL
        self.print_log("刷写固件成功！")

    def default_firmware(self):
        """
        默认的固件烧写地址
        """
        cur_dir = os.getcwd()

        self.m_boot_addr_val.set('0x1000')
        self.m_boot_path_val.set(os.path.join(cur_dir, 'bootloader_dio_40m.bin'))
        self.m_boot_path_entry.refresh()

        self.m_partitions_addr_val.set('0x8000')
        self.m_partitions_path_val.set(os.path.join(cur_dir, 'partitions.bin'))
        self.m_partitions_path_entry.refresh()

        # self.m_partitions_addr_entry.delete(0, tk.END)   # 清空文本框
        # self.m_partitions_addr_entry.insert(tk.END, '0x8000')
        # self.m_partitions_path_entry.delete(0, tk.END)   # 清空文本框
        # self.m_partitions_path_entry.insert(tk.END, "./partitions.bin")

        self.m_boot_app0_addr_val.set('0xe000')
        self.m_boot_app0_path_val.set(os.path.join(cur_dir, 'boot_app0.bin'))
        self.m_boot_app0_path_entry.refresh()

        self.m_user_data_addr_val.set('0x10000')
        # self.m_user_data_path_val.set(os.path.join(cur_dir, 'bootloader_dio_40m.bin'))
        self.m_user_data_path_entry.refresh()

        # self.m_user_data_addr_entry.delete(0, tk.END)   # 清空文本框
        # self.m_user_data_addr_entry.insert(tk.END, '0x10000')
        # self.m_user_data_path_entry.delete(0, tk.END)   # 清空文本框
        # self.m_user_data_path_entry.insert(tk.END, "")

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
                             height=info_height, yscrollcommand=self.m_log_scrollbar.set,
                             state=tk.DISABLED)
        # 两个控件关联
        self.m_log_scrollbar.config(command=self.m_log.yview)
        self.m_log.pack(side=tk.LEFT, fill=tk.Y, padx=3, pady=3)

        # 清空按键
        m_clear = tk.Button(father, text="X", command=self.log_clear,
                            width=1, height=2, font=('Helvetica', '4'))
        m_clear.pack(side=tk.BOTTOM, fill=tk.X, pady=1)

        self.m_log_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

    def print_log(self, msg):
        msg = msg + "\n"
        self.m_log.config(state=tk.NORMAL)
        self.m_log.insert(tk.END, msg)
        self.m_log.config(state=tk.DISABLED)

    def log_clear(self):
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
        self.m_scrollbar = tk.Scrollbar(father, width=12)
        # 信息文本框
        self.m_msg = tk.Text(father, width=info_width,
                             height=info_height, yscrollcommand=self.m_scrollbar.set,
                             state=tk.DISABLED)
        # 两个控件关联
        self.m_scrollbar.config(command=self.m_msg.yview)
        self.m_msg.pack(side=tk.LEFT, fill=tk.Y, padx=3, pady=3)

        # 清空按键
        self.m_clear = tk.Button(father, text="X", command=self.msg_clear,
                                 width=1, height=2, font=('Helvetica', '4'))
        self.m_clear.pack(side=tk.BOTTOM, fill=tk.X, pady=1)

        self.m_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

    def msg_clear(self):
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
        # self.m_com_select.bind('<<ComboboxSelected>>', self.com_choose)
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
        self.m_connect = tk.Button(botton_frame, text="打开串口", fg='black',
                                   command=self.com_connect, width=12, height=1)
        self.m_connect.pack(side=tk.LEFT, fill=tk.X, padx=5)
        # 重启按钮
        self.m_reboot = tk.Button(botton_frame, text="重启", fg='black',
                                  command=self.esp_reboot, width=8, height=1)
        self.m_reboot.pack(side=tk.RIGHT, fill=tk.X, padx=5)
        self.m_reboot["state"] = tk.NORMAL

        botton_frame.pack(side=tk.TOP, pady=5)

    def com_choose(self, event):
        """"
        端口被选中的时候 触发
        """
        widget = event.widget  # 当前的组件
        value = widget.get()  # 选中的值
        print(value)

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

        if self.m_connect["text"] == "打开串口":

            param = self.get_download_param()
            if self.ser != None:
                self.ser.close()  # 关闭串口
            self.ser = serial.Serial(param["port"], param["baud"], timeout=10)
    
            # 判断是否打开成功
            if self.ser.is_open:
                self.run_thread = threading.Thread(target=self.read_data,
                                                args=(self.ser, ))
                self.run_thread.start()
                BOOL = True  # 读取标志位

                self.m_connect["text"] = "关闭串口"
                self.m_com_select["state"] = tk.DISABLED
                self.m_baud_select["state"] = tk.DISABLED
                self.m_data_bit_select["state"] = tk.DISABLED
                self.m_check_bit_select["state"] = tk.DISABLED
                self.m_stop_bit_select["state"] = tk.DISABLED
                self.m_reboot["state"] = tk.DISABLED
        else:
            self.m_connect["text"] = "打开串口"
            self.m_com_select["state"] = tk.NORMAL
            self.m_baud_select["state"] = tk.NORMAL
            self.m_data_bit_select["state"] = tk.NORMAL
            self.m_check_bit_select["state"] = tk.NORMAL
            self.m_stop_bit_select["state"] = tk.NORMAL
            self.m_reboot["state"] = tk.NORMAL

            if self.ser != None:
                self.ser.close()  # 关闭串口
                del self.ser
                self.ser = None
                # 杀线程
                _async_raise(self.run_thread)
                self.run_thread = None
                print("self.run_thread stop")
                STRGLO = ""  # 读取的数据
                BOOL = False  # 读取标志位

    def read_data(self, ser):
        global STRGLO, BOOL
        # 循环接收数据，此为死循环，可用线程实现
        print("self.run_thread start")
        while BOOL:
            if ser.in_waiting:
                STRGLO = ser.read(ser.in_waiting).decode("utf8")
                self.m_msg.config(state=tk.NORMAL)
                self.m_msg.insert(tk.END, STRGLO)
                self.m_msg.config(state=tk.DISABLED)

    def get_download_param(self):
        """
        获取下载参数
        """
        return {
            "port": self.m_com_select.get(),
            "baud": self.m_baud_select.get(),

            "bootloader_addr": self.m_boot_addr_val.get(),
            "bootloader_path": self.m_boot_path_val.get(),

            "partitions_addr": self.m_partitions_addr_val.get(),
            "partitions_path": self.m_partitions_path_val.get(),

            "boot_app0_addr": self.m_boot_app0_addr_val.get(),
            "boot_app0_path": self.m_boot_app0_path_val.get(),

            "user_data_addr": self.m_user_data_addr_val.get(),
            "user_data_path": self.m_user_data_path_val.get(),
        }

    def esp_reboot(self):
        """
        重启芯片
        """
        self.m_connect["state"] = tk.DISABLED
        param = self.get_download_param()
        self.print_log("已发送重启指令！")
        self.m_father.update()
        cmd = ['--port', param["port"], '--baud', param["baud"],
               '--after', 'hard_reset', "read_mac"]
        esptool.main(cmd)
        self.m_connect["state"] = tk.NORMAL

    def __del__(self):
        """"
        资源释放 查杀线程
        """
        if self.ser != None:
            self.ser.close()  # 关闭串口
            self.ser = None
        if self.run_thread != None:
            # 杀线程
            _async_raise(self.run_thread)