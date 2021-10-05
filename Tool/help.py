# -*- coding: utf-8 -*-
################################################################################
#
# Author: ClimbSnail(HQ)
# original source is here.
#   https://github.com/ClimbSnail/HoloCubic_AIO
# 
#
################################################################################

import tkinter as tk
import tkutils as tku


class Helper(object):
    """
    帮助信息页类
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
        self.init_info(self.m_father)

    def init_info(self, father):
        """
        初始化信息打印框
        :param father: 父容器
        :return: None
        """
        info_width = father.winfo_width()
        info_height = father.winfo_height() / 2

        info = '''
        本上位机专门针对HoloCubic AIO固件开发，其中固件烧录功能兼容其他第三方固件。

        加入AIO内测QQ群：755143193 获取最新版本及相关技术支持。
        功能持续补充完善中，本上位机的所有源码均开源，欢迎大家学习，
        也欢迎加入到AIO固件的开发队列。

        观看演示链接：
            https://www.bilibili.com/video/BV1jh411a7pV?p=1

        了解Holocubic AIO固件
            请访问 https://github.com/ClimbSnail/HoloCubic_AIO （最新版本）
            或者 https://gitee.com/ClimbSnailQ/HoloCubic_AIO

        了解Holocubic硬件开源方案
            请访问 https://github.com/peng-zhihui/HoloCubic
        '''

        self.m_project_info = tk.Text(father, height=45, width=140)
        self.m_project_info.tag_configure('bold_italics',
                                          font=('Arial', 12, 'bold', 'italic'))
        self.m_project_info.tag_configure('big', font=('Verdana', 16, 'bold'))
        self.m_project_info.tag_configure('color', foreground='#476042',
                                          font=('Tempus Sans ITC', 12, 'bold'))

        self.m_project_info.pack()
        self.m_project_info.config(state=tk.NORMAL)
        self.m_project_info.insert(tk.END, info, 'big')
        self.m_project_info.config(state=tk.DISABLED)
