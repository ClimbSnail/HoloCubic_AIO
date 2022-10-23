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
from tkinter.constants import COMMAND
import util.tkutils as tku
from tkinter import ttk
from util.common import *
from ctypes import *
from util.file_info import *
from util.robotsocket import *
import sys
import traceback


# 文件数据的结构
# FileObj {"type":"file", "name":"Filore_1", "path":"", "sub_file":[]}


class FileManager(object):
    """
    菜单栏类
    """

    def __init__(self, father, engine, lock=None):
        """
        FileManager 初始化
        :param father:父类窗口
        :param engine:引擎对象，用于推送与其他控件的请求
        :param lock:线程锁
        :return:None
        """
        self.__engine = engine  # 负责各个组件之间数据调度的引擎
        self.__father = father  # 保存父窗口
        self.__tree_map_file = {}  # 用于绑定文件对象与tree元素的关系 treeID->
        self.__path_map_file = {}  # 用于绑定文件对象与tree元素的关系
        self.__clientsocket = None
        self.__is_freestatus = False  # 标志是否是空闲状态 False否

        # 连接器相关控件
        self.m_conn_frame = tk.Frame(self.__father, bg=father["bg"])
        self.init_connect(self.m_conn_frame)  # 初始化ip地址
        self.m_conn_frame.pack(side=tk.TOP, pady=5)

        # 目录树
        self.path_tree_frame = tk.Frame(father, bg=father["bg"])
        self.path_tree_frame.place(x=10, y=50)
        self.path_tree_frame.update()
        self.init_path_tree(self.path_tree_frame)  # 初始化目录树
        # self.path_tree_frame.pack(side=tk.LEFT, fill=tk.Y, padx=5)

        # 视图区
        self.view_file_frame = tk.Frame(father, bg=father["bg"])
        self.init_view_file(self.view_file_frame)  # 初始化视图区
        self.view_file_frame.pack(side=tk.LEFT, fill=tk.Y, padx=5)

        # 初始化右击操作项(默认不显示)
        self.init_section(father)

        # print("this test")
        # fs = DirList("/")
        # recv_data = b'##\x02\x01\x00d\x04/\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        # recv_data = b'##\x01\x02\x01\xfa\x04/\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00System Volume Information/\tmovie/\tweather/\timage/\tB1.JPG\t\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        # fs.decode(recv_data)
        # print(fs.header_mark)
        # print(fs.msg_len)

        # fs = DirList("/", "test_dir")
        # data = fs.encode()
        # print(getSendInfo(data))

        # fs = DirList("")
        # fs.decode(data)
        # print(fs.dir_info.decode('utf-8'))

    def init_section(self, father):
        """
        初始化右击的操作栏(实现右键菜单)
        :param father: 父容器
        """

        def op_file_download():
            print("Enter op_file_download")
            pass

        def op_file_rename():
            print("Enter op_rename")
            pass

        def op_file_delect():
            print("Enter op_delect")
            pass

        def op_file_read_param():
            print("Enter op_read_param")
            pass

        # 创建文件的操作菜单
        self.__file_op_menu = tk.Menu(father, tearoff=0)
        self.__file_op_menu.add_command(label="下载", command=op_file_download)
        self.__file_op_menu.add_separator()
        self.__file_op_menu.add_command(label="重命名", command=op_file_rename)
        self.__file_op_menu.add_separator()
        self.__file_op_menu.add_command(label="删除", command=op_file_delect)
        self.__file_op_menu.add_separator()
        self.__file_op_menu.add_command(label="属性", command=op_file_read_param)

        def op_folder_upload_file():
            print("Enter op_folder_upload_file")
            pass

        def op_folder_create_subfolder():
            print("Enter op_folder_create_subfolder")
            pass

        def op_folder_rename():
            print("Enter op_folder_rename")
            pass

        def op_folder_delect():
            print("Enter op_folder_delect")
            pass

        # 创建文件夹的操作菜单
        self.__folder_op_menu = tk.Menu(father, tearoff=0)
        self.__folder_op_menu.add_command(label="上传文件", command=op_folder_upload_file)
        self.__folder_op_menu.add_separator()
        self.__folder_op_menu.add_command(label="新建文件夹", command=op_folder_create_subfolder)
        self.__folder_op_menu.add_separator()
        self.__folder_op_menu.add_command(label="重命名", command=op_folder_rename)
        self.__folder_op_menu.add_separator()
        self.__folder_op_menu.add_command(label="删除", command=op_folder_delect)

    def init_connect(self, father):
        """
        初始化连接
        :param father: 父容器
        :return: None
        """
        border_padx = 10  # 两个控件的间距

        ip_frame = tk.Frame(father, bg=father["bg"])
        self.m_ip_label = tk.Label(ip_frame, text="ip地址",
                                   # font=self.my_ft1,
                                   bg=father['bg'])
        self.m_ip_label.pack(side=tk.LEFT, padx=border_padx)
        # 创建输入框
        self.m_ip_entry = tk.Entry(ip_frame, width=20, highlightcolor="LightGrey")
        # self.m_ip_entry["state"] = tk.DISABLED
        self.m_ip_entry.pack(side=tk.LEFT, padx=border_padx)
        self.m_ip_entry.delete(0, tk.END)  # 清空文本框
        # self.m_ip_entry.insert(tk.END, "192.168.123.241:8081")
        self.m_ip_entry.insert(tk.END, "本功能目前不可用")
        # 连接按钮
        # self.conn_botton = tk.Frame(father, bg=father["bg"])
        self.conn_botton = tk.Button(ip_frame, text="连接", fg='black',
                                     command=self.connect_holocubic, width=8, height=1)

        self.conn_botton.pack(side=tk.RIGHT, fill=tk.X, padx=5)

        ip_frame.pack(side=tk.TOP, pady=5)

    def connect_holocubic(self):
        # 客户端范例
        def myRecvHandle(dat):  # 接收函数
            msg_head = MsgHead()
            msg_head.decode(dat)
            print("Massages Len = ", msg_head.msg_len)

            msg_fs = FileSystem()
            # dat = b'##\x01\x02\x01\xfa\x04/\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00System Volume Information/\tmovie/\tweather/\timage/\tB1.JPG\t\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
            msg_fs.decode(dat)
            print("Massages action_type = ", msg_fs.action_type)

            display_data = ("Client recv %s\n" % dat).encode(encoding="utf-8")
            print("Massages dat = ", display_data)

            # 消息处理
            if msg_fs.action_type == AT.AT_FREE_STATUS:
                print("AT_FREE_STATUS")
                self.__is_freestatus = True
                return None
            elif msg_fs.action_type == AT.AT_DIR_CREATE:
                print("AT_DIR_CREATE")
            elif msg_fs.action_type == AT.AT_DIR_REMOVE:
                print("AT_DIR_REMOVE")
            elif msg_fs.action_type == AT.AT_DIR_RENAME:
                print("AT_DIR_RENAME")
            elif msg_fs.action_type == AT.AT_DIR_LIST:
                print("AT_DIR_LIST")
                msg = DirList()
                msg.decode(dat)
                dir_path = msg.dir_path.decode('utf-8').strip(b'\x00'.decode())
                sub_file_list = msg.dir_info.decode('utf-8').split('\t')[:-1]
                print("dir_path len: ", len(dir_path))
                print("DirList info: ", dir_path)
                print("DirList info: ", sub_file_list)
                self.reflush_folder(dir_path, sub_file_list)
            elif msg_fs.action_type == AT.AT_FILE_CREATE:
                print("AT_FILE_CREATE")
            elif msg_fs.action_type == AT.AT_FILE_WRITE:
                print("AT_FILE_WRITE")
            elif msg_fs.action_type == AT.AT_FILE_READ:
                print("AT_FILE_READ")
                msg = FileRead()
                msg.decode(dat)
            elif msg_fs.action_type == AT.AT_FILE_REMOVE:
                print("AT_FILE_REMOVE")
            elif msg_fs.action_type == AT.AT_FILE_RENAME:
                print("AT_FILE_RENAME")
            elif msg_fs.action_type == AT.AT_FILE_GET_INFO:
                print("AT_FILE_GET_INFO")
                msg = FileGetInfo()
                msg.decode(dat)

        if self.conn_botton["text"] == "连接":
            try:
                ip_port = self.m_ip_entry.get().strip()
                ip, port = ip_port.split(":")
                print(ip, port)
                # 初始化端口并设置接收数据的函数(当接收到数据，自动被调用)
                self.__clientsocket = RobotSocketClient(ip, int(port), myRecvHandle)
                self.__clientsocket.start()  # socket开始工作

                self.conn_botton["text"] = "断开连接"
            except Exception as err:
                print(err)
        else:
            self.conn_botton["text"] = "连接"
            if self.__clientsocket != None:
                self.__clientsocket.__del__()
                # del self.__clientsocket
                self.__clientsocket = None

    def init_path_tree(self, father):
        """
        初始化连接
        :param father: 父容器
        :return: None
        """

        def display_op_menu(event):
            """
            显示一个文件操作框
            """
            try:
                for item in self.tree.selection():
                    print(self.tree.focus())
                    print(item)
                    if self.__tree_map_file[item]["type"] == "file":
                        # 右击的是文件
                        self.__file_op_menu.post(event.x_root, event.y_root)
                    elif self.__tree_map_file[item]["type"] == "folder":
                        # 右击的是文件夹
                        self.__folder_op_menu.post(event.x_root, event.y_root)
                    print(self.__tree_map_file[item]["path"])
            except Exception as err:
                print(str(traceback.format_exc()))
                print(err)

        border_padx = 10  # 两个控件的间距

        path_tree_frame = tk.Frame(father, bg=father["bg"])

        self.tree = ttk.Treeview(path_tree_frame, show="tree", selectmode="browse",
                                 height=28)
        tree_y_scroll_bar = tk.Scrollbar(path_tree_frame, command=self.tree.yview, relief=tk.SUNKEN, width=6)
        tree_y_scroll_bar.pack(side=tk.RIGHT, fill=tk.Y)
        self.tree["yscrollcommand"] = tree_y_scroll_bar.set
        # self.tree.config(yscrollcommand = tree_y_scroll_bar.set)
        tree_x_scroll_bar = tk.Scrollbar(path_tree_frame, command=self.tree.xview, relief=tk.SUNKEN, width=6)
        tree_x_scroll_bar.pack(side=tk.BOTTOM, fill=tk.X)
        self.tree["xscrollcommand"] = tree_x_scroll_bar.set
        # self.tree.config(xscrollcommand = tree_x_scroll_bar.set)
        self.tree.pack(expand=1, fill=tk.BOTH)

        # <<TreeviewSelect>>
        # <<TreeviewOpen>>
        # <<TreeviewClose>>
        # <Button-1> <Button-2> <Button-3> 鼠标、左、右键单机
        self.tree.bind("<<TreeviewOpen>>", lambda event: self.tree_open())
        self.tree.bind("<Button-3>", display_op_menu)
        # text.bind("<MouseWheel>", lambda event : self.update_line())
        self.folder_img = tk.PhotoImage(file=r"./image/folder.png")
        self.file_img = tk.PhotoImage(file=r"./image/text_file.png")

        # 初始化根
        self.tree_root = self.tree.insert("", tk.END, text="内存卡文件", open=True, image=self.folder_img)
        root_file = {"tree": self.tree_root, "type": "folder", "name": "内存卡文件", "path": "/", "sub_file": []}
        self.__tree_map_file[self.tree_root] = root_file  # 初始化总目录
        self.__path_map_file[root_file["path"]] = root_file
        self.display_path_tree(self.tree_root, root_file);

        # path_tree_frame.pack(side=tk.TOP, pady=5)
        path_tree_frame.pack(side=tk.RIGHT, fill=tk.Y)

    def display_path_tree(self, cur_tree_root, fileObj):
        """
        显示目录树
        :param cur_tree_root: 当前树根
        :param fileObj: 当前要更新的文件父对象
        :return: None
        """
        if fileObj["sub_file"] == None:
            print("文件元素不需要显示")
            return None  # 文件元素不需要显示

        # 删除之前创建的节点
        self.tree.item
        for sub_item in self.tree.get_children(cur_tree_root):
            self.__tree_map_file[sub_item]["tree"] = None  # 由于下一步需要删除
            del self.__tree_map_file[sub_item]
            self.tree.delete(sub_item)

        # 刷新显示子元素
        for sub_file in fileObj["sub_file"]:
            if sub_file["type"] == "folder":
                image = self.folder_img
            else:
                image = self.file_img
            sub_tree = self.tree.insert(cur_tree_root, tk.END, text=sub_file["name"],
                                        values=(sub_file["path"],), open=True, image=image)

            # 绑定tree与文件对象的关系
            self.__tree_map_file[sub_tree] = sub_file
            sub_file["tree"] = sub_tree
            # self.__file_map_tree[sub_file["path"]] = sub_tree

    def reflush_folder(self, updata_path, sub_file_list):
        """
        刷新目录

        """
        try:
            self.__path_map_file[updata_path]["sub_file"].clear()
            for sub_file_name in sub_file_list:
                sub_tmp = None
                if "/" == sub_file_name[-1]:
                    sub_tmp = {"tree": None, "type": "folder", "name": sub_file_name[:-1],
                               "path": updata_path + sub_file_name[:-1], "sub_file": []}
                else:
                    sub_tmp = {"tree": None, "type": "file", "name": sub_file_name, "path": updata_path + sub_file_name,
                               "sub_file": None}

                # 添加节点
                self.__path_map_file[sub_tmp["path"]] = sub_tmp
                # 将子节点添加到父节点中
                self.__path_map_file[updata_path]["sub_file"].append(sub_tmp)

            print(self.__path_map_file[updata_path])
        except Exception as err:
            print(str(traceback.format_exc()))
            print(err)

        # 刷新显示
        self.display_path_tree(self.__path_map_file[updata_path]["tree"], self.__path_map_file[updata_path]);

    def tree_open(self):
        """
        Tree元素被打开
        """
        for item in self.tree.selection():
            # 得到当前选中的节点
            print("open---> ", self.tree.item(item, "open"))
            if self.__tree_map_file[item]["type"] == "file":
                return None
            if self.__clientsocket != None:
                path = self.__tree_map_file[item]["path"]
                path = path if path == "/" else path.rstrip("/")
                send_data = DirList(path).encode()
                print("Send ---> ", len(send_data), end="  ")
                print(send_data)
                # 发送查询数据
                self.__clientsocket.send_to_ser(send_data)

    def init_view_file(self, father):
        """
        初始化连接
        :param father: 父容器
        :return: None
        """
        border_padx = 10  # 两个控件的间距
        view_file_frame = tk.Frame(father, bg=father["bg"])

        view_file_frame.pack(side=tk.TOP, pady=5)

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
        menuBar.add_cascade(label=self.__engine.word_map["Menu"]["Model"], menu=self.modelBar)
        # 在菜单项中加入子菜单
        self.modelBar.add_command(label=self.__engine.word_map["Menu"]["Create"], command=self.click_model_create)
        # 创建分割线
        self.modelBar.add_separator()

    def click_model_create(self):
        """
        点击模型"创建"菜单项触发的函数
        :return: None
        """
        print("click_model_create")
        # self.__engine.OnThreadMessage(mh.M_CTRLMENU, mh.M_MODEL_FILEMANAGER,
        #                               mh.A_FILE_CREATE, self.m_model_filepath)

    def __del__(self):
        if self.__clientsocket != None:
            self.__clientsocket.close()
            self.__clientsocket.__del__()
            self.__clientsocket = None
