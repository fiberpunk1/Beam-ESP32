# Fiberpunk-Beam-v1.0用户使用手册

![board](Images/board.png)

## 1. 快速开始

1. 配置你的SD卡
使用Beam之前，必须要设置一下你的SD卡。
拔下SD，插到电脑USB口，然后打开BeamManager软件，点击设置，步骤入下图所示:

点击Setting:

![setting](Images/1-1.png)

下面是配置文件:

![setting](Images/1-3.png)

单击Updata Mounted SD来刷新已经插入的SD卡盘符，然后在这个按键下面的下拉框中，选择SD所在的位置。
此处我们要设置的功能表如下:

- Time Laspe, 是否开启延时摄影
- SSID: 你的wifi名
- Password: Wifi密码
- Name: 为你的打印机设置一个本地唯一的名字
- Stop X: 拍摄延时摄影时，喷头停止的X坐标
- Stop Y: 拍摄延时摄影时，喷头停止的Y坐标
- React Length: 延时摄影动作结束后，回抽的距离

设置完成后，将SD卡插入到Beam的SD卡槽中，给Beam上电。

2. 上电, 连接wifi以及打印机

Beam上电后，手动按下Start 按键，接下来看到状态灯亮红灯，说明SD卡初始化成功，如果红灯闪烁，说明没有找到SD卡，请检查您的SD卡是否正确插入。红灯亮起后，过一小段时间，如果绿灯亮了，说明Beam已经正确连接上了您的wifi，您可以通过BeamManager来对他进行一些操作了。

接下来，将您的打印机，通过USB线，与Beam的USB母口连接，连接后，在Beam的主界面，点击Search Device, BeamManager开始扫描局域网中的Beam模块。

![扫描](Images/1-4.png)

扫描完成后，会弹出一个Beam操控界面。 如果您的局域网中包含了多个Beam模块，并且都正确配置了Wifi，那么Beam的界面中会同时出现多个操作窗口。

![操作界面](Images/1-op-1.png)

上图中框选的DeviceName,就是之前您在SD卡中初始化的名称，这样做是为了让您更好的区分各台打印机。 

在指令发送栏敲入G28，发送三次，查看打印机是否能正常复位(发送控制指令前，请确保您的打印机已经正常上电，能够运动)。

3. 发送文件，控制打印

点击SendFile按键，选择您要发送的Gcode文件，就可以将文件发给Beam模块了。

![图片](Images/sendingfile.png)

上传完成后，单击FileList，可以获得Beam中的文件列表。

![图片](Images/filelist.png)

在list中您可以选中对应文件进行删除。 注意不要删除config.txt这个文件。

选中您上传的打印文件，就可以开始打印了。

4. Time Lapse

如果您开启了Time Lapse，那么Beam会控制打印机在每一层结束后，移动到您设置的地点，向外发射一个拍摄信号。 我们的软件和硬件可支持如下三种方式来触发拍摄(到本版本为止，只测试了第一种方式)。

- Beam发送网络信号给BeamManager，BeamManager控制电脑上的USB摄像头，拍照(已测试)
- Beam发送串口信号，告知外界当前需要拍照(未测试)
- Beam发送网络信号给Wifi摄像头，让wifi摄像头拍摄(未测试)

这里我们介绍第一种方式，使用USB摄像头拍摄。
在您的打印机控制面板中，单击CameraSetting，开启一个USB摄像头界面，如下图所示:

![camera](Images/usb-camera.png)

在上面的界面中，单击update camera list，当您有新插入的摄像头时，可以重新获取到摄像头列表。 在下拉列表中选中您的摄像头，单击capture test，实现拍摄测试，拍摄的照片会在右侧显示出来。 set save path，设置图片的保存位置。
当所有的图片拍摄完成之后，单击convert to mp4，可以实现将图片导出为mp4。这里实际上是调用了fffmpeg.exe，如果您有更细致的图片转视频需求，您可以查看fffmpeg这个工具的用法。我们也会在后续版本中，加入更多这方面的参数设置。

> 注意: 使用当前的版本v0.1.0,开启的camera setting不要关闭这个窗口，让他保持在后台运行

## 2. 设置技巧


## FAQ

1. 我的wifi连不上，是为什么？

Beam模块是基于ESP32实现的网络IOT模块，目前我们只能支持2.4G网络，如果是5G的wifi我们不支持。另外，如果wifi信号太弱，也会导致wifi无法连接。 如果您是只有单个普通家用路由器，我建议Beam距离路由器的距离不要超过8米，中间隔墙不要过多。

2. Beam的固件如何升级？

Beam的固件是开源的，您可以参考开发文档，来对固件进行升级。




