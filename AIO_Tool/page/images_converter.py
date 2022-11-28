# -*- coding: utf-8 -*-
################################################################################
#
# Author: ClimbSnail(HQ)
# original source is here.
#   https://github.com/ClimbSnail/HoloCubic_AIO_Tool
# 
#
################################################################################

from util import *
from util.common import *
from util.widget_base import EntryWithPlaceholder
from util.convertor_core import Converter
from util.convertor_core import _const
import util.tkutils as tku

import tkinter as tk
from tkinter import ttk
import os.path
from PIL import Image
import os
import shutil

FLAG = _const()

color_dict = {
    'CF_TRUE_COLOR': FLAG.CF_TRUE_COLOR,
    'CF_TRUE_COLOR_ALPHA': FLAG.CF_TRUE_COLOR_ALPHA,
    'CF_TRUE_COLOR_CHROMA': FLAG.CF_TRUE_COLOR_CHROMA,
    'CF_INDEXED_1_BIT': FLAG.CF_INDEXED_1_BIT,
    'CF_INDEXED_2_BIT': FLAG.CF_INDEXED_2_BIT,
    'CF_INDEXED_4_BIT': FLAG.CF_INDEXED_4_BIT,
    'CF_INDEXED_8_BIT': FLAG.CF_INDEXED_8_BIT,
    'CF_ALPHA_1_BIT': FLAG.CF_ALPHA_1_BIT,
    'CF_ALPHA_2_BIT': FLAG.CF_ALPHA_2_BIT,
    'CF_ALPHA_4_BIT': FLAG.CF_ALPHA_4_BIT,
    'CF_ALPHA_8_BIT': FLAG.CF_ALPHA_8_BIT,
    'CF_RAW': FLAG.CF_RAW,
    'CF_RAW_ALPHA': FLAG.CF_RAW_ALPHA,
    'CF_RAW_CHROMA': FLAG.CF_RAW_CHROMA
}

output_dict = {
    'C_array': -1,
    'Binary_332': FLAG.CF_TRUE_COLOR_332,
    'Binary_565': FLAG.CF_TRUE_COLOR_565,
    'Binary_565_SWAP': FLAG.CF_TRUE_COLOR_565_SWAP,
    'Binary_888': FLAG.CF_TRUE_COLOR_888
}


