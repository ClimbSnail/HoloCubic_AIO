# -*- coding: utf-8 -*-
################################################################################
#
# Author: ClimbSnail(HQ)
# original source is here.
#   https://github.com/ClimbSnail/Robot_For_RaspberryPi
# 
#
################################################################################

import threading
import socket  # socket模块
import time
import ctypes
import inspect


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


class RobotSocket(object):

    def __init__(self, ip, port, callback_func=None, name=""):
        self._name = name
        self._ip = ip
        self._port = port
        self._callback_func = callback_func

    def close(self):
        try:
            self.connfd.close()  # 关闭连接
            self.connfd = None
        except Exception as err:
            print(err)

    @property
    def callback_func(self):
        return self._callback_func

    @callback_func.setter
    def callback_func(self, callback):
        self._callback_func = callback

    def start(self, ):
        # override
        pass

    def __del__():
        pass


class RobotSocketServer(RobotSocket):
    # 服务端类
    def __init__(self, ip, port, callback_func=None, max_bind=1, name="RobotSocketServer"):
        """
        RobotSocketServer类对象的初始化
        :param ip: 点分十进制的ip字符串
        :param port: 端口号整型数据(0-65535)
        :param callback_func: 接收处理函数
        :param max_bind: 最大服务数量
        :param name: socket实例名称
        """
        super().__init__(ip, port, callback_func, name)
        self.__sersocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # 定义socket类型，网络通信，TCP
        self.__sersocket.bind((self._ip, self._port))  # 套接字绑定的IP与端口
        self.__max_bind = max_bind
        self.__client_link_dict = {}
        self.__sersocket.listen(self.__max_bind)  # 开始TCP监听
        self.__recv_buff = 1024 * 128

    def start(self):
        '''
        启动socket实例
        :return:
        '''

        def scanner():
            while True:
                connfd, addr = self.__sersocket.accept()  # 接受TCP连接，并返回新的套接字与IP地址
                print('Connected by', addr)  # 输出客户端的IP地址
                run_thread = threading.Thread(target=self.recvfrom_client, args=(connfd, addr))
                run_thread.start()
                self.__client_link_dict[addr] = {"fd": connfd, 'pthread': run_thread}

        run_thread = threading.Thread(target=scanner, args=())
        run_thread.start()

    def recvfrom_client(self, connfd, addr):
        """
        客户端连接状态的数据处理
        :param connfd: 连接客户端的文件句柄
        :param addr: 客户端的地址
        :return:
        """
        try:
            while True:
                recv = connfd.recv(self.__recv_buff)  # 把接收的数据实例化
                if recv == b'':  # 断开连接
                    break
                if self.callback_func != None:
                    self.callback_func(recv, addr)
        except Exception as err:
            print("This thread was killed, Client disconnected\t->\t", end='')
            print(err)

    def send_to_client(self, dat, addr):
        """
        向本次连接的客户端发送数据
        :param dat: 要发送的数据（bytes类型）
        :param addr: 要发送到的客户端地址
        :return:
        """
        try:
            if addr in self.__client_link_dict.keys():
                self.__client_link_dict["fd"].sendall(dat)
            else:
                print("Address is no found or disconnect.")
        except Exception as err:
            print(err)

    def __del__(self):
        try:
            for conninfo in self.__client_link_dict.items():
                connfd = conninfo["fd"]
                connfd.close()  # 关闭连接
                _async_raise(conninfo['pthread'])
                del conninfo
        except Exception as err:
            print(err)


class RobotSocketClient(RobotSocket):
    # 客户端类
    def __init__(self, ip, port, callback_func=None, disconntime=0.5, name="RobotSocketClient"):
        """
        RobotSocket类对象的初始化
        :param ip: 点分十进制的ip字符串
        :param port: 端口号整型数据(0-65535)
        :param callback_func: 接收处理函数
        :param name: socket实例名称
        """
        super().__init__(ip, port, callback_func, name)
        self.__clientsocket = None
        self.__connFlag = False  # 连接状态
        self.__disconntime = disconntime  # 掉线重连的时间
        self.__recv_buff = 1024 * 128

    def start(self):
        '''
        启动socket实例
        :return:
        '''

        def reconner():
            while True:
                try:
                    addr = (self._ip, self._port)
                    if False == self.__connFlag:
                        print("Try to reconnect......")
                        del self.__clientsocket
                        self.__clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # 定义socket类型，网络通信，TCP
                        self.__clientsocket.connect(addr)
                        print('Connected by', addr)  # 输出客户端的IP地址
                        self.__connFlag = True
                except Exception as err:
                    print(err)
                    time.sleep(self.__disconntime)

        self.reconner_thread = threading.Thread(target=reconner, args=())
        self.reconner_thread.start()
        self.recvfrom_ser_thread = threading.Thread(target=self.recvfrom_ser, args=())
        self.recvfrom_ser_thread.start()

    def recvfrom_ser(self, ):
        """
        客户端连接状态的数据处理
        :param client: 连接客户端的文件句柄
        :param addr: 客户端的地址
        :return: 
        """""
        try:
            while True:
                try:
                    if True == self.__connFlag:
                        recv = self.__clientsocket.recv(self.__recv_buff)  # 把接收的数据实例化
                        if recv == b'':  # 断开连接
                            break
                        if self.callback_func != None:
                            self.callback_func(recv)

                except Exception as err:
                    self.__clientsocket.close()
                    self.__connFlag = False
                    print(err)  # 发生异常所在的文件
                    time.sleep(self.__disconntime * 0.2)

        except Exception as err:
            print(err)

    def send_to_ser(self, dat):
        """
        向本次连接的客户端发送数据
        :param dat: 要发送的数据（bytes类型）
        :return:
        """
        try:
            self.__clientsocket.sendall(dat)
        except Exception as err:
            print(err)

    def __del__(self):
        try:
            self.__clientsocket.close()  # 关闭连接
            del self.__clientsocket
            self.__clientsocket = None
        except Exception as err:
            print(err)

        self.__connFlag = False
        _async_raise(self.reconner_thread)
        _async_raise(self.recvfrom_ser_thread)


if __name__ == "__main__":
    # This is demo

    # 服务器端范例
    def myRecvHandle(dat, addr):  # 接收函数
        sersocket.send_to_client(dat, addr)
        dat = ("Server recv %s from %s\n" % (dat, addr)).encode(encoding="utf-8")
        print(dat)


    # 初始化端口并设置接收数据的函数(当接收到数据，自动被调用)
    sersocket = RobotSocketServer("192.168.123.244", 6666, myRecvHandle, max_bind=10)
    sersocket.start()  # socket开始工作
    import time

    while True:
        time.sleep(1)
    # sersocket.send_to_client(b'Hello \n')
    """
    # 客户端范例
    def myRecvHandle(dat):  # 接收函数
        dat = ("Client recv %s\n" % dat).encode(encoding="utf-8")
        print(dat)

    # 初始化端口并设置接收数据的函数(当接收到数据，自动被调用)
    clientsocket = RobotSocketClient("192.168.123.244", 6666, myRecvHandle)
    clientsocket.start()    # socket开始工作
    import time
    while True:
        dataIn = input("输入要发送给服务端的数据：")
        dat = ("%s\n" % dataIn).encode(encoding="utf-8")
        clientsocket.send_to_ser(dat)
    """
