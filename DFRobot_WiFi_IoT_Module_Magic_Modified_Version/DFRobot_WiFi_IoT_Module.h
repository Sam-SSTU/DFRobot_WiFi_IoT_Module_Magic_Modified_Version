#ifndef _DFROBOT_WIFI_IOT_MODULE_H
#define _DFROBOT_WIFI_IOT_MODULE_H

#include <Arduino.h>
#include <Wire.h>
#include <SimpleList.h>

extern bool OBLOQ_DEBUG; //针对不支持调试信息的主板，需定义OBLOQ_DEBUG true打印信息

/**************************以下这些值禁止修改，修改后需要自行调试适配****************************/
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) 
#include "SCoop.h"
#define IOT_MAX_MESSAGE 32
#define IICREADDATAMAX  32
#define ENABLE_ALLOCATE_MEM 0   //mqtt消息内存分配模式
#define IOTDEBUG 0              //打印调试信息标志
#elif defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_LEONARDO)
#include "SCoop.h"
#define IOT_MAX_MESSAGE 32
#define IICREADDATAMAX  32
#define ENABLE_ALLOCATE_MEM 0   
#define IOTDEBUG 0
#elif defined(NRF5) || defined(NRF52833)
#define IOT_MAX_MESSAGE 128
#define IICREADDATAMAX  200
#define ENABLE_ALLOCATE_MEM 1   
#define IOTDEBUG 1
#elif defined(ARDUINO_ARCH_RP2040)
#include <FreeRTOS.h>
#include <task.h>
#define IOT_MAX_MESSAGE 128
#define IICREADDATAMAX  200
#define ENABLE_ALLOCATE_MEM 1   
#define IOTDEBUG 1
#else
#include "SCoop.h"
#define IOT_MAX_MESSAGE 32
#define IICREADDATAMAX  32
#define ENABLE_ALLOCATE_MEM 0   
#define IOTDEBUG 0
#endif
/********************************************************************************************/
#define SUBSCRIBE_UPPER_LIMIT 5
#define IOT_TOPIC_MAX_LEN 20 //topic的最大长度
/**********调试时手动控制 最后记得改为0**********/
#define ENABLE_SOFTSERIALPRINT 0 //串口模式使用软串口打印信息
#define ENABLE_DBGIOTSTATUS 0 //打印 iot状态
/*********************************/
typedef void (*MqttHandle)(const char *message);
typedef void (*DebugHandle)(const char *message, int8_t code);



class DFRobot_WiFi_IoT_Module
{
public:
    enum Topicnum
    {
        topic_0,
        topic_1,
        topic_2,
        topic_3,
        topic_4
    };
    virtual void subTopic() = 0;
    virtual void publish(Topicnum topic_num, const String &message) = 0;
    virtual void registerMqttHandle(const MqttHandle handles[]) = 0;
    virtual void registerDebugHandle(const DebugHandle handle = NULL) = 0;
    virtual void receiveObloqData() = 0;
};



class DFRobot_WiFi_IoT_Module_I2C:public DFRobot_WiFi_IoT_Module
{
public:
    #define    IOT_COMMAND_REGTSTER  0X1E           //Command register

    /* Register command*/ 

    #define    IOT_RUN_COMMAND        0X02           //WIFI_IOT running command
    #define    IOT_READ_COMMAND       0X00           //WIFI_IOT read command
    #define    IOT_SET_COMMAND        0X01           //WIFI_IOT setup command
    #define    IOT_TYPE_REGISTER      0X1F           //Parameter type register

    /* Parameter type setting */ 

    #define    SET_WIFI_NAME          0X01           
    #define    SET_WIFI_PASSWORLD     0X02
    #define    SET_MQTT_SERVER        0X03
    #define    SET_MQTT_PORT          0X04
    #define    SET_MQTT_ID            0X05
    #define    SET_MQTT_PASSWORLD     0X06
    #define    SET_HTTP_ID            0X07
    #define    SET_HTTP_PORT          0X08
    #define    SET_MQTT_DEVICEID      0x09

