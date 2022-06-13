###### <sub>All In One™</sub><br />稚晖君`HoloCubic`小电视<br />──<br /><sup>小白制作指南</sup><br />`#最新版本|V2.1#`<br /><br /><br />**素笺淡墨、溜马小哥、神秘藏宝室、无常喵态、醉梦、董小心、小二黑、皖南浪漫革命、撝谦、一叶知秋**<br />*入群导航必看*

[TOC]



# 第一章  前 言

​      本文出自`#HoloCubic AIO多功能固件群755143193 #(blue)`  ，是对`#HoloCubic #(blue)`和`#AIO固件（All in One） #(orange)`制作的详细描述。透明小电视简介 https://www.bilibili.com/video/BV1jh411a7pV?p=6 。有问题多提问，态度好一点，群友们都是用爱发电`#注:看了说明书之后再提问 #(orange)`

​      本文主要内容是根据群内现有资料汇总编写。

​      原作者的项目链接：https://github.com/peng-zhihui/HoloCubic

​      群主项目的地址 ：https://github.com/ClimbSnail/HoloCubic_AIO （最新版本）或者：https://gitee.com/ClimbSnailQ/HoloCubic_AIO

​     `#视频参考： #(red)`

​     `#群主（ClimbSnail B站:溜马小哥）的|视频合集 📝 #(blue)`:

​     https://www.bilibili.com/video/BV1jh411a7pV/?spm_id_from=333.788.recommend_more_video.0

​     `#一叶知秋君笑我的|保姆级教程 #(blue)`:https://www.bilibili.com/video/BV11h41147iJ?spm_id_from=333.999.0.0

​     `#神秘藏宝室老哥的|【制作避坑篇】💡 #(blue)`:https://b23.tv/WuBqTGO

​     `#撝谦的|焊接参考教程 #(blue)`:https://b23.tv/A83JUGt?share_medium=android&share_source=qq&bbid=BBCEC45F-7A83-45A8-B161-5384DDA3085232266infoc&ts=1651579335471

​     `#群友薛定_饿_了么|教程参考 #(blue)`:https://www.bilibili.com/video/BV1eu411i7Qo?p=1&share_medium=android&share_plat=android&share_session_id=8f9d7992-dfe1-46c9-84b6-055729c97ae0&share_source=QQ&share_tag=s_i&timestamp=1654861439&unique_k=JUQMGWO

其它待补充……

# 第二章  硬件、外壳等制作

​      `#第二章所涉及的所有硬件文件均在"硬件外壳相关"的群文件夹|对应版本压缩包中 #(yellow)`

## 硬件制作前准备



​      电脑安装Altium Designer（简称“AD”，群文件夹"面向开发者工具"有，可自行下载）

​      安装目的：① 焊接有问题时方便打开原理图&PCB查看

​       ② 打包给板厂（如：嘉立创）为了避免提示拆单嫌疑，建议添加或者删除一些丝印。（如果不会修改，也不会生成Gerber文件，可以用“华秋DFM”直接生成Gerber文件，方法在2.3.2）

