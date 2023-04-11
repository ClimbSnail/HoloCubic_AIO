import os
import yaml
import datetime

def getLegalFileName(filPathList, rule, newFileName):
    '''
    获取符合指定后缀名的文件 返回路劲列表
    '''    
    legalList = []  # 需要修改的文件名路径
    legalNewNameList = []   # 新文件名列表
    illegalList = []
    for filePath in filPathList:
        name, ext = os.path.splitext(filePath)
        for name, value in rule.items():
            if ext in value:
                legalList.append(filePath)
                legalNewNameList.append(newFileName[name])
                break
        else:
            illegalList.append(filePath)
    
    return legalList, legalNewNameList, illegalList

def addHeaderToFile(srcFilePath, dstFilePath, header):
    '''
    copy文件并添加文件头
    '''
    with open(srcFilePath, "r") as file:
        fileData = file.read()
        
    with open(dstFilePath, "w") as file:
        file.write(header)
        file.write(fileData)


if __name__ == "__main__":
    try:
        nowTime = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        taskFile = open("task.yaml", "r", encoding="utf-8")
        tastInfo = yaml.load(taskFile, Loader=yaml.SafeLoader)
        taskFile.close()
        
        header = tastInfo["Header"].replace("%s", nowTime)
        TextFileName = tastInfo["TextFileName"]
        TextFileContent = tastInfo["TextFileContent"]
        NewFileNameDict = tastInfo["NewFileName"]
        gerberList = tastInfo["GerberList"]

        BaseOutPath = ".\\AIO_Gerber_" + datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
        os.mkdir(BaseOutPath)
        # 解析多个Gerber
        for gerberInfo in gerberList:
            SrcDir = gerberInfo["SrcDir"]
            OutDir = gerberInfo["OutDir"]
            print("\nGerberInfo ---> \n", SrcDir, "\n", gerberInfo)

            # 创建新 Gerber 的目录
            dstDir = OutDir
            if not dstDir:
                dirName = os.path.basename(SrcDir)
                dstDir = os.path.join(BaseOutPath, dirName)
            if not os.path.exists(dstDir):
                os.mkdir(dstDir)

            #创建PCB下单必读文档
            with open(os.path.join(dstDir, TextFileName), "w") as textFile:
                textFile.write(TextFileContent)

            SrcDirFiles = os.listdir(SrcDir)
            print(SrcDirFiles)
            legalList, legalNewNameList, illegalList = getLegalFileName(SrcDirFiles, gerberInfo["SUFFIX"], NewFileNameDict)

            dstPathList = []
            for filePath in legalNewNameList:
                dstPathList.append(os.path.join(dstDir, os.path.basename(filePath)))

            print("\n\n", dstPathList)
            for srcPath, dstPath in zip(legalList, dstPathList):
                pass
                addHeaderToFile(os.path.join(SrcDir, srcPath), dstPath, header)

            # 原封不动拷贝没有命中规则的文件
            dstPathList = []
            for filePath in illegalList:
                dstPathList.append(os.path.join(dstDir, os.path.basename(filePath)))
            for srcPath, dstPath in zip(illegalList, dstPathList):
                pass
                os.system('copy "%s" "%s"'%(os.path.join(SrcDir, srcPath), dstPath))

        print("\n\n生成的文件在本目录下的 【%s】 下\n" % BaseOutPath)
        input("按回车以关闭本软件。")
        exit()
    except Exception as err:
        print(err)
        print(err.__traceback__.tb_frame.f_globals["__file__"],
            "\tLines：", err.__traceback__.tb_lineno)  # 发生异常所在的文件

    