    #define    SEND_PING              0X01
    #define    CONNECT_WIFI           0X02
    #define    RECONNECT_WIFI         0X03
    #define    BREAK_WIFI_CON         0X04
    #define    CONNECT_MQTT           0X05
    #define    SUBSCRIBE_TOPIC0       0X06
    #define    SUBSCRIBE_TOPIC1       0X07
    #define    SUBSCRIBE_TOPIC2       0X08
    #define    SUBSCRIBE_TOPIC3       0X09
    #define    SUBSCRIBE_TOPIC4       0X0A
    #define    SEND_TOPIC0            0X0B
    #define    SEND_TOPIC1            0X0C
    #define    SEND_TOPIC2            0X0D
    #define    SEND_TOPIC3            0X0E
    #define    SEND_TOPIC4            0X0F
    #define    HTTP_GET_URL           0X10
    #define    HTTP_POST_URL_CON      0X11
    #define    HTTP_PUT_URL_CON       0X12
    #define    QUERY_VERSION          0X13
    #define    CONNECT_MQTT_ONENET    0x14
    #define    DISCONNECT_MQTT        0x15

    #define    READ_COMMAND           0X06           //Read register

    #define    IOT_LEN_REGISTER       0X20           //Register for storing parameter length 

    /* The parameter type of the returned value */ 

    #define    NONE                   0X00
    #define    PING_STATUE            0X01
    #define    WIFI_STATUE_CON        0X02
    #define    IP_ADDRESS             0X03
    #define    MQTT_STATUE_CON        0X04
    #define    SUBSCRIBE_STATUE       0X05
    #define    SUBSCRIBE_TOPIC4       0X0A
    #define    HTTP_NORMAL_RETURN     0X10
    #define    HTTP_ERROR_RETURN      0X11
    #define    GET_VERSION            0X12
    #define    GET_MQTTSEND_STATUE    0X13

    /* Data register */ 

    #define    READ_LEN_REGISTER      0X21
    #define    READ_DATA_REGISTER     0X22

    // obliq 运行状态
    #define     IOT_RUN_STATE_NONE                      0
    #define     IOT_RUN_STATE_PING_SUCCESS              1
    #define     IOT_RUN_STATE_PING_FAIL                 2
    #define     IOT_RUN_STATE_WIFI_CONNECTING           3
    #define     IOT_RUN_STATE_WIFI_SUCCESS              4
    #define     IOT_RUN_STATE_WIFI_CONNECT_ERR          5
    #define     IOT_RUN_STATE_READ_IP                   6
    #define     IOT_RUN_STATE_MQTT_CONNECTING           7 
    #define     IOT_RUN_STATE_MQTT_SUCCESS              8
    #define     IOT_RUN_STATE_MQTT_ERR                  9
    #define     IOT_RUN_STATE_MQTT_SUBSCIBE_SUCCESS     10
    #define     IOT_RUN_STATE_MQTT_SUBSCIBE_ERR         11
    #define     IOT_RUN_STATE_MQTT_SUBSCIBE_LIMIT       12 
    #define     IOT_RUN_STATE_MQTT_SEND_SUCCESS         13
    #define     IOT_RUN_STATE_MQTT_SEND_ERR             14 
    #define     IOT_RUN_STATE_MQTT_RECEIVE_SUCCESS      15
    #define     IOT_RUN_STATE_HTTP_NORMAL_RETURN        16
    #define     IOT_RUN_STATE_HTTP_ERROR_RETURN         17 
    #define     IOT_RUN_STATE_VERSION                   18
    #define     IOT_RUN_STATE_IIC_READ_ERR              19 
    #define     IOT_RUN_STATE_MEM_ALLOCATE_FAIL         20
    #define     IOT_RUN_STATE_MQTT_RECEIVE_MESSAGE_LEN  21
    #define     IOT_RUN_STATE_MESSAGE_RECEIVE_FAILURE   12
    #define     IOT_RUN_STATE_FALG                      23

    uint8_t IPaddress[14]={0};

