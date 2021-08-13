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


class TfManager(object):
    """
    菜单栏类
    """

    def __init__(self, father, engine, lock=None):
        """
        VideoTool初始化
        :param father:父类窗口
        :param engine:引擎对象，用于推送与其他控件的请求
        :param lock:线程锁
        :return:None
        """
        self.m_engine = engine  # 负责各个组件之间数据调度的引擎
        self.father = father  # 保存父窗口

    def init_modelBar(self, menuBar):
        """
        初始化模型菜单子项
        :param menuBar: 主菜单
        :return: None
        """
        self.m_model_filepath = None
        # 创建菜单项
        self.modelBar = tk.Menu(menuBar, tearoff=0)
        # 将菜单项添加到菜单栏
        menuBar.add_cascade(label=self.m_engine.word_map["Menu"]["Model"], menu=self.modelBar)
        # 在菜单项中加入子菜单
        self.modelBar.add_command(label=self.m_engine.word_map["Menu"]["Create"], command=self.click_model_create)
        # 创建分割线
        self.modelBar.add_separator()

    def click_model_create(self):
        """
        点击模型"创建"菜单项触发的函数
        :return: None
        """
        print("click_model_create")
        # self.m_engine.OnThreadMessage(mh.M_CTRLMENU, mh.M_MODEL_FILEMANAGER,
        #                               mh.A_FILE_CREATE, self.m_model_filepath)
