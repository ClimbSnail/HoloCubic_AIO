#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import sys

re_obj = re.compile(r'/\*\(.+\)\*/')
# re_obj = re.compile(r'[\u4e00-\u9fa5]')
word_set = set()
result = "" # 存放返回的结果


def searchWordByRe(file_name):
    global re_obj
    global word_set
    global result

    file_obj = open(file_name, 'r',  encoding='utf-8') 

    try:
        text_list = file_obj.readlines()

        for line in text_list:
            dict_list = re_obj.findall(line)

            for font in dict_list:
                word = font[3:-3]
                if word not in word_set:
                    # 去重
                    word_set.add(word)
                    result = result + word

    finally:
        file_obj.close()

    print(result)


def searchWordByEocode(file_name):
    global re_obj
    global word_set
    global result

    file_obj = open(file_name, 'r',  encoding='utf-8') 

    try:
        text_list = file_obj.readlines()
        for line in text_list:
            for word in line:
                if u'\u4e00' <= word <= u'\u9fff':
                    if word not in word_set:
                        # 去重
                        word_set.add(word)
                        result = result + word

    finally:
        file_obj.close()
    
    print(result)


if __name__ == "__main__":
    """
    用法：python get_font.py 字模.c文件的路径
    """

    file_name = sys.argv[1]
    print(file_name)

    searchWordByRe(file_name)
    # searchWordByEocode()