    DFRobot_WiFi_IoT_Module_I2C(TwoWire *pWire = &Wire, uint8_t address = 0x16);
    ~DFRobot_WiFi_IoT_Module_I2C();
    void startConnect(const String &ssid, const String &pwd, const String &iotId, const String &iotPwd, const String iotTopics[], const String &server, uint32_t port);
    void startConnect(const String &ssid, const String &pwd, const String &ip, uint32_t port);
    
    void subTopic();
    void publish(Topicnum topic_num, const String &message);
    void publish(Topicnum topic_num, int32_t i);
    void publish(Topicnum topic_num, double f);
    void publish(Topicnum topic_num, float f)  { publish(topic_num, (double)f); };
    void publish(Topicnum topic_num, int i) { publish(topic_num, (int32_t)i); }
    void publish(Topicnum topic_num, uint32_t i) { publish(topic_num, (int32_t)i); };
    void publish(Topicnum topic_num, uint16_t i) { publish(topic_num, (int32_t)i); };
    void publish(Topicnum topic_num, uint64_t i) { publish(topic_num, (int32_t)i); };
    
    void registerMqttHandle(const MqttHandle handles[]);
    void registerDebugHandle(const DebugHandle handle = NULL);
    void receiveObloqData();
    
    char *get(const String &url, int timeOut);
    char *post(const String &url, const String &data, int timeOut);
    char *put(const String &url, const String &data, int timeOut);
    char *getVersion();

private:
    TwoWire *_pWire;
    uint8_t _address;
    uint8_t obloqStatus;
    uint8_t wifiState;
    uint8_t mqttState;
    uint8_t buffer[2]={0};

    SimpleList<String> mqttTopicArray;
    MqttHandle mqttHandle[SUBSCRIBE_UPPER_LIMIT] = {NULL,NULL,NULL,NULL,NULL};
    DebugHandle debugHandle;
#if ENABLE_ALLOCATE_MEM 
    uint8_t *iotRecvData;
#else
    uint8_t iotRecvData[IOT_MAX_MESSAGE+3];
#endif

    void begin(void);
    void connectWifi(const String &ssid, const String &pwd);
    void MQTTBegin(const String &server, const String &port, const String &productID, const String &pwd, char* deviceID = NULL);
    void HTTPBegin(const String &ip, const String &port);
    void iotDebug(int8_t code, uint8_t* data = NULL);
    void loop(uint8_t status, int timeOut = 20000);

    void clearBuffer(void);
    void connection(uint8_t command);
    void manageFunction(uint8_t command, uint8_t config, String data);
    void manageFunction(uint8_t command, uint8_t config, uint8_t *data, uint16_t len);
    void writeReg(uint8_t reg, void *pBuf, size_t size);
    uint8_t readReg(uint8_t reg, void *pBuf, size_t size);
    uint8_t getData(uint8_t reg, void *pBuf, size_t size);
};


class DFRobot_WiFi_IoT_Module_UART:public DFRobot_WiFi_IoT_Module
{
public:
    //Type of returned message
    #define SYSTEMTYPE '1'
    #define WIFITYPE   '2'
    #define HTTPTYPE   '3'
    #define MQTTTYPE   '4'

    //Message status returned by the system
    #define SYSTEMPING      '1'
    #define SYSTEMVERSION   '2'
    #define SYSTEMHEARTBEAT '3'

    //Each sub state in WiFi connected state
    #define WIFIDISCONNECT    '1'
    #define WIFICONNECTING    '2'
    #define WIFICONNECTED     '3'
    #define WIFICONNECTFAILED '4'


