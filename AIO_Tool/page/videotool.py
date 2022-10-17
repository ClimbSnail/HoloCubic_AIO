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

import os
import tkinter as tk
import util.tkutils as tku
from tkinter import ttk
from tkinter import filedialog


class VideoTool(object):
    """
    视频转化页类
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

        output_param_frame = tk.Frame(father, bg=father["bg"])
        # 路径
        path_frame = tk.Frame(output_param_frame, bg=father["bg"])
        self.init_path(path_frame)
        path_frame.pack(side=tk.LEFT, pady=5)

        # 连接器相关控件
        # 使用LabelFrame控件 框出连接相关的控件
        self.connor_param_frame = tk.LabelFrame(output_param_frame, text="输出设置",
                                                #  labelanchor="nw",
                                                bg="white")
        # self.connor_param_frame.place(anchor="ne", relx=100.0, rely=100.0)
        # self.connor_param_frame.grid(row=1, column=1)
        # self.connor_param_frame.place(x=self.__father.winfo_width()+5, y=0)
        # self.connor_param_frame.update()
        self.connor_param_frame.pack(side=tk.LEFT, pady=5)
        self.init_options(self.connor_param_frame)  # 初始化参数

        output_param_frame.pack(side=tk.TOP, pady=5)

    def init_path(self, father):
        """
        初始化输入文件路径 输出文件路径
        :param father: 父容器
        :return: None
        """
        border_padx = 10  # 两个控件的间距

        # 输入原文件
        src_frame = tk.Frame(father, bg=father["bg"])
        # 创建输入框
        self.m_src_path_entry = tk.Entry(src_frame, width=80, highlightcolor="LightGrey")
        # self.m_src_path_entry["state"] = tk.DISABLED
        self.m_src_path_entry.pack(side=tk.LEFT, padx=border_padx)
        # 原视频输入按钮
        self.src_path_botton = tk.Frame(father, bg=father["bg"])
        self.src_path_botton = tk.Button(src_frame, text="选择视频", fg='black',
                                         command=self.choose_src_file, width=8, height=1)

        self.src_path_botton.pack(side=tk.RIGHT, fill=tk.X, padx=5)

        src_frame.pack(side=tk.TOP, pady=5)

        # 输入输出路径
        dst_frame = tk.Frame(father, bg=father["bg"])
        # 创建输入框
        self.m_dst_path_entry = tk.Entry(dst_frame, width=80, highlightcolor="LightGrey")
        # self.m_dst_path_entry["state"] = tk.DISABLED
        self.m_dst_path_entry.pack(side=tk.LEFT, padx=border_padx)
        defualt_outpath = os.path.join(os.getcwd(), ROOT_PATH)
        self.m_dst_path_entry.delete(0, tk.END)  # 清空文本框
        self.m_dst_path_entry.insert(tk.END, defualt_outpath)
        # 原视频输入按钮
        self.dst_path_botton = tk.Frame(father, bg=father["bg"])
        self.dst_path_botton = tk.Button(dst_frame, text="输出路径", fg='black',
                                         command=self.choose_dst_path, width=8, height=1)

        self.dst_path_botton.pack(side=tk.RIGHT, fill=tk.X, padx=5)

        dst_frame.pack(side=tk.TOP, pady=5)

        # 转换按钮
        button_frame = tk.Frame(father, bg=father["bg"])
        # 转换按钮
        self.trans_botton = tk.Frame(father, bg=father["bg"])
        self.trans_botton = tk.Button(button_frame, text="开始转换", fg='black',
                                      command=self.trans_format, width=8, height=1)

        self.trans_botton.pack(side=tk.TOP, fill=tk.X, padx=5)

        button_frame.pack(side=tk.TOP, pady=5)

    def choose_src_file(self):
        """
        点击"打开"菜单项触发的函数
        :return:
        """
        # 打开文件对话框 获取文件路径
        # defaultextension 为选取保存类型中的拓展名为文件名
        # filetypes为文件拓展名
        filepath = filedialog.askopenfilename(
            title='选择一个视频文件',
            defaultextension=".espace",
            # filetypes=[('mp4', '.mp4 .MP4'), ('avi', '.avi .AVI'), 
            #     ('mov', '.mov .MOV'), ('gif', '.gif .GIF'), ('所有文件', '.* .*')]
            # )
            filetypes=[('常用格式', '.mp4 .MP4 .avi .AVI .mov .MOV .gif .GIF'),
                ('所有文件', '.* .*')]
            )
        if filepath == None or filepath == "":
            return None
        else:
            # self.m_src_path_entry["state"] = tk.NORMAL
            self.m_src_path_entry.delete(0, tk.END)  # 清空文本框
            self.m_src_path_entry.insert(tk.END, filepath)
            # self.m_src_path_entry["state"] = tk.DISABLED

    def choose_dst_path(self):

        # 打开文件对话框 获取文件路径
        # defaultextension 为选取保存类型中的拓展名为文件名
        # filetypes为文件拓展名
        filepath = filedialog.askdirectory()
        if filepath == None or filepath == "":
            return None
        else:
            self.m_dst_path_entry.delete(0, tk.END)  # 清空文本框
            self.m_dst_path_entry.insert(tk.END, filepath)

    def trans_format(self):
        """
        格式转化
        """
        cur_dir = os.getcwd()  # 当前目录
        self.trans_botton["text"] = "正在转换"
        param = self.get_output_param()
        cmd_resize = 'ffmpeg -i "%s" -vf scale=%s:%s "%s"'  # 缩放转化
        # cmd_to_rgb 的倒数第二个参数其实没什么作用，因为rgb本身就是实际的像素点
        # （这个是为了跟cmd_to_mjpeg统一格式才加的参数）
        cmd_to_rgb = 'ffmpeg -i "%s" -vf "fps=%s,scale=-1:%s:flags=lanczos,crop=%s:in_h:(in_w-%s)/2:0" -c:v rawvideo -pix_fmt rgb565be -q:v %s "%s"'
        cmd_to_mjpeg = 'ffmpeg -i "%s" -vf "fps=%s,scale=-1:%s:flags=lanczos,crop=%s:in_h:(in_w-%s)/2:0" -q:v %s "%s"'

        name_suffix = os.path.basename(param["src_path"]).split(".")
        suffix = name_suffix[-1]  # 后缀名
        # 生成的中间文件名
        video_cache_name = name_suffix[0] + "_" + param["width"] + "x" + param["height"] + "_cache." + suffix
        # 带上路径
        video_cache = os.path.join(cur_dir, ROOT_PATH, CACHE_PATH, video_cache_name)
        # 最终输出的文件
        if param["format"] == 'rgb565be':
            out_format_tail = ".rgb"
            trans_cmd = cmd_to_rgb  # 最后的转换命令
        elif param["format"] == 'MJPEG':
            out_format_tail = ".mjpeg"
            trans_cmd = cmd_to_mjpeg  # 最后的转换命令
        else:
            out_format_tail = ".mjpeg"
            trans_cmd = cmd_to_mjpeg  # 最后的转换命令
        final_out = os.path.join(param["dst_path"],
                                 name_suffix[0] + "_" + param["width"] + "x" + param["height"] + out_format_tail)

        # 清理之前的记录
        try:
            os.remove(video_cache)
            os.remove(final_out)
        except Exception as err:
            pass

        middle_cmd = cmd_resize % (param["src_path"], param["width"],
                                   param["height"], video_cache)
        print(middle_cmd)
        out_cmd = trans_cmd % (video_cache, param["fps"], param["height"],
                               param["width"], param["width"], param["quality"], final_out)
        print(out_cmd)
        os.system(middle_cmd)
        os.system(out_cmd)
        # os.remove(video_cache)        
        self.trans_botton["text"] = "开始转化"

    def init_options(self, father):
        """
        初始化模型菜单子项
        :param father: 父容器
        :return: None
        """
        border_padx = 10  # 两个控件的间距

        # 单选按钮
        self.m_radio_val = tk.IntVar()  # IntVar
        radio_frame = tk.Frame(father, bg="DimGray")
        tk.Radiobutton(radio_frame, variable=self.m_radio_val, value=0,
                       text="默认", width=10, bg="DimGray",
                       command=self.radio_select).pack(side=tk.LEFT, pady=5)

        tk.Radiobutton(radio_frame, variable=self.m_radio_val, value=1,
                       text="自定义", width=10, bg="DimGray",
                       command=self.radio_select).pack(side=tk.RIGHT)
        self.m_radio_val.set(0)
        radio_frame.pack(side=tk.TOP, padx=5, fill="x")

        # 分辨率(长宽)
        ratio_frame = tk.Frame(father, bg=father["bg"])
        self.m_ratio_label = tk.Label(ratio_frame, text="分辨率（宽x高）",
                                      # font=self.my_ft1,
                                      bg=father['bg'])
        self.m_ratio_label.pack(side=tk.LEFT, padx=border_padx)
        self.m_width_entry = tk.Entry(ratio_frame, width=6, highlightcolor="LightGrey")
        self.m_width_entry.insert(tk.END, '240')
        self.m_width_entry.pack(side=tk.LEFT, padx=border_padx)
        self.m_height_entry = tk.Entry(ratio_frame, width=6, highlightcolor="LightGrey")
        self.m_height_entry.insert(tk.END, '240')
        self.m_height_entry.pack(side=tk.LEFT, padx=border_padx)
        ratio_frame.pack(side=tk.TOP, pady=5)

        # 帧率（fps）
        fps_frame = tk.Frame(father, bg=father["bg"])
        self.m_fps_label = tk.Label(fps_frame, text="帧率(fps)",
                                    # font=self.my_ft1,
                                    bg=father['bg'])
        self.m_fps_label.pack(side=tk.LEFT, padx=border_padx)
        # 创建输入框
        # self.m_fps_entry = tk.Entry(father, font=self.my_ft1, width=5, highlightcolor="LightGrey")
        self.m_fps_entry = tk.Entry(fps_frame, width=5, highlightcolor="LightGrey")
        self.m_fps_entry.insert(tk.END, '20')
        # self.m_pre_val_text = "1500"    # 保存修改前m_val_text输入框中的内容，供错误输入时使用
        # self.m_fps_entry.bind("<Return>", self.change_val)  # 绑定enter键的触发
        self.m_fps_entry.pack(side=tk.LEFT, padx=border_padx)
        # 质量
        self.m_quality_label = tk.Label(fps_frame, text="质量",
                                    # font=self.my_ft1,
                                    bg=father['bg'])
        self.m_quality_label.pack(side=tk.LEFT, padx=border_padx)
        self.m_quality_select = ttk.Combobox(fps_frame, width=5, state='readonly')
        self.m_quality_select["value"] = ('1', '2', '3', '4', '5', '6', '7', '8', '9')  # , 'GIF'
        # 设置默认值，即默认下拉框中的内容
        self.m_quality_select.current(4)
        self.m_quality_select.pack(side=tk.LEFT, padx=border_padx)
        fps_frame.pack(side=tk.TOP, pady=5)

        # 格式
        format_frame = tk.Frame(father, bg=father["bg"])
        self.m_format_label = tk.Label(format_frame, text="格式",
                                       # font=self.my_ft1,
                                       bg=father['bg'])
        self.m_format_label.pack(side=tk.LEFT, padx=border_padx)
        self.m_format_select = ttk.Combobox(format_frame, width=10, state='readonly')
        self.m_format_select["value"] = ('MJPEG', 'rgb565be')  # , 'GIF'
        # 设置默认值，即默认下拉框中的内容
        self.m_format_select.current(0)
        self.m_format_select.pack(side=tk.RIGHT, padx=border_padx)
        format_frame.pack(side=tk.TOP, pady=5)

        self.radio_select()

    def radio_select(self):
        """
        选择触发的函数
        :return:
        """
        if self.m_radio_val.get() == 0:
            self.m_width_entry["state"] = tk.DISABLED
            self.m_height_entry["state"] = tk.DISABLED
            self.m_fps_entry["state"] = tk.DISABLED
            self.m_quality_select["state"] = tk.DISABLED
            self.m_format_select["state"] = tk.DISABLED
        elif self.m_radio_val.get() == 1:
            self.m_width_entry["state"] = tk.NORMAL
            self.m_height_entry["state"] = tk.NORMAL
            self.m_fps_entry["state"] = tk.NORMAL
            self.m_quality_select["state"] = tk.NORMAL
            self.m_format_select["state"] = tk.NORMAL

    def get_output_param(self):
        """
        得到输出参数
        """
        return {
            "src_path": self.m_src_path_entry.get().strip(),
            "dst_path": self.m_dst_path_entry.get().strip(),
            "width": self.m_width_entry.get().strip(),
            "height": self.m_height_entry.get().strip(),
            "fps": self.m_fps_entry.get().strip(),
            "quality": self.m_quality_select.get().strip(),
            "format": self.m_format_select.get().strip()
        }
