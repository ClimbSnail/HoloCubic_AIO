import os

if __name__ == '__main__':
    print("\n\n使用说明：\n\t将'mp4', 'avi', 'mov'一个或多个视频文件放在本程序同级目录，并且ffmpeg.exe程序也应在本程序的同级目录。\n")

    cmd_resize = "ffmpeg -i %s -vf scale=%s:%s %s"
    cmd_to_rgb = 'ffmpeg -i %s -vf "fps=9,scale=-1:%s:flags=lanczos,crop=%s:in_h:(in_w-%s)/2:0" -c:v rawvideo -pix_fmt rgb565be %s'
    video_middle_list = []
    video_out_list = []
    support_format = ["mp4", "avi", "mov"]

    try:
        ratio = input("请输入分辨率(如180*180请输入180)：").strip()
        if ratio == "":
            raise "输入错误"

        # 列出文件夹下所有的目录与文件
        list_file = os.listdir("./")
        video_file_list = [filename for filename in list_file if filename.split('.')[-1] in support_format]

        for file_name in video_file_list:
            name = file_name.split('.')
            suffix = name[-1]  # 后缀名
            # 生成的中间文件名
            video_cache = name[0] + "_" + str(ratio) + "_cache." + suffix
            video_middle_list.append(video_cache)
            video_out = name[0] + "_9fps.rgb"
            video_out_list.append(video_out)

            middle_cmd = cmd_resize % (file_name, ratio, ratio, video_cache)
            print(middle_cmd)

            out__cmd = cmd_to_rgb % (video_cache, ratio, ratio, ratio, video_out)
            print(out__cmd)
            os.system(middle_cmd)
            os.system(out__cmd)

    except Exception as err:
        print(err)

    print("\n\n生成的中间文件为-------> ", video_middle_list)

    print("生成的最终文件为-------> ", video_out_list)
    print("\n\n")
    del_flag = input("是否删除中间文字（Y/N）: ").strip()
    try:
        if del_flag == 'Y' or "y":
            for filename in video_middle_list:
                os.remove(filename)
            print("\n删除的文件-------> ", video_middle_list)

    except Exception as err:
        print(err)

    input("\n运行结束")