class ImagesConverter(object):
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
        self.__engine = engine  # 负责各个组件之间数据调度的引擎
        self.__father = father  # 保存父窗口

        self.m_select_frame = tk.Frame(self.__father, bg=father["bg"])
        self.init_setting(self.m_select_frame)
        self.m_select_frame.pack(side=tk.TOP, pady=5)

        self.m_path_frame = tk.Frame(self.__father, bg=father["bg"])
        self.init_image_path(self.m_path_frame)
        self.m_path_frame.pack(side=tk.TOP, pady=5)

        self.m_info_frame = tk.Frame(self.__father, bg=father["bg"])
        self.init_info(self.m_info_frame)
        self.m_info_frame.pack(side=tk.TOP, pady=5)

    def init_setting(self, father):
        """
        初始化设置条
        :param father: 父容器
        :return: None
        """
        border_padx = 15  # 两个控件的间距

        self.m_jpg_label = tk.Label(father, text="JPG格式输出",
                                    # font=self.my_ft1,
                                    bg=father['bg'])
        self.m_jpg_label.pack(side=tk.LEFT)
        # 勾选框的键值对象
        self.__jpg_enable_val = tk.IntVar()
        self.__jpg_enable_val.set(1)
        # 勾选框
        self.__jpg_enable = tk.Checkbutton(father, text="", bg=father["bg"],
                                           variable=self.__jpg_enable_val,
                                           onvalue=1, offvalue=0, height=1,
                                           width=1, command=self.enable_jpg)
        self.__jpg_enable.pack(side=tk.LEFT)
        # 色彩格式
        color_frame = tk.Frame(father, bg=father["bg"])
        self.m_color_label = tk.Label(color_frame, text="ColorFormat",
                                      # font=self.my_ft1,
                                      bg=father['bg'])
        self.m_color_label.pack(side=tk.LEFT)
        self.m_color_select = ttk.Combobox(color_frame, width=20, state='readonly')
        self.m_color_select["value"] = ('CF_TRUE_COLOR', 'CF_TRUE_COLOR_ALPHA',
                                        'CF_TRUE_COLOR_CHROMA', 'CF_INDEXED_1_BIT',
                                        'CF_INDEXED_2_BIT', 'CF_INDEXED_4_BIT', 'CF_INDEXED_8_BIT',
                                        'CF_ALPHA_1_BIT', 'CF_ALPHA_2_BIT',
                                        'CF_ALPHA_4_BIT', 'CF_ALPHA_8_BIT',
                                        'CF_RAW', 'CF_RAW_ALPHA', 'CF_RAW_CHROMA')
        self.m_color_select["state"] = tk.DISABLED
        # 设置默认值，即默认下拉框中的内容
        self.m_color_select.current(1)
        self.m_color_select.pack(side=tk.RIGHT, padx=border_padx)
        color_frame.pack(side=tk.LEFT, pady=5)

        # 输出格式
        output_frame = tk.Frame(father, bg=father["bg"])
        self.m_output_label = tk.Label(output_frame, text="OutputFormat",
                                       # font=self.my_ft1,
                                       bg=father['bg'])
        self.m_output_label.pack(side=tk.LEFT)
        self.m_output_select = ttk.Combobox(output_frame, width=15, state='readonly')
        self.m_output_select["value"] = ('C_array', 'Binary_332', 'Binary_565',
                                         'Binary_565_SWAP', 'Binary_888')
        self.m_output_select["state"] = tk.DISABLED
        # 设置默认值，即默认下拉框中的内容
        self.m_output_select.current(2)
        self.m_output_select.pack(side=tk.LEFT, padx=border_padx)
        output_frame.pack(side=tk.LEFT, pady=5)

        # 输出格式
        out_ratio_frame = tk.Frame(father, bg=father["bg"])
        self.m_output_width = tk.Label(out_ratio_frame, text="分辨率（宽x高）",
                                       # font=self.my_ft1,
                                       bg=father['bg'])
        self.m_output_width.pack(side=tk.LEFT)
        # 创建宽输入框
        self.m_width_val = tk.StringVar()
        self.m_width_entry = EntryWithPlaceholder(out_ratio_frame, width=6, highlightcolor="LightGrey",
                                                  placeholder="宽", placeholder_color="grey",
                                                  textvariable=self.m_width_val)
        self.m_width_entry.pack(side=tk.LEFT, padx=5)
        # 创建高输入框
        self.m_height_val = tk.StringVar()
        self.m_height_entry = EntryWithPlaceholder(out_ratio_frame, width=6, highlightcolor="LightGrey",
                                                   placeholder="高", placeholder_color="grey",
                                                   textvariable=self.m_height_val)
        self.m_height_entry.pack(side=tk.LEFT, padx=5)
        out_ratio_frame.pack(side=tk.LEFT, pady=5)

        # 设置默认值输出分辨率
        self.default_ratio()

    def default_ratio(self):
        """
        设置默认值输出分辨率
        :return: None
        """
        self.m_width_val.set('240')
        self.m_height_val.set('240')
        # self.m_boot_path_entry.refresh()

    def init_image_path(self, father):
        """
        初始化设置条
        :param father: 父容器
        :return: None
        """
        border_padx = 15  # 两个控件的间距
        # 色彩格式
        image_path_frame = tk.Frame(father, bg=father["bg"])
        # 创建路径输入框
        self.m_image_path_val = tk.StringVar()
        self.m_image_path_entry = EntryWithPlaceholder(image_path_frame, width=80, highlightcolor="LightGrey",
                                                       placeholder="选择要转化的图片路径", placeholder_color="grey",
                                                       textvariable=self.m_image_path_val)
        self.m_image_path_entry.pack(side=tk.LEFT, padx=border_padx)
        # 原视频输入按钮
        self.m_image_path_botton = tk.Button(image_path_frame, text="选择", fg='black',
                                             command=self.choose_image_files, width=6, height=1)

        self.m_image_path_botton.pack(side=tk.LEFT, fill=tk.X, padx=5)

        # 转化按钮
        self.m_trans_botton = tk.Button(image_path_frame, text="开始转化", fg='black',
                                        command=self.trans_images, width=8, height=1)

        self.m_trans_botton.pack(side=tk.LEFT, fill=tk.X, padx=5)

        # 提示文字
        self.m_tip_label = tk.Label(image_path_frame, text="点击转化",
                                    fg="green",
                                    bg=father['bg'])
        self.m_tip_label.pack(side=tk.LEFT, padx=border_padx)

        image_path_frame.pack(side=tk.LEFT, pady=5)

    def enable_jpg(self):
        if self.__jpg_enable_val.get() == 1:
            self.m_color_select["state"] = tk.DISABLED
            self.m_output_select["state"] = tk.DISABLED
        else:
            self.m_color_select["state"] = tk.NORMAL
            self.m_output_select["state"] = tk.NORMAL

    def choose_image_files(self):
        """
        点击"user_data"文件触发的函数
        :return:
        """
        # 打开文件对话框 获取文件路径
        # defaultextension 为选取保存类型中的拓展名为文件名
        # filetypes为文件拓展名
        filepath = tk.filedialog.askopenfilenames(
            title='选择若干个图片',
            defaultextension=".espace",
            filetypes=[('Image', '.jpg .JPG .png .PNG'), ('所有文件', '.* .*')])
        if filepath == None or filepath == "":
            return None
        else:
            filepaths = ";".join(filepath)
            self.m_image_path_val.set(filepaths)
            # self.m_image_path_entry.delete(0, tk.END)  # 清空文本框
            # self.m_image_path_entry.insert(tk.END, filepath)

    def trans_images(self):
        """
        转化图片
        """
        self.m_tip_label.configure(text="正在转化")
        self.__father.update()
        images_path = self.m_image_path_val.get().strip()
        if images_path == None:
            print("Error: could not load image")
            return 0

        # 设计的图片地址栏如果是选择的话只有一张图片
        # 如果是批量拉取到界面中，则地址栏为多个图片地址的组合字符串(分号分隔)，如：path1;path2;path3
        for img_path in images_path.strip().split(";"):
            # 循环处理每张照片
            img_path = img_path.strip()
            if img_path == "":
                print("路径为空")
                return False

            input_path = None  # 真正参与转化的图片
            # 图片输出后的文件后缀
            save_suffix = os.path.splitext(img_path)[-1]  # '.png' '.jpg'
            try:
                width = int(self.m_width_val.get().strip())
                height = int(self.m_height_val.get().strip())
                src_im: Image.Image = Image.open(img_path)  # : Image.Image
                if self.__jpg_enable_val.get() == 1:
                    # 如果转化成jpg的话
                    # 由于PNG是RGBA四个通道 而jpg只有RGB三个通道
                    src_im = src_im.convert('RGB')
                    save_suffix = '.jpg'  # '.png' '.jpg'
                new_im = None
                if src_im.height == height and src_im.width == width:
                    new_im = src_im
                    input_path = img_path
                else:
                    new_filename = os.path.basename(img_path).split('.')[0] + save_suffix
                    print(new_filename)
                    input_path = os.path.join(ROOT_PATH, CACHE_PATH, new_filename)
                    new_im = src_im.resize((width, height))
                    new_im.save(input_path, quality=95)  # , format='JPEG', quality=95
                    # print("Look: ", input_path)
                    # src_im.save(input_path)
                    # print("Finish.\n")
            except Exception as err:
                print(err)

            print("正在转换图片 {} ...".format(os.path.basename(images_path)))
            if self.__jpg_enable_val.get() == 1:
                shutil.copy(input_path, ROOT_PATH)
            else:
                color_format = color_dict[self.m_color_select.get()]
                output_format = output_dict[self.m_output_select.get()]
                print("color_format = ", color_format)
                print("output_format = ", output_format)
                if output_format == -1:
                    out_obj = Converter(input_path, True, color_format)
                    out_obj.get_c_code_file(outpath=ROOT_PATH)
                else:
                    pass
                    print(input_path)
                    out_obj = Converter(input_path, True, output_format)
                    out_obj.get_bin_file(outpath=ROOT_PATH)
            self.m_tip_label.configure(text="转化完成")
            print("转化完成")

    def init_info(self, father):
        """
        初始化信息打印框
        :param father: 父容器
        :return: None
        """
        info_width = father.winfo_width()
        info_height = father.winfo_height() / 2

        info = '''
        本功能为LVGL图片转化工具  输入任意分辨率图片，转成你所指定的分辨率的图片。
        可以同时选择多张图片，进行批量转换。转化完毕的照片存在本软件同级目录的OutFile文件夹下。
        注：OutFile/Cache为缓存目录，可自行删除。

        若转为存在内存卡中的jpg照片请勾选："JPG格式输出"

        若转为存在内存卡中的bin照片请去掉"JPG格式输出"勾选，后选择：
            ColorFormat：CF_TRUE_COLOR_ALPHA    OutputFormat：Binary_565

        若转为存在Flash固件中的数组代码请去掉"JPG格式输出"勾选，后选择：
            ColorFormat：CF_INDEXED_4_BIT    OutputFormat：C_array
        '''

        self.m_project_info = tk.Text(father, height=30, width=140)
        self.m_project_info.tag_configure('bold_italics',
                                          font=('Arial', 12, 'bold', 'italic'))
        self.m_project_info.tag_configure('big', font=('Verdana', 16, 'bold'))
        self.m_project_info.tag_configure('color', foreground='#476042',
                                          font=('Tempus Sans ITC', 12, 'bold'))

        self.m_project_info.pack()
        self.m_project_info.config(state=tk.NORMAL)
        self.m_project_info.insert(tk.END, info, 'big')
        self.m_project_info.config(state=tk.DISABLED)
