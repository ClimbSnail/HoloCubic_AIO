import sys
import os
import serial
from esptool import main

VERSION = "Ver1.0"


if __name__ == '__main__':
	try:
		com_obj_list = list(serial.tools.list_ports.comports())

		select_com = None
		# 获取可用COM口名字
		com_list = [com_obj[0] for com_obj in com_obj_list]
		print(com_list)
		if len(com_list) == 1:
			select_com = com_list[0]
		else:
			select_com = input("input COM（例如 COM7）: ")

		
		#列出文件夹下所有的目录与文件
		list_file = os.listdir("./")
		firmware_path = 'HoloCubic_AIO固件_v1.5.bin'
		for file_name in list_file:
			if 'HoloCubic_AIO' in file_name:
				firmware_path = file_name.strip()
		
		cmd = ['CubicTool.py', '--port', select_com,
			'--baud', '921600',
			'write_flash', '-fm', 'dio', '-fs', '4MB',
			'0x1000', 'bootloader_dio_40m.bin',
			'0x00008000', 'partitions.bin',
			'0x0000e000', 'boot_app0.bin',
			'0x00010000', firmware_path]
		sys.argv = cmd
		main()
	except Exception as err:
		print(err)
	
	input("Enter")