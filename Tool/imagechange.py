import tkinter as tk
import tkutils as tku
from tkinter import ttk
import os.path, sys, time
from convertor_core import Convertor
from convertor_core import _const
from widget_base import EntryWithPlaceholder
from PIL import Image


FLAG = _const()

color_dict = {
    'CF_TRUE_COLOR': FLAG.CF_TRUE_COLOR,
    'CF_TRUE_COLOR_ALPHA':  FLAG.CF_TRUE_COLOR_ALPHA,
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
    'CF_TRUE_COLOR_332': FLAG.CF_TRUE_COLOR_332,
    'CF_TRUE_COLOR_565':  FLAG.CF_TRUE_COLOR_565,
    'CF_TRUE_COLOR_565_SWAP': FLAG.CF_TRUE_COLOR_565_SWAP,
    'CF_TRUE_COLOR_888': FLAG.CF_TRUE_COLOR_888
}


class ImagesChanger(object):
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
        self.m_father = father  # 保存父窗口
        self.m_select_frame = tk.Frame(self.m_father, bg=father["bg"])
        self.init_setting(self.m_select_frame)
        self.m_select_frame.pack(side=tk.TOP, pady=5)

        self.m_path_frame = tk.Frame(self.m_father, bg=father["bg"])
        self.init_image_path(self.m_path_frame)
        self.m_path_frame.pack(side=tk.TOP, pady=5)
        

    def init_setting(self, father):
        """
        初始化设置条
        :param father: 父容器
        :return: None
        """
        border_padx = 15  # 两个控件的间距
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
        self.m_output_select = ttk.Combobox(output_frame, width=20, state='readonly')
        self.m_output_select["value"] = ('C array', 'CF_TRUE_COLOR_332', 'CF_TRUE_COLOR_565',
                                         'CF_TRUE_COLOR_565_SWAP', 'CF_TRUE_COLOR_888')
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
                                                 command=self.choose_image_file, width=6, height=1)

        self.m_image_path_botton.pack(side=tk.LEFT, fill=tk.X, padx=5)

        # 转化按钮
        self.m_trans_botton = tk.Button(image_path_frame, text="开始转化", fg='black',
                                                 command=self.trans_images, width=8, height=1)

        self.m_trans_botton.pack(side=tk.LEFT, fill=tk.X, padx=5)
        image_path_frame.pack(side=tk.LEFT, pady=5)


    def choose_image_file(self):
        """
        点击"user_data"文件触发的函数
        :return:
        """
        # 打开文件对话框 获取文件路径
        # defaultextension 为选取保存类型中的拓展名为文件名
        # filetypes为文件拓展名
        filepath = tk.filedialog.askopenfilename(
            title='选择一个图片',
            defaultextension=".espace",
            filetypes=[('JPG', '.jpg .JPG')])
        if filepath == None or filepath == "":
            return None
        else:
            self.m_image_path_entry.delete(0, tk.END)  # 清空文本框
            self.m_image_path_entry.insert(tk.END, filepath)
    
    def trans_images(self):
        """
        转化图片
        """
        image_path = self.m_image_path_entry.get().strip()
        if image_path == None:  
            print("Error: could not load image")
            return 0
        
        try:
            width = int(self.m_output_width.get().strip())
            height = int(self.m_output_height.get().strip())
            self.img: Image.Image = Image.open(image_path).resize((width, height))
        except Exception as err:
            print(err)

        color_format = color_dict[self.m_color_select.get()]
        output_format = output_dict[self.m_output_select.get()]

        for i, img_path in enumerate(sys.argv[1:]):
            print("正在转换图片{} ...".format(os.path.basename(img_path)))
            c = Convertor(img_path, output_format)
            c.get_bin_file()