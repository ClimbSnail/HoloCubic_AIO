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
import util.massagehead as mh
from page.videotool import VideoTool
from page.download_debug import DownloadDebug
from page.setting import Setting
from page.help import Helper
from page.images_converter import ImagesConverter 
from page.filemanager import FileManager

import os
import tkinter as tk
import util.tkutils as tku
from tkinter import ttk
from tkinter import messagebox


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
        self.root.iconbitmap("./holo_256.ico")  # 窗体图标
        # 文件转化的创建输出目录
        try:
            dir_path = os.path.join("OutFile", "Cache")
            os.makedirs(dir_path)
        except Exception as e:
            pass

        self.width = 700
        self.height = 500

        # tk.tab   tab页面的管理器
        self.m_tab_manager = ttk.Notebook(self.root)

        # 下载调试页面
        self.m_debug_tab = tk.Frame(self.m_tab_manager, bg="white")
        self.m_tab_manager.add(self.m_debug_tab, text="下载调试")
        self.m_debug_tab_windows = DownloadDebug(self.m_debug_tab, self)

        # 参数设置页面
        self.m_setting_tab = tk.Frame(self.m_tab_manager, bg="white")
        self.m_tab_manager.add(self.m_setting_tab, text="参数设置")
        self.m_setting_tab_windows = Setting(self.m_setting_tab, self)

        # 文件管理页面
        self.m_file_tab = tk.Frame(self.m_tab_manager, bg="white")
        self.m_tab_manager.add(self.m_file_tab, text="文件管理")
        self.m_file_tab_windows = FileManager(self.m_file_tab, self)

        # 图片转换页面
        self.m_image_tab = tk.Frame(self.m_tab_manager, bg="white")
        self.m_tab_manager.add(self.m_image_tab, text="图片转换")
        self.m_image_tab_windows = ImagesConverter (self.m_image_tab, self)

        # 视频转码页面
        self.m_video_tab = tk.Frame(self.m_tab_manager, bg="white")
        self.m_tab_manager.add(self.m_video_tab, text="视频转码")
        self.m_video_tab_windows = VideoTool(self.m_video_tab, self)

        # 屏幕分享页面
        self.m_srceen_tab = tk.Frame(self.m_tab_manager, bg="white")
        self.m_tab_manager.add(self.m_srceen_tab, text="屏幕分享")

        # 帮助页面
        self.m_help_tab = tk.Frame(self.m_tab_manager, bg="white")
        self.m_tab_manager.add(self.m_help_tab, text="帮助")
        self.m_help_tab_windows = Helper(self.m_help_tab, self)

        self.m_tab_manager.pack(expand=True, fill=tk.BOTH)
    
    def OnThreadMessage(self, fromwho, towho, action, param = None):
        """
        引擎的调度函数 控件利用此函数可间接操作或者获取其他控件的对应资源
        :param fromwho:表示调用者
        :param towho:表示请求操作的控件
        :param action:表示请求操作的操作类型
        :param param:操作请求所携带的参数(根据具体请求来指定参数类型)
        """
        print(fromwho, towho, action, param)
        #info = fromwho+" "+towho+" "+action+" "+param
        #self.OnThreadMessage(mh.M_ENGINE, mh.M_SYSINFO, mh.A_INFO_PRINT, info+"\n")

        if towho == mh.M_DOWNLOAD_DEBUG: # 下载模块操作请求
            self.m_debug_tab_windows.api(action, param)    # 处理消息

        elif towho == mh.M_SETTING: # 设置模块操作请求
            self.m_modelManager.api(action, param)

    def on_closing(self):
        """
        关闭主窗口时要触发的函数
        :return: None
        """
        if self.m_file_tab_windows != None:
            self.m_file_tab_windows.__del__()
            del self.m_file_tab_windows
            self.m_file_tab_windows = None

        # if messagebox.askokcancel("Quit", "Do you want to quit?"):
        self.root.destroy()

        if self.m_debug_tab_windows != None:
            del self.m_debug_tab_windows
            self.m_debug_tab_windows = None

        if self.m_setting_tab_windows != None:
            del self.m_setting_tab_windows
            self.m_setting_tab_windows = None

    def OnThreadMessage(self, fromwho, towho, action, param=None):
        """
        引擎的调度函数 控件利用此函数可间接操作或者获取其他控件的对应资源（用函数模拟网络通信模型）
        :param fromwho:表示调用者
        :param towho:表示请求操作的控件
        :param action:表示请求操作的操作类型
        :param param:操作请求所携带的参数(根据具体请求来指定参数类型)
        """
        print(fromwho, towho, action, param)

        if towho == mh.M_ENGINE and action == mh.A_UPDATALANG:  # 更新请求
            self.m_modelManager.api(mh.A_UPDATALANG)  # 按钮语言更新

    def __del__(self):
        """
        释放资源
        """
        # del self.m_debug_tab_windows
        self.m_debug_tab_windows = None

        if self.m_file_tab_windows != None:
            self.m_file_tab_windows.__del__()
            del self.m_file_tab_windows
            self.m_file_tab_windows = None


if __name__ == '__main__':
    tool_windows = tk.Tk()  # 创建窗口对象的背景色
    tool_windows.title("HoloCubic_AIO Tools" + "\t  " + VERSION)  # 窗口名
    tool_windows.geometry('1000x655+10+10')
    tool_windows.resizable(False, False)  # 设置窗体不可改变大小
    engine = Engine(tool_windows)
    tku.center_window(tool_windows)  # 将窗体移动到屏幕中央

    # 进入消息循环 父窗口进入事件循环，可以理解为保持窗口运行，否则界面不展示
    tool_windows.mainloop()
