import sys
import tkinter as tk
import tkutils as tku
import serial
from esptool import main

VERSION = "Ver1.0"

select_com = None

class Engine(object):
    """
    引擎
    """
    def __init__(self, root):
        """
        引擎初始化
        :param root:窗体控件
        """
        self.root = root
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.word_map = {}  # 储存指定语言所对应字段下的文字

        self.word_map["DownPage"] = {}
        self.word_map["DownPage"]["Download"] = "下载"
        com_obj_list = list(serial.tools.list_ports.comports())

        # 获取可用COM口名字
        com_list = [com_obj[0] for com_obj in com_obj_list]
        if len(com_list) != 0:
            select_com = com_list[0]

        self.width = 700
        self.height = 500
        self.m_frame = tk.Frame(self.root, width=self.width, height=self.height, bg="white")
        self.m_model_map = {}   # 储存舵机控件 name(key):object
        self.m_frame.pack(side = tk.LEFT)
        self.m_frame.update()
        # 初始化操作按钮
        self.add_button = tk.Button(self.root, text=self.word_map["DownPage"]["Download"],
                               command = self.download, width=10, height=1)

        self.add_button.place(x=self.m_frame.winfo_width()-self.width,
                              y=self.m_frame.winfo_height()-self.height*2)

    def download(self):
        global select_com
        # select_com = input("input COM（例如 COM7）: ")
        cmd = ['CubicTool.py', '--port', select_com,
            '--baud', '921600',
            'write_flash', '-fm', 'dio', '-fs', '4MB',
            '0x1000', 'bootloader_dio_40m.bin',
            '0x00008000', 'partitions.bin',
            '0x0000e000', 'boot_app0.bin',
            '0x00010000', 'HoloCubic_AIO固件_v1.3.bin']
        sys.argv = cmd
        main()

    def on_closing(self):
        """
        关闭主窗口时要触发的函数
        :return: None
        """
        pass
        # if messagebox.askokcancel("Quit", "Do you want to quit?"):
        #     root.destroy()

if __name__ == '__main__':
    tool_windows = tk.Tk()  # 创建窗口对象的背景色
    tool_windows.title("HoloCubic_AIO"+"\t  "+VERSION)   #窗口名
    tool_windows.geometry('1000x655+10+10')
    tool_windows.resizable(False, False)  # 设置窗体不可改变大小
    engine = Engine(tool_windows)
    tku.center_window(tool_windows)  # 将窗体移动到屏幕中央

    # 进入消息循环 父窗口进入事件循环，可以理解为保持窗口运行，否则界面不展示
    tool_windows.mainloop()