## **关于此库 About this repository** ##

Mind+库中提取的lot库，直接丢在Arduino IDE里面跑mind+生成的代码会报错，此库为魔改版，物理去除了BUG。

----------

The wifi lot library extracted from the Mind+ will report errors directly when running in Arduino IDE. This library is a modified version, which solves the bug.

----------
![image](https://github.com/user-attachments/assets/32f62415-3eb2-4c28-a06a-3ac59b24cdfc)

WIFI IoT Module是一款支持多编程平台，多物联网平台的WIFI模块。它支持micro:bit、Arduino主控，MakeCode、Mind+、ArduinoIDE多种编程环境；支持EasyIoT、IFTTT、Thingspeak等多种流行的物联网平台。

WIFI IoT Module采用易用的Gravity接口，I2C和URAT两种通讯协议，UART模式兼容OBLOQ物联网模块的功能和用法，I2C模式兼容更多主控板，并可以最大限度避免传感器之间的冲突。不论是用于课堂IoT教学，还是家电的物联网改装，选用WIFI IoT Module都是非常好的方案。

The WIFI IoT Module is a Wi-Fi module that supports multiple programming platforms and IoT platforms. It is compatible with micro:bit and Arduino controllers and can be programmed using various environments such as MakeCode, Mind+, and Arduino IDE. Additionally, it supports popular IoT platforms like EasyIoT, IFTTT, and ThingSpeak.

The WIFI IoT Module features an easy-to-use Gravity interface and supports both I2C and UART communication protocols. In UART mode, it is fully compatible with the OBLOQ IoT module in terms of functionality and usage. In I2C mode, it offers broader compatibility with various controllers while minimizing conflicts between sensors. Whether for IoT education in classrooms or smart home modifications, the WIFI IoT Module is an excellent choice.

| Dfrobot Wifi Lot Module不同版本对比 | 官方版本                                                                            | Mind+版                                | Mind+魔改版                              |
| -------------------------- | ------------------------------------------------------------------------------- | ------------------------------------- | ------------------------------------- |
| 特点                         | 仅适用于官方Wiki提供的示例代码                                                               | **无法**运行官方Wiki提供的示例代码，**无法**在IDE中运行Mind+生成的代码 | **无法**运行官方Wiki提供的示例代码，**可以在IDE中运行Mind+生成的代码** |
| 链接/来源                      | https://github.com/DFRobot/DFRobot_WiFi_IoT_Module?tab=MIT-1-ov-file#readme     | 下载mind+提取                             | 本仓库                                   |


| Dfrobot Wifi Lot Module Versions Comparison | Official Version                                                                      | Mind+ Version                           | Mind+ Modified Version                       |
| ------------------------------------------- | ------------------------------------------------------------------------------------- | --------------------------------------- | -------------------------------------------- |
| Features                                    | Only applicable for the example code provided in the official Wiki                    | **Cannot** run the example code from the official Wiki, **cannot** run Mind+ code in IDE | **Cannot** run the example code from the official Wiki, **can run Mind+ code in IDE** |
| Link/Source                                 | https://github.com/DFRobot/DFRobot_WiFi_IoT_Module?tab=MIT-1-ov-file#readme           | Downloaded from Mind+                    | This repository                               |


----------
本库仅适用于在arduino ide中运行mind+生成的代码

This library is only suitable for running mind+ generate code in the Arduino IDE 

----------
如何安装库？How to install a library?
https://mc.dfrobot.com.cn/thread-1854-1-1.html#pid6955
