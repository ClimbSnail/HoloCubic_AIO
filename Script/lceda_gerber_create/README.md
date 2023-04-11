# 项目说明
修改其他EDA工具生成的Gerber文件为LCEDA可识别的Gerber文件，让非lceda设计的EDA文件能够"白嫖"。

# 使用方法

文件`task.yaml`中的每个`Gerber`即为每个任务，修改、添加、删除来控制任务。

执行脚本 `python lceda_gerber.py`

# 修改原理
1. 修改`task.yaml`中所指定后缀名的文件，添加头部以及更换编辑的时间戳。
2. 本程序不会修改你原始的文件，新生成的文件会在已`SnailHeater_Gerber_XXXX`命名的文件夹下

## 打包成可执行程序

使用pyinstaller打包程序 pyinstaller --icon ./holo_256.ico -w -F lceda_gerber.py