    enum ObloqErrorCode {
        OBLOQ_OK                     =  0,
        OBLOQ_SERIAL_CONNECT_FAILURE = -1,
        OBLOQ_MQTT_CONNECT_FAILURE   = -2,
        OBLOQ_WIFI_CONNECT_FAILURE   = -3,
        OBLOQ_SUB_FAILURE            = -4,
        OBLOQ_PULISH_FAILURE         = -5,
        OBLOQ_STEPS_ERROR            = -6
    };
    enum ObloqStatusInfo {
        OBLOQ_NONE               = 1,
        OBLOQ_MQTT_CONNECT_OK    = 2,
        OBLOQ_SUB_OK             = 3,
        OBLOQ_PING_OK            = 4,
        OBLOQ_PULISH_OK          = 5,
        OBLOQ_MQTT_DISCONNECT_OK = 6,
        OBLOQ_GET_VERSION_OK     = 7,
        OBLOQ_WIFI_CONNECT_OK    = 8,
        OBLOQ_HTTP_RECEIVE_OK    = 9,
        OBLOQ_MQTT_RECEIVE_OK    = 10,
        OBLOQ_MESSAGE_FLAG       = 11,
    };

    uint8_t IPaddress[14]={0};

    DFRobot_WiFi_IoT_Module_UART();
#if ENABLE_SOFTSERIALPRINT
    DFRobot_WiFi_IoT_Module_UART(Stream *softserial);
#endif
    ~DFRobot_WiFi_IoT_Module_UART();
    
    void startConnect(uint8_t receive, uint8_t send, const String& ssid, const String& pwd, const String& iotId, const String& iotPwd, const String iotTopics[], const String& server, uint32_t port);
    void startConnect(Stream *serial, const String& ssid, const String& pwd, const String& iotId, const String& iotPwd, const String iotTopics[], const String& server, uint32_t port);
    void startConnect(uint8_t receive, uint8_t send, const String& ssid, const String& pwd, const String& ip, uint32_t port);
    void startConnect(Stream *serial, const String& ssid, const String& pwd, const String& ip, uint32_t port);

    void subTopic();
    void publish(Topicnum topic_num, const String &message);
    void publish(Topicnum topic_num, int32_t i);
    void publish(Topicnum topic_num, double f);
    void publish(Topicnum topic_num, float f)  { publish(topic_num, (double)f); };
    void publish(Topicnum topic_num, int i) { publish(topic_num, (int32_t)i); }
    void publish(Topicnum topic_num, uint32_t i) { publish(topic_num, (int32_t)i); };
    void publish(Topicnum topic_num, uint16_t i) { publish(topic_num, (int32_t)i); };
    void publish(Topicnum topic_num, uint64_t i) { publish(topic_num, (int32_t)i); };

    void registerMqttHandle(const MqttHandle handles[]);
    void registerDebugHandle(const DebugHandle handle = NULL);
    void receiveObloqData();

    char *get(const String &url, int timeOut);
    char *post(const String &url, const String &data, int timeOut);
    char *put(const String &url, const String &data, int timeOut);
    char *getVersion();
    
private: 

    Stream *_s;
    Stream *printSerial;
    uint8_t con = 0;
    int8_t obloqStatus;
    uint8_t singleHardSerialFlag;     //单硬串口标志，等于1时 使用串口发送数据，禁止打印信息，可以使用软串口打印
    uint8_t softSerialFlag;
    SimpleList<String> mqttTopicArray;
    String httpIp = "0";
    MqttHandle mqttHandle[SUBSCRIBE_UPPER_LIMIT] = {NULL,NULL,NULL,NULL,NULL};
    DebugHandle debugHandle;
    uint8_t iotRecvData[IOT_MAX_MESSAGE+IOT_TOPIC_MAX_LEN+10];
    void begin(void);
    void connectWifi(const String &ssid, const String &pwd);
    void MQTTBegin(const String &server, const String &port, const String &productID, const String &pwd, char* deviceID = NULL);
    void HTTPBegin(const String &ip, const String &port);
    void iotDebug(int8_t code, uint8_t* data = NULL);
    uint8_t loop(uint8_t status, int timeOut = 20000);

    void argumentParsing(char *data, uint8_t len);
    uint8_t isIP(const char str[]);
    void saveTopic(const String iotTopics[]);
    void serialSend(const String &msg);
    void init(int8_t receive = -1, int8_t send = -1);
#if defined(NRF5) || defined(NRF52833)
    void serialInit(uint8_t receive, uint8_t send);
#endif
};
#endif