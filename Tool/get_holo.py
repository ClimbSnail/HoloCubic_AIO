import os.path, sys, time
from convertor_core import Convertor
from convertor_core import _const


if __name__ == '__main__':

    if len(sys.argv) < 2:
        print("用法: 把要转换的 JPG/PNG/BMP 文件拖到.exe图标上即可")
        time.sleep(3)
        sys.exit(0)

    FLAG = _const()
    for i, img_path in enumerate(sys.argv[1:]):
        print("正在转换图片{} ...".format(os.path.basename(img_path)))
        c = Convertor(img_path, FLAG.CF_TRUE_COLOR_565)
        c.get_bin_file()
        # c.get_c_code_file()