![image-20220610202303750](https://pic.imgdb.cn/item/62a337ac0947543129627416.png![图片](xxx.png#center))

​      `#安装“嘉立创下单助手”——白嫖必备 #(orange)`

​      下载链接（右上角：下载PC小助手）：https://www.jlc.com/?s=AD&sdclkid=AL2s152ibJDibOfpA5gD&renqun_youhua=727919&bd_vid=11596835000121341583

![image-20220610202449490](https://pic.imgdb.cn/item/62a338150947543129630ab0.png![图片](xxx.png#center))

## 硬件版本（图片源于群友分享）

​      稚晖君一共有四个版本，透明底座版本 Naive 、 metal 、钢铁侠版本 Ironman 和 BiliBili天使底座 ， 

群友大多数使用的是透明底座 Naive 版本。

​      `#①  透明底座Naive版本|（板厚推荐 1.2mm） #(cyan)`：

![image-20220610203418927](https://pic.imgdb.cn/item/62a33a4f0947543129665b5b.png![图片](xxx.png#center))

​                       

​      `#②  Metal 金属/透明版本|（板厚推荐 1.2mm【透明Metal2.0支持1.6mm，建议1.2mm】） #(cyan)`：

<img src="https://pic.imgdb.cn/item/62a33a7009475431296687a3.png![图片](xxx.png#center)" alt="image-20220610203452685" style="zoom:67%;" />



​      `#③  钢铁侠 Ironman 版本|（稚晖君源文件 板厚推荐 1.0mm） #(cyan)`： 

 <img src="https://pic.imgdb.cn/item/62a33af50947543129674a03.png![图片](xxx.png#center)" alt="image-20220610203558203" style="zoom:80%;" />



​      `#④ BiliBili 小金人版本|无 #(cyan)`

 <img src="https://pic.imgdb.cn/item/62a33afc09475431296753fa.png![图片](xxx.png#center)" alt="image-20220610203621211" style="zoom:80%;" />



​       大多数群友使用的是透明底座Naive版本，当然群主二次开车也制作了不少金属的Metal版本的。也有群友制作钢铁侠头版本的（如图2.2）。

 ![image-20220610203652890](https://pic.imgdb.cn/item/62a33b020947543129675d06.png![图片](xxx.png#center))

​                                                                                                                                      图2.2

​       很多大佬写了Holocubic固件，各版本固件基本通用（Metal、Naive）这两个硬件版本（至少群主的AIO固件是全兼容的）。

​       稀饭哪个版本就做哪个版本。Holocubic原作者稚晖君的部分电路是存在时序的问题（`#需要将C7电容更改为1uF~10uF可以解决 #(orange)`），群里有改良过的版本（`#一定要看群文件最新版本 #(orange)`）。

​       注：

​      **①** `#硬件只支持2.4G 的wifi。 #(yellow)`5G 以及双频合一的都不支持。任何固件都无法突破硬件的限制。

​      **②** `#对于非理/工科、没有电子基础、动手能力一般又想玩的 #(yellow)`，建议询问下群友，部分群友是有转让或售卖的成品（`#个人制作一次成功成本大约200左右，制作请慎重 #(yellow)`）。

​      **③** 元器件购买不确定的一定要问群友！！！

​      **④** `#新的DC-DC（metal）版本，LDO（LP2992或者ME6211C33）、DCDC（MP1471） 二选一【焊接了MP1471，拓展板、主板的LDO都要拆掉】。 #(yellow)`

​      **⑤** Metal、Naive通用性需要查看群内最新的（`#各版本压缩包 #(yellow)`）

## PCB制作

### 文件下载及说明



​      `#第一步 #(orange)`：通过2.2`#学习了解了群资料 #(yellow)`后，确认好硬件版本，在AIO群文件中找到对应的PCB文件进行下载（群文件会不定期更新，旧文件会被移除）。

​      `#注：群文件是经过群主改良测试过的 #(red)`

![image-20220610212836190](https://pic.imgdb.cn/item/62a3e6130947543129190deb.png![图片](xxx.png#center))

![image-20220610212850946](https://pic.imgdb.cn/item/62a3e6260947543129191ed2.png![图片](xxx.png#center))

​                                                                                                                   `#2.3.1|文件下载 #(lime)` 

​      `#第二步 #(orange)`：下载完后解压，会出现五个文件如图2.3.2所示。以Metal版本举例说明。

![image-20220610213015473](https://pic.imgdb.cn/item/62a3e63f0947543129193380.png![图片](xxx.png#center))

​                                                                                                                    `#2.3.2|解压文件 #(lime)` 

![image-20220610213035457](https://pic.imgdb.cn/item/62a4025c094754312935287f.png![图片](xxx.png#center))

![image-20220610213044184](https://pic.imgdb.cn/item/62a4026e0947543129353d31.png![图片](xxx.png#center))

​                                                                                                           `#2.3.3|主板文件夹文件说明 #(lime)`

![image-20220610213100688](https://pic.imgdb.cn/item/62a4028b0947543129355f73.png![图片](xxx.png#center))

![image-20220610213107948](https://pic.imgdb.cn/item/62a402990947543129356d76.png![图片](xxx.png#center))

​                                                                                                      `#2.3.4|拓展板文件夹文件说明 #(lime)`



### PCB打样教程

​       `#注：下单前一定要用AD改下丝印 #(orange)`，添加或者删除丝印都行，然后将PCB文件压缩打包给立创。不会改丝印，生成Gerber文件也行。（不会生成Gerber文件的可以下载一个“华秋DFM”软件）

![image-20220610213219026](https://pic.imgdb.cn/item/62a3e67c0947543129196627.png![图片](xxx.png#center))

​                                                                                                             `#图为AD09添加丝印 #(lime)`



#### 如何使用华秋DFM生成Gerber

​      华秋DFM下载链接：https://dfm.elecfans.com/index

​      网盘下载链接：https://www.aliyundrive.com/s/vh9fgUf57jz

​      `#①  常用Gerber文件清单 #(purple)`

      我们在做PCB板的时候，一般交给板厂的Gerber文件中必须包括下面的1-10和15，其中11-14可以不用，但建议一起放在Gerber文件中。

​          1.GTO(Top Overlay):顶层丝印层，常见为白油。

​          2.GTS(Top Solder):顶层阻焊层，常见为绿油。

​          3.GTL（Top Layer）:顶层走线层。

​          4.Gtp(Top Paste Mask):顶层锡膏防护层。

​          5.GBp(Bottom Paste Mask):底层锡膏防护层。

​          6.GBL(Bottom Layer):底层走线层。

​          7.GBS(Bottom Solder):底层阻焊层，常见为绿油。

​          8.GBO（Bottom Overlay）:底层丝印层，常见为白油。

​          9.GMx（Mechanical）或GKO(Keep-out Layer):用来定义板框，板框在哪层就选哪层。

​          10.Gx(Mid Layer):为中间信号X层，中间有多少层就有多少文件。

​          11.Gd(Drill Drawing):钻孔制图层。

​          12.Gg(Drill Guide)：钻孔说明层。

​          13.Gpt(Top Pad Master):顶层焊盘层。

​          14.Gpb(Bottom Pad Master):底层焊盘层。

​          15.NC drill Files:钻孔文件，AD导出一般为txt文件。一般有如下几个文件①RoundHoles-NonPlated（圆孔-非电镀钻孔文件）②RoundHoles-Plated（圆孔电镀钻孔文件）③SlotHoles-Plated（槽孔镀层 钻孔文件）。



​      `#②  Gerber各文件讲解 #(purple)`

​           1.Top Overlay/Bottom Overlay:顶层和底层丝印层，主要显示元器件边框，位号，属性，标注信息等等。

​           2.Top Solder/Bottom Solder:层和底层阻焊层，显示的是不需要覆盖绿油的焊盘，开窗，器件等。

​           3.Top Layer/Bottom Layer:顶层和底层走线层，顶层和底层的走线信息。

​           4.Mechanical或Keep-out Layer：用于放置机械图形，如PCB的外形等，在这层上放置的图形在任何层上都有相应的图形，并且是不会被丝印上阻焊剂的。一般用于放置板框。

​           5.Top Paste Mask/Bottom Paste Mask：这两层是用于制作钢网的文件。

​           6.Drill Drawing，Drill Guide，NC drill Files都是钻孔信息，它们各有什么作用了？为什么导出Gerber文件了还需要导出NC Drill Files？

​              ① DrillGuide主要作用是引导钻孔用的，主要是用于手工钻孔以定位

​              ② DrillDrawing是用于查看钻孔孔径的，在手工钻孔时，这两个文件要配合使用。不过现在大多是数控钻孔，所以这两层用处不是很大，即需要我们为数控提供NC drill Files（一般为txt或excel文件）。虽然Drill Drawing和Drill Guide文件可以由NC drill Files生成，但为了减少后续工作，建议在文件中加上这两个文件。



​      `#第一步 #(yellow)`：打开华秋DFM，会提示让你注册/登录（我没试过不登录能不能，我的是自动登录的），注册/登录之后，点击文件→打开→PCB（需要改生成的Gerber文件）

![image-20220610213545135](https://pic.imgdb.cn/item/62a4048a0947543129379f71.png![图片](xxx.png#center))

​                                                                                                                            `# 导入PCB #(lime)`



![image-20220610213611294](https://pic.imgdb.cn/item/62a404a1094754312937b627.png![图片](xxx.png#center))

​                                                                                                                       `# 导入中 #(lime)`                   

​       `#第二步 #(yellow)`：打开之后就会看到如下图所示的Gerber文件

![image-20220610213634982](https://pic.imgdb.cn/item/62a404b7094754312937ce42.png![图片](xxx.png#center))

![image-20220610213642912](https://pic.imgdb.cn/item/62a404ca094754312937e2ac.png![图片](xxx.png#center))



​      `#第三步 #(yellow)`：选择文件，导出Gerber文件，选择需要导出的文件夹导出。导出之后会提示导出成功。

![image-20220610213700360](https://pic.imgdb.cn/item/62a404d7094754312937f1d6.png![图片](xxx.png#center))

![image-20220610213709034](https://pic.imgdb.cn/item/62a404fd0947543129381f79.png![图片](xxx.png#center))

![image-20220610213716614](https://pic.imgdb.cn/item/62a405290947543129384e4f.png![图片](xxx.png#center))



​      `#第四步 #(yellow)`：在导出的文件查看Gerber文件，确认后，就可以直接压缩打包。

![image-20220610213732789](https://pic.imgdb.cn/item/62a4051109475431293834bc.png![图片](xxx.png#center))



​      `#第五步 #(yellow)`：导出的Gerber文件可以对照前面列出的文件，查看有无漏掉或者多余的。

![image-20220610213749660](https://pic.imgdb.cn/item/62a40564094754312938a2af.png![图片](xxx.png#center))



#### 立创PCB打样（参考一群管理的关于Naive版Holo3制作教程）



​      `#第一步 #(yellow)`：将2.2.2中标注的主板PCB文件&拓展板PCB & Gerber文件单独压缩打包。

![image-20220610213818292](https://pic.imgdb.cn/item/62a40590094754312938cc07.png![图片](xxx.png#center))



​      `#第二步 #(yellow)`：嘉立创下单软件上传PCB 压缩文件，解析过程请耐心等待。

![image-20220610213839623](https://pic.imgdb.cn/item/62a405ef0947543129393716.png![图片](xxx.png#center))

​                                                                                                                        `# 上传PCB文件 #(lime)`

![image-20220610213854642](https://pic.imgdb.cn/item/62a406250947543129397cdb.png![图片](xxx.png#center))

​                                                                                                                            `#  PCB文件解析 #(lime)`  



​      `#第三步 #(yellow)`：解析完成后就会出现下图所示的下单选项类，板子的长宽一般会自动识别，如果无法自动识别可以随便填比如：5x5等等，后面板厂会自己调整，板子数量为5（超过就不能白嫖）。

![image-20220610213917796](https://pic.imgdb.cn/item/62a4062e0947543129398733.png![图片](xxx.png#center))



​      `#第四步 #(yellow)`：板子厚度根据自己选择的小电视版本而定。

![image-20220610213934524](https://pic.imgdb.cn/item/62a406350947543129398fca.png![图片](xxx.png#center))

​                                                                                                                        `#  板子厚度选择 #(lime)`



​      `#第五步 #(yellow)`：阻焊层颜色可以根据自己的喜好来选择。

![image-20220610213949548](https://pic.imgdb.cn/item/62a4063c094754312939981a.png![图片](xxx.png#center))

​                                                                                                                     `#  阻焊层颜色选择 #(lime)`   



​      `#第六步 #(yellow)`：是否开钢网文件。

​       注：新手小白建议开主板的正面钢网，群里会提供钢网文件，大家自行上淘宝找钢网的卖家就可以，价格在10~20（不含邮费）。

![image-20220610214027440](https://pic.imgdb.cn/item/62a406430947543129399ed7.png![图片](xxx.png#center))

​                                                                                                                    `#  钢网文件 #(lime)`    

## 材料及所用工具购买

### 元器件购买（参考AIO群在线文档中的BOM表）



​      **①** 文件中有BOM 单，可根据BOM 购买器件（推荐淘宝优信电子购买）MPU 6050可以购买模块拆。也可以参考群在线文文档中的推荐链接购买 https://docs.qq.com/sheet/DQUpSbmN4TVNha0h0

​      **②** 屏幕可以在淘宝中景园购买（品牌瀚彩[目前显示效果最好的]）

​      **③** 不知道的或者不确定的，可以问群友。

​      **④** 电容/电阻的耐压值≥5V都可以（如16V、24V、50V等等）【仅限于本项目】

​      **⑤**  连接器座子型号为：翻盖下接8P（拓展板子和主板都用这个），排线买同向的0.5 * 8P（naive推荐6cm、metal用3cm）。

​      **⑥**  智晖君原版卡槽型号为DM3D-SF（不推荐），改板卡槽见群在线文档链接或者淘宝搜 翻盖内焊式TF卡座（8P）

​      **⑦**  LDO购买时一定要注意型号，ME6211C33【后面的33表示3.3V】别买错了，有群友就买错了。

​      注：某些元器件建议多买1-2个，翻盖下接8P座子建议买6-8个左右

<img src="https://pic.imgdb.cn/item/62a4064a094754312939a75c.png![图片](xxx.png#center)" alt="image-20220610214236590" style="zoom:67%;" />

​      **⑦** 内存卡的选择

​          固件对内存卡有一定的兼容性问题，并不是所有都兼容。目前支持`#SD、SDHC类型的储存卡，不支持SDXC! #(orange)`推荐以下已验证的内存卡。

​          ①  爱国者32G普通卡。淘宝大概20块钱。（性价比高，推荐）

​          ②  闪迪32G普通卡。淘宝大概30。

​      ⑧ 屏幕问题

​      屏幕的规格：1.3寸，驱动为ST7789 分辨率为240*240，焊接式12Pin。大多数屏幕板是金属外壳，`#注意短路问题! #(orange)`

​      这样的屏幕网上有很多，但对于制作Holocubic来说需要选择好，屏幕将会影响整体的显示效果。目前测试了很多家屏幕，效果最好的就是中景园的。

​      不推荐其他家的屏幕用来制作Holocubic。下面来实际比对下（请忽略背景差异）：

​      左边：中景园(翰彩)  右边：优智景



![image-20220611203259749](https://pic.imgdb.cn/item/62a48b7f0947543129e84767.png![图片](xxx.png#center))  



​      两个屏幕贴上棱镜后的差距主要在于优智景显示的周围空白区域严重偏蓝，很影响最终的效果。中景园的空白区域显示就很纯净。注：实际上中景园底色也偏蓝，但很轻微。

​      关于亮度问题（与屏幕无关）：

​     上图也可以看到，左边的明显亮度高，这是由于左边没有撕掉出厂偏光片带的保护膜（能增大亮度），同时左边的色彩由于保护膜的折射变得极其不准确。

​     做过实验的同学还会发现，在撕掉保护膜后，棱镜X轴放置和Y轴放置亮度是不一样的。人眼对色彩相较于亮度来说还是比较敏感的，个人建议撕掉保护膜。（`#亮度可以通过购买6：4的棱镜来改善，6：4的亮度比5：5的亮度略有提升 #(orange)`）

​     当然，如果更在意亮度也可以不撕掉保护膜，买屏幕的时候可以要求卖家发货的时候带保护膜且不要给保护膜贴那个绿色的标签。

### 制作工具（参照群主）

​      `#①|焊接类 #(orange)` 

​      电烙铁（936 焊台物美价廉，烙铁头样式多）；

​      焊台，又称：铁板烧（500w 鹿仙子）；

​      镊子（0402 体积较小，推荐用尖头镊子）；

​      焊锡丝（0.3mm/0.5mm/1.0mm DIY 常用就这三个规格）、

​      助焊剂及锡浆推荐买维修佬的（有条件可以买日本品牌中亚）。

​      `#②|砂纸 #(orange)`

​      300 目、800 目、5000 目、8000 目，根据需求购买（数字越大打磨越细）

​      高透外壳就是磨砂外壳，经过细砂纸打磨喷光油出来的。

​      `#群主推荐400和800目的，粗磨+细磨 800目可以磨出磨砂的感觉，另外打磨是需要流水打磨，不然打磨出来灯光一照都是划痕。 #(yellow)`

​      `#③|胶（教程看群主的） #(orange)`

​      704胶（封屏幕边框缝隙用）、UV胶 或者 B-7000 【粘贴屏幕用】注： UV胶 需要额外购买固 

化灯。棱镜贴合视频教程 https://www.bilibili.com/video/BV1jh411a7pV?p=5



### 外壳制作

​      纯白色的可以上三维猴（立创）

​      半透的可以去未来工厂或者淘宝（也可以咨询群友）

​      金属CNC （车已开走）【可以等群主的钢铁侠车】

## 外壳的3D打印

`#所有最新模型文件都在群对应版本压缩包中！！！ #(yellow)`

​      `#①|Naive透明底座（改进版） #(orange)`  

​            1.适用于Naive版本硬件。

​            2.原设计方案是使用螺丝连接上下壳，但群里有大佬磁吸改良版（磁铁为3*3mm圆形磁铁）推荐使用改良版。

​            3.底座使用的是3D打印机制作的，建议使用半透明的打印效果。

​            4.打印出来的表面如果想做成磨砂的效果，推荐使用800目的砂纸在水龙头冲洗下打磨。

​            5.如果找网上代打的，注意公差，壳子小了主板是放不进去的。

​            6.主板板厚1.2mm（使用改良版外壳）。拓展板板厚1.2mm。



​       `#②|Metal底座（改进版） #(orange)`

​            1. 适用于Metal版本硬件。

​            2. 底座使用的是3D打印机制作的，建议使用半透明的打印效果。

​            3. 打印出来的表面如果想做成磨砂的效果，推荐使用800目的砂纸在水龙头冲洗下打磨。

​            4. v1.0和v2.0版本均在稚晖君原版的基础上修改的，底部开了内存卡槽。

​            5. v1.0和v2.0适配本文件夹中所有版本的主板和拓展板。推荐适配的PCB板子打样1.2mm厚度的（主板及屏幕版）

​          `#V 1.0的左右加固座偏大,V2.0优化了这个缺陷。 #(yellow)`

​     

​      `#③|Ironman钢铁侠底座 #(orange)`

​         1.适用于Ironman版本硬件。

​         2.稚晖君原版底座推荐：主板板厚1.0mm。拓展板板厚1.0mm。



​     `#④|BiliBili #(orange)`

​         底座暂无。（见群文件）



​     `#⑤|钢铁侠头 #(orange)`

​          底座暂无。开源地址：https://gitee.com/qlexcel/holo-iron-man

## 动态BOM的使用及制作

### 动态BOM的使用



​       `#步骤一 #(yellow)`：打开2.2.2中标注的焊接图文件

![image-20220610214348237](https://pic.imgdb.cn/item/62a40655094754312939b2d0.png![图片](xxx.png#center))

​                                                                                                                                   `#  打开焊接丝印图文件 #(lime)`



​      `#步骤二 #(yellow)`：鼠标点击左边的器件它会显示具体内容（相同元器件也会高亮显示）

![image-20220610214411109](https://pic.imgdb.cn/item/62a4065a094754312939b798.png![图片](xxx.png#center))



​      `#步骤三 #(yellow)`：如果想看元器件的第一Pin可以在右上角设置里设置

![image-20220610214431618](https://pic.imgdb.cn/item/62a40661094754312939be18.png![图片](xxx.png#center))

### 动态BOM的制作



![image-20220610214454029](https://pic.imgdb.cn/item/62a4066a094754312939c9ae.png![图片](xxx.png#center))



![image-20220610214510405](https://pic.imgdb.cn/item/62a40670094754312939cf4a.png![图片](xxx.png#center))



​      注：AD制作动态BOM的脚本链接：https://github.com/lianlian33/InteractiveHtmlBomForAD

​      打不开可以通过网盘下载：https://www.aliyundrive.com/s/f6VVYfwsQ3S

​      或者群文件“开发者工具”里面“InteractiveHtmlBomForAD”

## 改良扩展板介绍

​      `#V 1.5 #(orange)`

​      1、扩展板改良版，增加662k 3.3v 300ma 稳压芯片（分担了屏幕电流，从而减小因主板2992 稳压芯片供电不足导致设备重启）；

​      2、扩展板也适用于MetalV2.0 外壳



![image-20220610214555211](https://pic.imgdb.cn/item/62a40677094754312939d797.png![图片](xxx.png#center))

![image-20220610214602286](https://pic.imgdb.cn/item/62a4067c094754312939dd3a.png![图片](xxx.png#center))



​      `#V 2.0 #(orange)`

​      1、扩展板改良版，增加MP1471 电源管理芯片（减小因主板LP2992 或ME6211稳压芯片供电不足导致设备重启以及发热问题）；

​      2、扩展板适用于MetalV2.0 外壳，Naïve需要改模具

![image-20220610214704307](https://pic.imgdb.cn/item/62a40681094754312939e153.png![图片](xxx.png#center))



## PCB到PCBA的焊接

​      

​      注：参考群文件《主控板焊接问题的一些排查方法》

​      由于主控板元器件的封装为0402，对于非理/工科、没有电子基础、动手能力一般且没有焊接经验的，多少有些困难，至于难易程度就看各位的“身手”了。

​      一般是按照元器件的大小，按照从小到大的顺序去焊接，建议使用拆焊台或者热风枪，不建议单用烙铁焊。没有焊过东西的，或者平 时不焊东西，只进行过电子电工实习的那种，建议买个加热台， 淘宝有 20多块的鹿仙子拆焊台（焊台使用方法请自行 b站）， 这个完全够用（当然土豪可以入个风枪加焊台组合），然后再上 根烙铁就可以。 

​      主控板背面所有的器件（SD卡槽和4个电阻），不焊接的话不会影响板子正常使用，建议等屏幕焊接，测试完毕后再焊接。

​      关于上锡困难的问题：什么都不懂的建议淘宝打个钢网，没搞 过的建议使用群文件打钢网。 用钢网上好锡之后（锡膏中温锡膏就可以，买一小罐 30克就 可以用很久，拼多多20块），按照群里的焊接对照图，把元器件按位置放好，放到焊台上加热就好了，在焊锡融化的过程会 导致元气件的位置有偏差，用镊子调整位置即可。 （注:焊接 esp32的时候，将芯片放好位置，焊锡融化后，关 掉加热台开关，等冷却的时候多用镊子按一按芯片） 

​       PS: C7电容可选1uF~10uF。（尤其是某宝“极客***”上买的，需要注意）

#### 主控板PCB焊接开始



​      `#①|确保电脑端口能够识别 【上电前一定要用万用表测一下电源跟地是否短路】 #(yellow)`

​      `#第一步 #(yellow)`：焊接LDO以及周边电阻电容

​      电阻、电容、三极管(VT1-2，8050)、稳压芯片（LP2992）优先焊接，天线（A1）和灯（D1）可以放在最后焊接。

![image-20220610214819180](https://pic.imgdb.cn/item/62a4068c094754312939ecac.png![图片](xxx.png#center))



​      `#第二步 #(yellow)`：焊接CP2102和Type C接口，

​      第一步焊接完后，准备焊接CP2102（U1）和Type C接口，如果这部分焊接没问题，这个时候板子插上电脑（插电脑前用万用表蜂鸣档测一下VCC（5V）和GND有没有短路），电脑的设备管理器就可以识别出串口了。

![image-20220610214835779](https://pic.imgdb.cn/item/62a40691094754312939f2c7.png![图片](xxx.png#center))

​        注：焊接方向——芯片圆点对PCB白 

![image-20220610214852463](https://pic.imgdb.cn/item/62a40696094754312939f847.png![图片](xxx.png#center))



​      `#②|这部分常见焊接问题 #(yellow)`：



​      `#Ⅰ|插电脑后无任何反应 #(cyan)`

​      大概率是虚焊，首先用肉眼或者放大镜看看是否有连锡，然后在接电脑或者电源的情况下，将万用表调到直流电压档（切记别用欧姆档，谁用谁知道）检查下LP2992（ME6211）的引脚电压，看看有无5V和3.3V出来，然后检查CP2102和Type C，还有R1电阻是不是没焊好，用镊子按一按。

​      `#注：数据线不要用只能充电的那种，要能传输数据的 #(yellow)`



​      `#Ⅱ|插电后电脑提示USB端口有异常电流 #(cyan)`



<img src="https://pic.imgdb.cn/item/62a4069c094754312939fda8.png![图片](xxx.png#center)" alt="image-20220610214915062" style="zoom:67%;" />



​      板子的VCC和GND短路了或者LDO坏了。（为了避免带来意外损失，建议上电前先量电源[5V & 3V3]跟地[GND]是否短路）

​      `#解决方法 #(orange)`：检查板子，重焊或者换一个LDO



​      `#Ⅲ|插电后，电脑识别出CP2102,但是有黄色感叹号 #(cyan)` 

​      `#解决方法 #(orange)`：没安装驱动，群文件有驱动，下载安装驱动。



​     `#Ⅳ|其他 #(cyan)`  

​      可能还是虚焊也有可能方向错了或者芯片坏了，检查下2992的引脚电压，有无5V和3.3V出来，然后检查CP2102和Type C。 

​       `#解决方法 #(orange)`：检测是否是虚焊或着是否是Type-C座子的问题



​      `#Ⅴ| CP2102被驱动被改 #(cyan)`

​      `#解决方法 #(orange)`：



![image-20220610215014567](https://pic.imgdb.cn/item/62a406a109475431293a047d.png![图片](xxx.png#center))

![image-20220610215021227](https://pic.imgdb.cn/item/62a406a709475431293a0ab6.png![图片](xxx.png#center))

![image-20220610215027585](https://pic.imgdb.cn/item/62a406b609475431293a20f6.png![图片](xxx.png#center))



​      `#③|确保ESP32能和串口正常通讯 #(yellow)`

​      `#第三步 #(yellow)`：焊接esp32（U4）芯片



![image-20220610215040004](https://pic.imgdb.cn/item/62a406be09475431293a2950.png![图片](xxx.png#center))

​                                                                                              注：ESP32 焊接方向——芯片圆点对PCB斜边 



​      串口没问题后，开始焊接ESP32，这个应该是板子上最难焊的一块芯片，有时候焊好后，ESP32芯片可以正常工作，可以烧录程序，blink灯可以正常闪烁，但是不代表你焊接的就没问题，依然有可能存在部分IO引脚短路或者虚焊，导致你后期连接屏幕时，屏幕不亮或者显示有问题，甚至MPU6050 无法正常连接等等。

​      注： 

​      **①**  背面的TF卡座和四个10K上拉电阻不影响烧录及使用。

​      **②**  烧录固件请跟群文件最新固件同步，尽量别用“远古”固件

​      **③**  烧录成功了或许不一定能点亮LED（有可能虚焊或灯坏了），但没有成功烧录固件LED是一定不会被点亮的。

​      你的电脑是通过USB →Type C→CP2102→ESP32，进行烧录程序的，中间任何一个环节通信失败，都会导致固件烧写的失败。

​       

​      焊接完了可以先用上位机查看串口是否有串口信息打印，`#查看串口打印的波特率是115200 #(yellow)`（波特率：115200【不是1152000，也不是921600（下载时用921600，当然115200也可以就是比较慢）】），如果焊接没问题（仅仅表示串口能和ESP32正常通讯，不代表其他IO口没问题）就会有串口信息，虚焊是没有的。

​      `#焊接正常的情况下，两个8050集电极，也就是最上面Pin脚的电压在3.3V左右。 #(yellow)`



![image-20220610215100448](https://pic.imgdb.cn/item/62a406c309475431293a2ef1.png![图片](xxx.png#center))

​      

​      关于上面的rst：、boot的相关说明见五、ESP32基础篇。



​      `#④|这部分常见焊接问题  #(yellow)` 

​      `#Ⅰ|插电脑烧录不成功 #(cyan)`

​      上位机卡在最后一点点不动弹 & 使用VS Code 烧录提示connect ESP 32 time out ...---...----...---- 这种情况大多都是ESP32虚焊了，什么连接失败的，或者flash错误的，都是ESP32芯片虚焊，重新焊，锡多（导致引脚黏连）或者锡少（就没焊上）都会导致这种问题，要灵活使用镊子、助焊剂、吸锡带、拆焊台。

​      `#解决方法 #(orange)`：补锡加焊，可以参考首页中视频。（有用或者成功了记得给他们一键三联😁）



![image-20220610215119591](https://pic.imgdb.cn/item/62a406d309475431293a3e6c.png![图片](xxx.png#center))



​      `#Ⅱ|C7 电容是0.1uF #(cyan)`  

​      稚晖君原文件里用的是0.1uf，所以使用稚晖君源文件打板的， C7电容使用的基本为 0.1uF，C7焊的是0.1uF电容，然后连接下载程序失败，但是在下载程序的过程中，短接ESP32的GPIO0引脚后，程序可以正常下载，建议将C7电容改为1uF~10uF。

​      `#解决方法 #(orange)`：将C7电容改为1uF~10uF。



![image-20220610215133241](https://pic.imgdb.cn/item/62a406da09475431293a4694.png![图片](xxx.png#center))



​      `#Ⅲ|其他 #(cyan)` 

​      也有可能是CP2102芯片和R3电阻没焊好，或者两个三极管，或者U2旁边那4个小器件……

​      `#解决方法 #(orange)`：重新上件焊接试试看。



​      `#Ⅳ |烧录后一直重启   #(cyan)`  

​      可能是32其它脚虚焊或者连锡了导致一直重启。能烧录、读取串口只能说明烧录脚没问题了，不代表32其它脚没问题

![image-20220610215159437](https://pic.imgdb.cn/item/62a406ec09475431293a5b22.png![图片](xxx.png#center))

​      `#解决方法 #(orange)`：加焊，用镊子动一动，轻轻压一压将里面多余的锡压出来。



![image-20220610215149636](https://pic.imgdb.cn/item/62a406e109475431293a4e1e.png![图片](xxx.png#center))





​     `#⑤|主控板正面未焊接部分  #(yellow)` 

​     

​      `#第四步 #(yellow)`：焊接正面剩余器件

​      如果你的ESP32可以正常烧录程序，那么你距离成功只剩下最后50%了。

​      MPU6050、FPC 8P座子、RGB灯、2.4G天线、光感（可以不用焊接）

![image-20220610215216982](https://pic.imgdb.cn/item/62a406f809475431293a6a01.png![图片](xxx.png#center))

​      注：MPU 6050焊接方向——圆点对白点；2.4G天线点对横杆；光感AA对白点。 



​     `#⑥|这部分常见问题  #(yellow)`：



​     `#Ⅰ |APP自己切换   #(cyan)`

​    （1） 上电后未初始化完成导致

​      `#解决方法 #(orange)`：断电水平放置等待MPU 6050 初始化完成（灯开始变化）



​    （2） 虚焊

​       如果你MPU6050虚焊了，你烧录群里的测试固件，程序会一直卡在MPU6050的初始化那里，导你开始怀疑人生，是不是自己ESP32芯片又坏了。

​      `#解决方法 #(orange)`：重新加焊。（可以参考神秘藏宝老哥的视频）



​     （3） MPU 6050的旁路电容

​       部分群友出现MPU 6050 附近的电容上错或者虚焊现象。

​     `#解决方法 #(orange)`：加焊MPU 6050（如果加焊不管用可以尝试加焊ESP 32 右下角那两个Pin）以及附近的电容（或者直接换掉重焊也行）。



![image-20220610215235637](https://pic.imgdb.cn/item/62a4070109475431293a73d1.png![图片](xxx.png#center))



​      `#Ⅱ |买到假货MPU6050 & 坏了   #(cyan)`

​      通过串口信息看到MPU 6050 一直报****** 80%可能是坏了或者20%可能是虚焊

![image-20220610215257158](https://pic.imgdb.cn/item/62a4070a09475431293a7ea3.png![图片](xxx.png#center))

​      `#解决方法 #(orange)`：换新的MPU 6050或者加焊（80%的概率是芯片坏了，20%的概率是虚焊了）



​      `#Ⅲ |光感焊接问题   #(cyan)` 

​      通过串口信息看到MPU 6050报

​      [E][esp32-hal-i2c.c:1434]i2cCheckLineState(): Bus Invalid State, TwoWire() Can't init sda=1, scl=0

​      `#解决方法 #(orange)`：确认下光感是否焊接对或者拿掉光感，重新焊接下MPU 6050 看看。（PS：部分群友是因为光感焊接方向不对导致的）

​     

​      下图仅作焊接完后参考

![image-20220610215315275](https://pic.imgdb.cn/item/62a4070e09475431293a83d9.png![图片](xxx.png#center))



#### 拓展板PCB焊接开始



​      扩展板没啥难度，根据材料清单和PCB直接焊接屏幕、FFC和MOS管就好，注意扩展板有两个版本，一个是原版（不建议），另一个是优化版（建议使用群主最新的DC-DC的拓展板【满血版】，ME 6211 +含有622K的优化版【残血版】）。

​       焊接屏幕排线的时候最好不要用拆焊台，直接用烙铁，涂一点焊锡膏就好，不然容易烧屏幕。



![image-20220610215342877](https://pic.imgdb.cn/item/62a40681094754312939e153.png![图片](xxx.png#center))

​      MP1471焊接时横线方向向下

​      这部分常见问题

​      屏幕板只有背光

​      请按照群文件的参考电压来测量下看看，一步一步排查。

![image-20220610215401098](https://pic.imgdb.cn/item/62a4071d09475431293a921c.png![图片](xxx.png#center))

![image-20220610215408851](https://pic.imgdb.cn/item/62a4072409475431293a9a2f.png![图片](xxx.png#center))

​    

​        座子焊接一定要小心！！！ 

![image-20220610214236590](https://pic.imgdb.cn/item/62a4064a094754312939a75c.png![图片](xxx.png#center))

# 第三章  上位机相关问题

## 上位机显示不全

​      出现问题：Windows 11或 Windows 10 打开上位机可能会出现如下图所示的情况

​      解决方法：在电脑分辨率的界面，调缩放比例。然后重新打开上位机，如果没有变化，重启电脑再打开上位机试试。

![image-20220610215502037](https://pic.imgdb.cn/item/62a4073009475431293aa7cf.png![图片](xxx.png#center))

## 上位机串口信息

### MPU 6050 初始化失败



​      出现问题：MPU 6050 初始化失败或者无连接

​      解决方法：MPU 6050 虚焊了，加焊位置见上面主板焊接。

![image-20220610215530444](https://pic.imgdb.cn/item/62a4073709475431293ab037.png![图片](xxx.png#center))

### TF卡无法读取（TF卡座电压图）



​      出现问题：TF卡无法正常读取到 或者 之前能够识别到一段时间后无法识别了。



![image-20220610215549378](https://pic.imgdb.cn/item/62a4073b09475431293ab507.png![图片](xxx.png#center))

![image-20220610215556324](https://pic.imgdb.cn/item/62a4074109475431293abb3e.png![图片](xxx.png#center))

​      解决方法： 首先先确认是不是内存卡（TF卡，注：TF卡最大支持32G）的问题，如果不是，那么就有可能是虚焊的问题，如下图，可能是电阻或者ESP32对应的某一个或者多个Pin脚虚焊了，需要加焊！



![image-20220610215610255](https://pic.imgdb.cn/item/62a4074609475431293ac050.png![图片](xxx.png#center))

### 无法烧录



​      出现问题：无法清空，进度条卡在最后一点儿，没有串口信息（波特率：115200【不是1152000，也不是921600（下载时用921600，当然115200也可以就是比较慢）】）

​      解决方法： ESP 32虚焊了，需要加焊。（稚晖君原版请将C7电容改为1-10uF）请参考前面主板焊接。

![image-20220610215629658](https://pic.imgdb.cn/item/62a406d309475431293a3e6c.png![图片](xxx.png#center))

# 第四章  如何使用万用表



<img src="https://pic.imgdb.cn/item/62a4075609475431293ad29a.png![图片](xxx.png#center)" alt="image-20220610215653753" style="zoom:80%;" />

​      以福禄克为例，如上图所示，万用表的外圈一般会分为六个部分：交流电流档（A~）、直流电流档（A-）、交流电压档（V~）、直流电压档（V-）、电阻档（Ω）、蜂鸣档（🔈符号我打不出来，自己看图吧）。

## 测电压（以小电视主板为例）【一定要是直流电压档】



​      ①  将万用表打到直流挡（有挡位的请自行选择合适的挡位，一般从大到小选）

​      ②  数字万用表的一只表笔接地，一只接待测Pin脚或焊盘（如图，一只表笔放在Type-C上，一只表笔放到LDO的5V Pin脚上）。

​      注：指针万用表一定要注意正负极，避免万用表损坏。

![image-20220610215717268](https://pic.imgdb.cn/item/62a4075b09475431293ad994.png![图片](xxx.png#center))

## 测通断（以小电视主板为例）【一定要断电】



​      ①  将万用表打到蜂鸣挡 （测试短路或者通路）

​      ②  假如是小电视的电源（VCC）跟地（GND）短路了（一只表笔放到GND上，一只表笔放到5V或者3V3上） ，如果有蜂鸣声就表示短路（通路）了，反之则无。如果检查元器件是否坏了（比如电阻、电容等），将两只表笔分别放置在元器件两端，有蜂鸣声表示坏了（0Ω这种除外），反之则无。

![image-20220610215736025](https://pic.imgdb.cn/item/62a4076109475431293adfee.png![图片](xxx.png#center))

# 第五章  ESP32 基础篇: 启动时 rst cause 和 boot mode



​      在 ESP32 启动时, ROM CODE 会读取 GPIO 状态和 rst cause 状态, 进而决定 ESP32 工作模式。

​      通过了解和掌握 rst cause 和 boot mode, 有助于定位某些系统问题。

​      例如:

​      ESP32 启动时会有如下打印:

​      rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)

​      ROM CODE 波特率为 115200

​      系统复位介绍和 ESP32 启动时 rst cause

​      系统复位介绍

​      系统提供三种级别的复位方式,分别是 CPU 复位，内核复位，系统复位。

​      所有的复位都不会影响 MEM 中的数据。如图展示了整个子系统的结构以及每种复位方式:

![image-20220610215813843](https://pic.imgdb.cn/item/62a4076609475431293ae56c.png![图片](xxx.png#center))

​      CPU 复位：只复位 CPU 的所有寄存器。

​      内核复位：除了 RTC,会把整个 digital 的寄存器全部复位,包括 CPU、所有外设和数字 GPIO。

​      系统复位：会复位整个芯片所有的寄存器,包括 RTC。

​      大多数情况下, APP_CPU 和 PRO_CPU 将被立刻复位,有些复位源只能复位其中一个。

​      APP_CPU 和 PRO_CPU 的复位原因也各自不同：

​      当系统复位起来之后，

​      PRO_CPU 可以通过读取寄存器 RTC_CNTL_RESET_CAUSE_PROCPU 来获取复位源；

​      APP_CPU 则可以通过读取寄存器 RTC_CNTL_RESET_CAUSE_APPCPU 来获取复位源。

​      ESP32 启动时 rst cause

​      例如 ESP32 启动时会有如下打印:

​      rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)

![image-20220610215828960](https://pic.imgdb.cn/item/62a4076b09475431293aea96.png![图片](xxx.png#center))

​      ESP32 上电 boot mode

​      ESP32 上电时会判断 strapping 管脚的状态, 并决定 boot mode.

​      例如常见的两种上电打印:

​      下载固件模式：

​       rst:0x1 (POWERON_RESET),boot:0x3 (DOWNLOAD_BOOT(UART0/UART1/SDIO_REI_REO_V2)) 

​       芯片运行模式：

​       rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)

​       以 ESP32-WROOM-32 为例说明。

​       boot 值由 strapping 管脚 的 6 位值 [MTDI, GPIO0, GPIO2, GPIO4, MTDO, GPIO5] 共同决定。

​       文章链接：https://blog.csdn.net/espressif/article/details/107977962



​                                                                                                            祝各位好运！



# 第六章  附录 （PCB连线图）

## 主板



![image-20220610215938488](https://pic.imgdb.cn/item/62a4077409475431293af45f.png![图片](xxx.png#center))

![image-20220610215946199](https://pic.imgdb.cn/item/62a4087c09475431293c14e5.png![图片](xxx.png#center))

![image-20220610215952810](https://pic.imgdb.cn/item/62a4077809475431293afa5a.png![图片](xxx.png#center))

![image-20220610215959236](https://pic.imgdb.cn/item/62a4077d09475431293b00c7.png![图片](xxx.png#center))

![image-20220610220005265](https://pic.imgdb.cn/item/62a4085009475431293bdf91.png![图片](xxx.png#center))

![image-20220610220011824](https://pic.imgdb.cn/item/62a4078209475431293b0634.png![图片](xxx.png#center))



![image-20220610220018753](https://pic.imgdb.cn/item/62a4082c09475431293bb7fa.png![图片](xxx.png#center))

![image-20220610220024328](https://pic.imgdb.cn/item/62a4078509475431293b0cc1.png![图片](xxx.png#center))

## 拓展板

​      `#①  2.0版本 #(yellow)`

![image-20220610220056134](https://pic.imgdb.cn/item/62a4078b09475431293b13ad.png![图片](xxx.png#center))

![image-20220610220102884](https://pic.imgdb.cn/item/62a4079109475431293b1a19.png![图片](xxx.png#center))

​      `#①  1.5版本 #(yellow)`

![image-20220610220117345](https://pic.imgdb.cn/item/62a4079e09475431293b267c.png![图片](xxx.png#center))

![image-20220610220122839](https://pic.imgdb.cn/item/62a4079d09475431293b262f.png![图片](xxx.png#center))

​      `#①  1.2版本 #(yellow)`

![image-20220610220134279](https://pic.imgdb.cn/item/62a407a209475431293b2b89.png![图片](xxx.png#center))

![image-20220610220140492](https://pic.imgdb.cn/item/62a407a309475431293b2cc6.png![图片](xxx.png#center))
