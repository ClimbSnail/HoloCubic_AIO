import os
import shutil
from glob import glob
import argparse


def notify(name):
    print(rf"""
创建app:{name}成功,接下来

编辑./HoloCubic_Firmware/src/HoloCubic_AIO.cpp
第一步：添加头文件
#include<app/{name}/{name}.h>

第二步：在setup中注册app
app_contorller->app_register(&{name}_app);
""")


def create_app(name):
    app_root = "HoloCubic_Firmware/src/app"
    app_path = os.path.join(app_root, name)
    example_path = os.path.join(app_root, "example")
    if os.path.exists(app_path):
        if input(f"app: {name} 已存在，删除并重新创建? [y/n]:") == 'y':
            shutil.rmtree(app_path)
        else:
            print(f"app: {name} 没有创建")
            return

    shutil.copytree(example_path, app_path)

    for file in glob(os.path.join(app_path, "example*")):
        renamed_file = file.replace("example", name)
        fin, fout = open(file, 'rt'), open(renamed_file, 'wt')
        for line in fin:
            line = line.replace('example', name.lower())
            line = line.replace('Example', name.capitalize())
            line = line.replace('EXAMPLE', name.upper())
            fout.write(line)
        fin.close()
        fout.close()
        os.remove(file)
    notify(name)


def main(config):
    if config.newapp:
        create_app(config.newapp)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='AIO manager')
    parser.add_argument('--newapp',
                        default="",
                        nargs="?",
                        type=str,
                        help='Start to write a new app in AIO')
    args = parser.parse_args()
    main(args)
