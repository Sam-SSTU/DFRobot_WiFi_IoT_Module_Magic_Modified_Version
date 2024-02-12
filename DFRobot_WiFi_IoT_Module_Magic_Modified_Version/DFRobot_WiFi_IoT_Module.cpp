#include <DFRobot_WiFi_IoT_Module.h>
#include <string.h>

DFRobot_WiFi_IoT_Module *activeObject = NULL;
bool OBLOQ_DEBUG = false;

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_AVR_MEGA2560)
defineTaskLoop(myTask1)
{ 
    //obloqScoopEnableFlag = 1; //函数主循环增加delay,解决接收消息异常现象
    sleep(50);
    activeObject->receiveObloqData();  
}
#elif defined(NRF5) || defined(NRF52833)
static void myTask1(){
    while(true){
        delay(50);
        activeObject->receiveObloqData();
    }
}
static void mqttTick(MicroBitEvent e)
{
    activeObject->receiveObloqData();
}
#elif defined(ARDUINO_ARCH_RP2040)
static void myTask1(void *param){
    while(true){
        // Serial.println("myTask1");
        delay(50);
        activeObject->receiveObloqData();
    }
    
}
#endif


DFRobot_WiFi_IoT_Module_I2C::DFRobot_WiFi_IoT_Module_I2C(TwoWire *pWire, uint8_t address)
{
    _pWire   = pWire;
    _address = address;
    activeObject = this;
    debugHandle = NULL;
    obloqStatus = IOT_RUN_STATE_NONE; 
    wifiState = 0;
    mqttState = 0;
#if ENABLE_ALLOCATE_MEM
    iotRecvData = NULL;
#else
    memset(iotRecvData, 0, IOT_MAX_MESSAGE+3);
#endif
    
};
DFRobot_WiFi_IoT_Module_I2C::~DFRobot_WiFi_IoT_Module_I2C()
{
#if ENABLE_ALLOCATE_MEM 
    if (iotRecvData){
        iotRecvData = NULL;
    }
#endif    
      
};

void DFRobot_WiFi_IoT_Module_I2C::startConnect(const String &ssid, const String &pwd, const String &iotId, const String &iotPwd, const String iotTopics[], const String &server, uint32_t port)
{
    _pWire->begin();
    // begin();
    // WiFi连接
    manageFunction(IOT_SET_COMMAND, SET_WIFI_NAME, ssid);
    manageFunction(IOT_SET_COMMAND, SET_WIFI_PASSWORLD, pwd);
    connection(CONNECT_WIFI);

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_AVR_MEGA2560)
    mySCoop.start();
    mySCoop.yield();
#elif defined(NRF5) || defined(NRF52833)
    create_fiber(myTask1);
#elif defined(ARDUINO_ARCH_RP2040)
    xTaskCreate(myTask1, "myTask1", 256, NULL, 1, nullptr);
#endif

    loop(IOT_RUN_STATE_WIFI_SUCCESS);
    loop(IOT_RUN_STATE_READ_IP);
    // connectWifi(ssid, pwd);
    //MQTT初始化
    MQTTBegin(server, String(port), iotId, iotPwd);
    for (uint8_t i = 0; i < SUBSCRIBE_UPPER_LIMIT; ++i)
    {
        if (iotTopics[i] != ""){
            mqttTopicArray.push_back(iotTopics[i]);
        }
    }
    // 订阅Topic
    subTopic();
}

void DFRobot_WiFi_IoT_Module_I2C::startConnect(const String &ssid, const String &pwd, const String &ip, uint32_t port)
{
    _pWire->begin();
    // begin();
    // WiFi连接
    manageFunction(IOT_SET_COMMAND, SET_WIFI_NAME, ssid);
    manageFunction(IOT_SET_COMMAND, SET_WIFI_PASSWORLD, pwd);
    connection(CONNECT_WIFI);

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_AVR_MEGA2560)
    mySCoop.start();
    mySCoop.yield();
#elif defined(NRF5) || defined(NRF52833)
    create_fiber(myTask1);
#elif defined(ARDUINO_ARCH_RP2040)
    xTaskCreate(myTask1, "myTask1", 256, NULL, 1, nullptr);
#endif

    loop(IOT_RUN_STATE_WIFI_SUCCESS);
    loop(IOT_RUN_STATE_READ_IP);

    HTTPBegin(ip, String(port));
}


void DFRobot_WiFi_IoT_Module_I2C::begin(void)
{
    uint8_t buffer[2];
    _pWire->begin();
    if(readReg(0,buffer, 2) == 2 ){
        clearBuffer();
    }
}

void DFRobot_WiFi_IoT_Module_I2C::connectWifi(const String &ssid, const String &pwd)
{
    manageFunction(IOT_SET_COMMAND, SET_WIFI_NAME, ssid);
    manageFunction(IOT_SET_COMMAND, SET_WIFI_PASSWORLD, pwd);
    connection(CONNECT_WIFI);
    loop(IOT_RUN_STATE_WIFI_SUCCESS);
    loop(IOT_RUN_STATE_READ_IP);
    // Serial.println("wifi ok");
}


void DFRobot_WiFi_IoT_Module_I2C::MQTTBegin(const String &server, const String &port, const String &productID, const String &pwd, char* deviceID)
{

    if(deviceID == NULL){
        manageFunction(IOT_SET_COMMAND, SET_MQTT_SERVER, server);
        manageFunction(IOT_SET_COMMAND, SET_MQTT_PORT, port);
        manageFunction(IOT_SET_COMMAND, SET_MQTT_ID, productID);
        manageFunction(IOT_SET_COMMAND, SET_MQTT_PASSWORLD, pwd);
        connection(CONNECT_MQTT);
        loop(IOT_RUN_STATE_MQTT_SUCCESS);
    }else{
        manageFunction(IOT_SET_COMMAND, SET_MQTT_SERVER, server);
        manageFunction(IOT_SET_COMMAND, SET_MQTT_PORT, port);
        manageFunction(IOT_SET_COMMAND, SET_MQTT_ID, productID);
        manageFunction(IOT_SET_COMMAND, SET_MQTT_PASSWORLD, pwd);
        manageFunction(IOT_SET_COMMAND, SET_MQTT_DEVICEID, deviceID);
        connection(CONNECT_MQTT_ONENET);
        loop(IOT_RUN_STATE_MQTT_SUCCESS);
    }
        // Serial.println("mqtt ok");
}

void DFRobot_WiFi_IoT_Module_I2C::subTopic()
{
    for (int i = 0; i < mqttTopicArray.size(); i++){
        manageFunction(IOT_RUN_COMMAND, SUBSCRIBE_TOPIC0 + i, mqttTopicArray[i]);//Subscribe to topic
        loop(IOT_RUN_STATE_MQTT_SUBSCIBE_SUCCESS);
        obloqStatus = IOT_RUN_STATE_NONE;
    }
}

void DFRobot_WiFi_IoT_Module_I2C::publish(Topicnum topic_num, const String &message)
{
    if (topic_num >= mqttTopicArray.size())
    {
        if(debugHandle || OBLOQ_DEBUG){
            Serial.println("The number of topics exceeds the maximum");
        }
        return;
    }
    if (message.length() > IOT_MAX_MESSAGE) {
        if(debugHandle || OBLOQ_DEBUG){
            Serial.print("The topic message sent is too long, Supports a maximum of [");
            Serial.print(IOT_MAX_MESSAGE);
            Serial.println("] bytes");
        } 
        return;
    }
    manageFunction(IOT_RUN_COMMAND, SEND_TOPIC0 + topic_num, message);
    loop(IOT_RUN_STATE_MQTT_SEND_SUCCESS);
    loop(IOT_RUN_STATE_MQTT_RECEIVE_SUCCESS);
    obloqStatus = IOT_RUN_STATE_NONE;
}

void DFRobot_WiFi_IoT_Module_I2C::publish(Topicnum topic_num, int32_t i) {
    if (topic_num >= mqttTopicArray.size())
        return;
    char buffer[34];
    memset(buffer, 0, 34);
    itoa(i, buffer, 10);
    publish(topic_num, (const String &)buffer);
}

void DFRobot_WiFi_IoT_Module_I2C::publish(Topicnum topic_num, double f) {
    if (topic_num >= mqttTopicArray.size())
        return;
    String str = String(f, 5);
    int len = str.length() + 1;
    char buffer[len];
    str.toCharArray(buffer, len);
    buffer[len - 1] = '\0';
    char *p = buffer + len - 1;
    while (*p == '\0' || *p == '0')
    {
        *p = '\0';
        p--;
    }
    if (*p == '.')
        *p = '\0';
    if (str == "-0")
        str = "0";
    str = String(buffer);
    publish(topic_num, (const String &)str);
}

void DFRobot_WiFi_IoT_Module_I2C::HTTPBegin(const String &ip, const String &port)
{
    manageFunction(IOT_SET_COMMAND, SET_HTTP_ID, ip);
    manageFunction(IOT_SET_COMMAND, SET_HTTP_PORT, port);
}

char *DFRobot_WiFi_IoT_Module_I2C::get(const String &url, int timeOut)
{
    delay(200);     //
    manageFunction(IOT_RUN_COMMAND, HTTP_GET_URL, url);
    loop(IOT_RUN_STATE_HTTP_NORMAL_RETURN, timeOut);
    return (char *)iotRecvData;
}

char *DFRobot_WiFi_IoT_Module_I2C::post(const String &url, const String &data, int timeOut)
{
    String urlData = "";
    urlData += url;
    urlData += ',';
    urlData += data;
    delay(200);
    manageFunction(IOT_RUN_COMMAND, HTTP_POST_URL_CON, urlData);
    loop(IOT_RUN_STATE_HTTP_NORMAL_RETURN, timeOut);
    return (char *)iotRecvData;
}

char *DFRobot_WiFi_IoT_Module_I2C::put(const String &url, const String &data, int timeOut)
{
    String urlData = "";
    urlData += url;
    urlData += ',';
    urlData += data;
    delay(200);
    manageFunction(IOT_RUN_COMMAND, HTTP_PUT_URL_CON, urlData);
    loop(IOT_RUN_STATE_HTTP_NORMAL_RETURN, timeOut);
    return (char *)iotRecvData;
}

char *DFRobot_WiFi_IoT_Module_I2C::getVersion()
{ 
    connection(QUERY_VERSION);
    loop(IOT_RUN_STATE_VERSION);
    return (char*)iotRecvData;
}

void DFRobot_WiFi_IoT_Module_I2C::registerMqttHandle(const MqttHandle handles[])
{
    if (handles == NULL)
        return;
    for (int i = 0; i < SUBSCRIBE_UPPER_LIMIT; ++i)
    {
        if (handles[i])
        {
            mqttHandle[i] = handles[i];
        }
    }
}


void DFRobot_WiFi_IoT_Module_I2C::registerDebugHandle(const DebugHandle handle)
{
    if (!handle) {
        return;
    }
    debugHandle = handle;

}

void DFRobot_WiFi_IoT_Module_I2C::iotDebug(int8_t code, uint8_t* data)
{
    if (code == IOT_RUN_STATE_MESSAGE_RECEIVE_FAILURE) {
        if(debugHandle || OBLOQ_DEBUG){
            Serial.print("receive message is too long, Supports a maximum of [");
            Serial.print(IOT_MAX_MESSAGE);
            Serial.println("] bytes");
        }              
        return;
    }

    if(debugHandle){
        switch (code)
        {
#if IOTDEBUG
            case IOT_RUN_STATE_NONE:
                break;
            case IOT_RUN_STATE_PING_SUCCESS:
                debugHandle("ping success", code); break;
            case IOT_RUN_STATE_PING_FAIL:
                debugHandle("ping failure", code); break;
            case IOT_RUN_STATE_WIFI_CONNECTING:
                break;
            case IOT_RUN_STATE_WIFI_SUCCESS:
                debugHandle("Wifi connection success", code); break;   
            case IOT_RUN_STATE_WIFI_CONNECT_ERR:
                debugHandle("Wifi connection failure", code); break;
            case IOT_RUN_STATE_READ_IP:
                debugHandle((char *)IPaddress, code); break;
            case IOT_RUN_STATE_MQTT_CONNECTING:
                break;
            case IOT_RUN_STATE_MQTT_SUCCESS:
                debugHandle("mqtt connection success", code); break;
            case IOT_RUN_STATE_MQTT_ERR:
                debugHandle("mqtt connection failure", code); break;
            case IOT_RUN_STATE_MQTT_SUBSCIBE_SUCCESS:
                debugHandle("mqtt subscibe success", code); break;
            case IOT_RUN_STATE_MQTT_SUBSCIBE_ERR:
                debugHandle("mqtt subscibe failure", code); break;
            case IOT_RUN_STATE_MQTT_SUBSCIBE_LIMIT:
                debugHandle("mqtt subscibe upper limit", code); break;
            case IOT_RUN_STATE_MQTT_SEND_SUCCESS:
                debugHandle("mqtt send success", code); break;
            case IOT_RUN_STATE_MQTT_SEND_ERR:
                debugHandle("mqtt send failure", code); break;
            case IOT_RUN_STATE_MQTT_RECEIVE_SUCCESS:
                break;
            case IOT_RUN_STATE_HTTP_NORMAL_RETURN:
                debugHandle("http normal return", code); break;
            case IOT_RUN_STATE_HTTP_ERROR_RETURN:
                debugHandle("http return error", code); break;
            case IOT_RUN_STATE_VERSION:
                debugHandle("get version", code); break;
            case IOT_RUN_STATE_IIC_READ_ERR:
                debugHandle("iic read iot data error", code); break;
            case IOT_RUN_STATE_MEM_ALLOCATE_FAIL:
                debugHandle("memory allocation failed", code); break;
            case IOT_RUN_STATE_MQTT_RECEIVE_MESSAGE_LEN:
                debugHandle(("receive topic"+String(data[0]-SUBSCRIBE_TOPIC0)+" len ["+String(data[1])+"]").c_str(), code); break;
            // case IOT_RUN_STATE_MESSAGE_RECEIVE_FAILURE:
            //     break;
#endif                           
#if ENABLE_DBGIOTSTATUS
            case IOT_RUN_STATE_FALG:
                Serial.print("status ["); Serial.print(data[0], HEX); Serial.print(" "); Serial.print(data[1], HEX); Serial.print(" "); Serial.print(obloqStatus); Serial.println("]"); break;
#endif            
            default:
                break;
        }
    }
}


void DFRobot_WiFi_IoT_Module_I2C::clearBuffer(void)
{
  uint8_t buf[10];
  buf[0] = 0x02;
  buf[1] = 0x17;
  writeReg(0x1E, buf, (uint8_t)2);
  //Serial.println("clear buffer!");
  delay(1000);
}


void DFRobot_WiFi_IoT_Module_I2C::manageFunction(uint8_t command, uint8_t config, String data)
{
  uint8_t i = 0, j = 0;
  i = data.length();
  uint8_t datalen = i+3;
  uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t)*datalen);
  if(buffer == NULL){
    if(debugHandle || OBLOQ_DEBUG){
        Serial.println("memory allocation failed");
    }
    return;
  }
  buffer[0]  =  command;
  buffer[1]  =  config;
  buffer[2]  =  datalen-3;
  for(i = 3,j = 0; i < datalen; i++){
    buffer[i] = (char)data[j++];
  } 
  writeReg(IOT_COMMAND_REGTSTER, buffer, datalen);
  free(buffer);
}

void DFRobot_WiFi_IoT_Module_I2C::manageFunction(uint8_t command, uint8_t config, uint8_t *data, uint16_t len)
{
  uint8_t i = 0, j = 0;
  uint8_t datalen = len+3;
  uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t)*datalen);
  if(buffer == NULL){
    if(debugHandle || OBLOQ_DEBUG){
        Serial.println("memory allocation failed");
    }
    return;
  }
  buffer[0]  =  command;
  buffer[1]  =  config;
  buffer[2]  =  len;
  for(i = 3,j = 0; i < datalen; i++,j++){
    buffer[i] = data[j];
  } 

  writeReg(IOT_COMMAND_REGTSTER, buffer, datalen);
  free(buffer);
}

void DFRobot_WiFi_IoT_Module_I2C::receiveObloqData()
{
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_AVR_MEGA2560)    
    sleep(50);
#endif
    if (readReg(IOT_COMMAND_REGTSTER, &buffer, 2) != 2) {
        iotDebug(IOT_RUN_STATE_IIC_READ_ERR); 
        obloqStatus = IOT_RUN_STATE_IIC_READ_ERR;
        return;
    }

    uint8_t command = buffer[0];
    uint8_t status = buffer[1];
    switch (command)
    {
        case PING_STATUE:
        {
            if (status == 0x01) {
                obloqStatus = IOT_RUN_STATE_PING_SUCCESS;
            } else if (status == 0x00){
                obloqStatus = IOT_RUN_STATE_PING_FAIL;
            } else{
                obloqStatus = IOT_RUN_STATE_NONE;
            }
            break;
        }
        case WIFI_STATUE_CON:
        {
            if(status == 0x03){
                iotDebug(IOT_RUN_STATE_WIFI_SUCCESS);        
                obloqStatus = IOT_RUN_STATE_WIFI_SUCCESS;
            }else if(status == 0x00){                
                wifiState++; 
                if(wifiState == 2){
                    wifiState = 0;
                    connection(CONNECT_WIFI);
                }
                iotDebug(IOT_RUN_STATE_WIFI_CONNECT_ERR);         
                obloqStatus = IOT_RUN_STATE_WIFI_CONNECT_ERR;
            }else if(status == 0x02){       
                obloqStatus = IOT_RUN_STATE_WIFI_CONNECTING;
            }else{
                obloqStatus = IOT_RUN_STATE_NONE;
            }
            break;
        }  
        case IP_ADDRESS:
        {
            uint8_t len = status;
            if(len > 0x7F){
                obloqStatus = IOT_RUN_STATE_NONE;
                return ;
            }
            if(getData(READ_DATA_REGISTER, IPaddress,len) != len){
                iotDebug(IOT_RUN_STATE_IIC_READ_ERR);
                obloqStatus = IOT_RUN_STATE_IIC_READ_ERR;
                return;
            }else{          
                iotDebug(IOT_RUN_STATE_READ_IP);
            } 
            if (mqttState == 1) {
                mqttState = 0;
                connection(DISCONNECT_MQTT);
                delay(200);
                connection(CONNECT_MQTT);
            }
            obloqStatus = IOT_RUN_STATE_READ_IP;
            break;
        }
        case MQTT_STATUE_CON:
        {
            if(status == 0x01){
                iotDebug(IOT_RUN_STATE_MQTT_SUCCESS);   
                mqttState = 1;
                obloqStatus = IOT_RUN_STATE_MQTT_SUCCESS;
            }else if(status == 0x02){
                iotDebug(IOT_RUN_STATE_MQTT_ERR);
                obloqStatus = IOT_RUN_STATE_MQTT_ERR;
            }else if(status == 0x00){
                connection(DISCONNECT_MQTT);
                connection(CONNECT_WIFI);
                obloqStatus = IOT_RUN_STATE_MQTT_CONNECTING;
            }else{
                obloqStatus = IOT_RUN_STATE_NONE;
            }
            break;
        }  
        case SUBSCRIBE_STATUE:
        {
            if(status == 0x01){           
                iotDebug(IOT_RUN_STATE_MQTT_SUBSCIBE_SUCCESS);
                obloqStatus = IOT_RUN_STATE_MQTT_SUBSCIBE_SUCCESS;
            }else if(status == 0x03){           
                iotDebug(IOT_RUN_STATE_MQTT_SUBSCIBE_ERR);               
                obloqStatus = IOT_RUN_STATE_MQTT_SUBSCIBE_ERR;
            }else if(status == 0x02){                          
                iotDebug(IOT_RUN_STATE_MQTT_SUBSCIBE_LIMIT);
                obloqStatus = IOT_RUN_STATE_MQTT_SUBSCIBE_LIMIT;
            }else{
                obloqStatus = IOT_RUN_STATE_NONE;
            }
            break;
        }
        case SUBSCRIBE_TOPIC0:
        case SUBSCRIBE_TOPIC1:
        case SUBSCRIBE_TOPIC2:
        case SUBSCRIBE_TOPIC3:
        case SUBSCRIBE_TOPIC4:
        {
            uint8_t datalen = status;
            if(datalen == 0xFF){
                return ;            //返回错误数据
            }
            iotDebug(IOT_RUN_STATE_MQTT_RECEIVE_MESSAGE_LEN, buffer);
            if(datalen > 0x80 || datalen > IOT_MAX_MESSAGE){
                iotDebug(IOT_RUN_STATE_MESSAGE_RECEIVE_FAILURE);
                obloqStatus = IOT_RUN_STATE_MESSAGE_RECEIVE_FAILURE;
                return ;
            }
#if ENABLE_ALLOCATE_MEM
            iotRecvData = (uint8_t *)malloc(sizeof(uint8_t)*(datalen+1));
            if(iotRecvData == NULL){
                iotDebug(IOT_RUN_STATE_MEM_ALLOCATE_FAIL);
                obloqStatus = IOT_RUN_STATE_MEM_ALLOCATE_FAIL;
                return ;
            } 
            memset(iotRecvData, 0, datalen+1);
            if(getData(READ_DATA_REGISTER, iotRecvData, datalen) != datalen){            
                    iotDebug(IOT_RUN_STATE_IIC_READ_ERR);
                    obloqStatus = IOT_RUN_STATE_IIC_READ_ERR;
                    free(iotRecvData);
                    return;
                }else{
                    // Serial.print("topic");
                    // Serial.print(command-SUBSCRIBE_TOPIC0);
                    // Serial.print(":");
                    // Serial.println((char *)iotRecvData);
                    if(mqttHandle[command-SUBSCRIBE_TOPIC0]){
                        mqttHandle[command-SUBSCRIBE_TOPIC0]((char *)iotRecvData);
                    }
                    obloqStatus = IOT_RUN_STATE_MQTT_RECEIVE_SUCCESS;
                    free(iotRecvData);
                }
            break;
#else
            memset(iotRecvData, 0, datalen+1);
            if(getData(READ_DATA_REGISTER, iotRecvData, datalen) != datalen){            
                    iotDebug(IOT_RUN_STATE_IIC_READ_ERR);
                    obloqStatus = IOT_RUN_STATE_IIC_READ_ERR;
                    return;
                }else{
                    // Serial.print("topic");
                    // Serial.print(command-SUBSCRIBE_TOPIC0);
                    // Serial.print(":");
                    // Serial.println((char *)iotRecvData);
                    if(mqttHandle[command-SUBSCRIBE_TOPIC0]){
                        mqttHandle[command-SUBSCRIBE_TOPIC0]((char *)iotRecvData);
                    }
                    obloqStatus = IOT_RUN_STATE_MQTT_RECEIVE_SUCCESS;
                }
            break;
#endif  
        }
        case HTTP_NORMAL_RETURN:
        {
            uint8_t datalen = 0;
            if(status < 0xFE) // todo: http 可能收到0x10 0x00
            {
                datalen = status;
                if(datalen > IOT_MAX_MESSAGE){
                    iotDebug(IOT_RUN_STATE_MESSAGE_RECEIVE_FAILURE);
                    obloqStatus = IOT_RUN_STATE_MESSAGE_RECEIVE_FAILURE;
                    return ;
                }
#if ENABLE_ALLOCATE_MEM                
                iotRecvData = (uint8_t *)malloc(sizeof(uint8_t)*datalen+1);
                if(iotRecvData == NULL){
                    iotDebug(IOT_RUN_STATE_MEM_ALLOCATE_FAIL);
                    obloqStatus = IOT_RUN_STATE_MEM_ALLOCATE_FAIL;
                    return;
                }
                memset(iotRecvData, 0, datalen+1);
                if(getData(READ_DATA_REGISTER, iotRecvData, datalen) != datalen){                 
                    iotDebug(IOT_RUN_STATE_IIC_READ_ERR);
                    obloqStatus = IOT_RUN_STATE_IIC_READ_ERR;
                    free(iotRecvData);
                }else{
                    obloqStatus = IOT_RUN_STATE_HTTP_NORMAL_RETURN;
                }
#else 
                memset(iotRecvData, 0, datalen+1);
                if(getData(READ_DATA_REGISTER, iotRecvData, datalen) != datalen){                 
                    iotDebug(IOT_RUN_STATE_IIC_READ_ERR);
                    obloqStatus = IOT_RUN_STATE_IIC_READ_ERR;
                }else{
                    iotDebug(IOT_RUN_STATE_HTTP_NORMAL_RETURN);
                    obloqStatus = IOT_RUN_STATE_HTTP_NORMAL_RETURN;
                }
#endif  
            } else {
                obloqStatus = IOT_RUN_STATE_NONE;
            }
            break;
        }
        case HTTP_ERROR_RETURN:
        {           
            iotDebug(IOT_RUN_STATE_HTTP_ERROR_RETURN);
            obloqStatus = IOT_RUN_STATE_HTTP_ERROR_RETURN;
            break;
        }
        case GET_VERSION:
        {
            uint8_t datalen = 0;
            if(status < 0xFE)
            {
                datalen = status;
                if(datalen > IOT_MAX_MESSAGE){
                    obloqStatus = IOT_RUN_STATE_NONE;
                    iotDebug(IOT_RUN_STATE_MESSAGE_RECEIVE_FAILURE);
                    obloqStatus = IOT_RUN_STATE_MESSAGE_RECEIVE_FAILURE;
                    return ;
                }
#if ENABLE_ALLOCATE_MEM
                iotRecvData = (uint8_t *)malloc(sizeof(uint8_t)*datalen+1);
                if(iotRecvData == NULL){
                    iotDebug(IOT_RUN_STATE_MEM_ALLOCATE_FAIL);
                    obloqStatus = IOT_RUN_STATE_MEM_ALLOCATE_FAIL;
                    return;
                }
                memset(iotRecvData, 0, datalen+1);
                if(getData(READ_DATA_REGISTER, iotRecvData, datalen) != datalen){                 
                    iotDebug(IOT_RUN_STATE_IIC_READ_ERR);
                    obloqStatus = IOT_RUN_STATE_IIC_READ_ERR;
                    free(iotRecvData);
                }else{
                    iotDebug(IOT_RUN_STATE_VERSION);
                    obloqStatus = IOT_RUN_STATE_VERSION;
                }
#else                
                memset(iotRecvData, 0, datalen+1);
                if(getData(READ_DATA_REGISTER, iotRecvData, datalen) != datalen){                 
                    iotDebug(IOT_RUN_STATE_IIC_READ_ERR);
                    obloqStatus = IOT_RUN_STATE_IIC_READ_ERR;
                }else{
                    iotDebug(IOT_RUN_STATE_VERSION);
                    obloqStatus = IOT_RUN_STATE_VERSION;
                }
#endif
            } else {
                obloqStatus = IOT_RUN_STATE_NONE;
            }
            break;
        }
        case GET_MQTTSEND_STATUE:
        {
            if(status == 0x01){
                iotDebug(IOT_RUN_STATE_MQTT_SEND_SUCCESS);
                obloqStatus = IOT_RUN_STATE_MQTT_SEND_SUCCESS;
            }else if(status == 0x00){
                iotDebug(IOT_RUN_STATE_MQTT_SEND_ERR);
                obloqStatus = IOT_RUN_STATE_MQTT_SEND_ERR;
            }else{
                obloqStatus = IOT_RUN_STATE_NONE;
            }
            break;
        }
        default:
        {
            obloqStatus = IOT_RUN_STATE_NONE;
            break;
        }    
    }
    iotDebug(IOT_RUN_STATE_FALG, buffer);            
}

void DFRobot_WiFi_IoT_Module_I2C::connection(uint8_t command)
{
    uint8_t buffer[2];
    buffer[0] = 0x02;
    buffer[1] = command;
    writeReg(IOT_COMMAND_REGTSTER, &buffer, 2);	
}


void DFRobot_WiFi_IoT_Module_I2C::loop(uint8_t status, int timeOut)
{
    uint32_t startingTime = millis();
    while (true)
    {
        if(status == obloqStatus)
        {
            return;
        }
#if defined(ARDUINO_ARCH_RP2040)
        delay(5);
#else
        delay(50);
#endif
        if((millis() - startingTime) > timeOut)
        {
            if(debugHandle || OBLOQ_DEBUG){            
                Serial.print("time out status:");
                Serial.println(status);
            }
#if ENABLE_ALLOCATE_MEM
#else 
            memset(iotRecvData, 0, IOT_MAX_MESSAGE+3);
#endif
            return;
        }
    }
}





void DFRobot_WiFi_IoT_Module_I2C::writeReg(uint8_t reg, void *pBuf, size_t size)
{
    uint8_t *_pBuf = (uint8_t*)pBuf;
    _pWire->beginTransmission(_address);
    _pWire->write(reg);
    if(size > 32){
        uint16_t j = 1;
        for(uint8_t i = 0; i<size; i++){
            if(i >= (31*j)){
                _pWire->endTransmission(false);
                _pWire->beginTransmission(_address); 
                j++;
            }
            _pWire->write(_pBuf[i]);
        }
    }else{
        for(size_t i = 0; i < size; i++){
            _pWire->write(_pBuf[i]);
        }
    }
    _pWire->endTransmission();
    delay(20);
}

uint8_t DFRobot_WiFi_IoT_Module_I2C::readReg(uint8_t reg, void *pBuf, size_t size){
    if(pBuf == NULL){
        return 0;
    }
    uint8_t *_pBuf = (uint8_t*)pBuf;
    _pWire->beginTransmission(_address);
    _pWire->write(reg);
    _pWire->write(IOT_READ_COMMAND);
    _pWire->write(READ_COMMAND);
    if(_pWire->endTransmission() != 0){
        return 0;
    }
    delay(100);
    _pWire->requestFrom(_address,size);
    for(size_t i = 0; i < size; i++){
        _pBuf[i] = _pWire->read();
    }
    return size;
}

uint8_t DFRobot_WiFi_IoT_Module_I2C::getData(uint8_t reg, void *pBuf, size_t size)
{
    size_t i = 0;
    size_t temp = 0;
    if(pBuf == NULL){
        return 0;
    }
    if(size == 0){
        return 0;
    }
    uint8_t *_pBuf = (uint8_t*)pBuf;
    if(size < IICREADDATAMAX){
        _pWire->beginTransmission(_address);
        _pWire->write(reg);
        if(_pWire->endTransmission() != 0){
            return 0;
        }
        _pWire->requestFrom(_address,size);
        for(i = i; i < size; i++){
            _pBuf[i] = _pWire->read();
        }
    }else{
        for(uint8_t j = 0; j < size / IICREADDATAMAX; j++){
            _pWire->beginTransmission(_address);
            _pWire->write(reg);
            if(_pWire->endTransmission() != 0){
                return 0;
            }
            _pWire->requestFrom(_address,size);
            for(i = i; i < IICREADDATAMAX; i++){
                _pBuf[i] = _pWire->read();
            }
        }
        temp = i;
        _pWire->beginTransmission(_address);
        _pWire->write(reg);
        if(_pWire->endTransmission() != 0){
            return 0;
        }
        _pWire->requestFrom(_address,size%IICREADDATAMAX);
        for(i = 0; i < size%IICREADDATAMAX; i++){  
            _pBuf[temp+i] = _pWire->read();
        }
    }
  
  return size;
}



DFRobot_WiFi_IoT_Module_UART::DFRobot_WiFi_IoT_Module_UART()
{
    activeObject = this;
    debugHandle = NULL;
    obloqStatus = OBLOQ_NONE; 
    singleHardSerialFlag = 0;
    softSerialFlag = 0;
    memset(iotRecvData, 0, IOT_MAX_MESSAGE+IOT_TOPIC_MAX_LEN+10);
    printSerial = &Serial;
}

#if ENABLE_SOFTSERIALPRINT
DFRobot_WiFi_IoT_Module_UART::DFRobot_WiFi_IoT_Module_UART(Stream *softserial)
{
    activeObject = this;
    debugHandle = NULL;
    obloqStatus = OBLOQ_NONE; 
    singleHardSerialFlag = 0;
    softSerialFlag = 0;
    memset(iotRecvData, 0, IOT_MAX_MESSAGE+IOT_TOPIC_MAX_LEN+10);
    printSerial = softserial;
}
#endif

DFRobot_WiFi_IoT_Module_UART::~DFRobot_WiFi_IoT_Module_UART()
{
    
}


void DFRobot_WiFi_IoT_Module_UART::startConnect(uint8_t receive, uint8_t send, const String& ssid, const String& pwd, const String& iotId, const String& iotPwd, const String iotTopics[], const String& server, uint32_t port)
{
    init(receive, send);
    begin();
    // WiFi连接
    connectWifi(ssid, pwd);
    // Mqtt连接
    MQTTBegin(server, String(port), iotId, iotPwd);
    
    // 订阅Topic
    saveTopic(iotTopics);
    subTopic();
}

void DFRobot_WiFi_IoT_Module_UART::startConnect(Stream *serial, const String& ssid, const String& pwd, const String& iotId, const String& iotPwd, const String iotTopics[], const String& server, uint32_t port)
{

    _s = serial;
    softSerialFlag = 1;
    init();
    begin();
    // WiFi连接
    connectWifi(ssid, pwd);
    // // Mqtt连接
    MQTTBegin(server, String(port), iotId, iotPwd);
    // 订阅Topic
    saveTopic(iotTopics);
    subTopic();

}

void DFRobot_WiFi_IoT_Module_UART::startConnect(uint8_t receive, uint8_t send, const String& ssid, const String& pwd, const String& ip, uint32_t port)
{
    init(receive, send);
    begin();
    // WiFi连接
    connectWifi(ssid, pwd);
    HTTPBegin(ip, String(port));
}

void DFRobot_WiFi_IoT_Module_UART::startConnect(Stream *serial, const String& ssid, const String& pwd, const String& ip, uint32_t port)
{
    _s = serial;
    softSerialFlag = 1;
    init();
    begin();
    // WiFi连接
    connectWifi(ssid, pwd);
    HTTPBegin(ip, String(port));
}

void DFRobot_WiFi_IoT_Module_UART::subTopic()
{
    String sendMsg = "";
    for (int i = 0; i < mqttTopicArray.size(); i++)
    {
        sendMsg = "|4|1|2|" + mqttTopicArray[i] + "|\r";
        serialSend(sendMsg);       
        loop(OBLOQ_SUB_OK);
        obloqStatus = OBLOQ_NONE;
    }
}

void DFRobot_WiFi_IoT_Module_UART::publish(Topicnum topic_num, const String &message)
{ 
    String sendMsg = "";
    if (topic_num >= mqttTopicArray.size())
    {
        if(debugHandle || OBLOQ_DEBUG){
            printSerial->println("The number of topics exceeds the maximum");
        }
        return;
    }
    if (message.length() > IOT_MAX_MESSAGE) {
        if (!singleHardSerialFlag || ENABLE_SOFTSERIALPRINT){
            if(debugHandle || OBLOQ_DEBUG){
                // printSerial->println("The topic message send is too long");
                printSerial->print("The topic message sent is too long, Supports a maximum of [");
                printSerial->print(IOT_MAX_MESSAGE);
                printSerial->println("] bytes");
            }
        }
        return;
    }
    sendMsg = "|4|1|3|" + mqttTopicArray[topic_num] + "|" + message + "|\r";
    serialSend(sendMsg);
    loop(OBLOQ_PULISH_OK);
    loop(OBLOQ_MQTT_RECEIVE_OK);
    obloqStatus = OBLOQ_NONE;
}

void DFRobot_WiFi_IoT_Module_UART::publish(Topicnum topic_num, int32_t i) {
    if (topic_num >= mqttTopicArray.size())
        return;
    char buffer[34];
    memset(buffer, 0, 34);
    itoa(i, buffer, 10);
    publish(topic_num, (const String &)buffer);
}

void DFRobot_WiFi_IoT_Module_UART::publish(Topicnum topic_num, double f) {
    if (topic_num >= mqttTopicArray.size())
        return;
    String str = String(f, 5);
    int len = str.length() + 1;
    char buffer[len];
    str.toCharArray(buffer, len);
    buffer[len - 1] = '\0';
    char *p = buffer + len - 1;
    while (*p == '\0' || *p == '0')
    {
        *p = '\0';
        p--;
    }
    if (*p == '.')
        *p = '\0';
    if (str == "-0")
        str = "0";
    str = String(buffer);
    publish(topic_num, (const String &)str);
}

void DFRobot_WiFi_IoT_Module_UART::registerMqttHandle(const MqttHandle handles[])
{
    if (handles == NULL)
        return;
    for (int i = 0; i < SUBSCRIBE_UPPER_LIMIT; ++i)
    {
        if (handles[i])
        {
            mqttHandle[i] = handles[i];
        }
    }
}

void DFRobot_WiFi_IoT_Module_UART::registerDebugHandle(const DebugHandle handle)
{
    if (!handle) {
        return;
    }
    debugHandle = handle;
}

char *DFRobot_WiFi_IoT_Module_UART::get(const String &url, int timeOut)
{
    String sendMsg = "";
    char *p = (char *)iotRecvData;
    sendMsg = "|3|1|http://" + httpIp + "/" + url + "|\r";
    serialSend(sendMsg);

    loop(OBLOQ_HTTP_RECEIVE_OK, timeOut);
    obloqStatus = OBLOQ_NONE;
    return (char *)(p+7);
}

char *DFRobot_WiFi_IoT_Module_UART::post(const String &url, const String &data, int timeOut)
{
    String sendMsg = "";
    char *p = (char *)iotRecvData;
    sendMsg = "|3|2|http://" + httpIp + "/" + url + "," + data + "|\r";
    serialSend(sendMsg);
    loop(OBLOQ_HTTP_RECEIVE_OK, timeOut);
    obloqStatus = OBLOQ_NONE;
    return (char *)(p+7);
}

char *DFRobot_WiFi_IoT_Module_UART::put(const String &url, const String &data, int timeOut)
{
    String sendMsg = "";
    char *p = (char *)iotRecvData;
    sendMsg = "|3|3|http://" + httpIp + "/" + url + "," + data + "|\r";
    serialSend(sendMsg);
    loop(OBLOQ_HTTP_RECEIVE_OK, timeOut);
    obloqStatus = OBLOQ_NONE;
    return (char *)(p+7);
}

char *DFRobot_WiFi_IoT_Module_UART::getVersion(){
    char *p = (char *)iotRecvData;
    serialSend("|1|2|\r");
    loop(OBLOQ_GET_VERSION_OK);
    return (char*)(p+5);
}

void DFRobot_WiFi_IoT_Module_UART::begin(void)
{
    serialSend("|1|1|\r");
    loop(OBLOQ_PING_OK);
}
void DFRobot_WiFi_IoT_Module_UART::connectWifi(const String &ssid, const String &pwd)
{
    String sendMsg = "|2|1|" + ssid + "," + pwd + "|\r";
    serialSend(sendMsg);
    loop(OBLOQ_WIFI_CONNECT_OK);
  
}
void DFRobot_WiFi_IoT_Module_UART::MQTTBegin(const String &server, const String &port, const String &productID, const String &pwd, char* deviceID)
{
    String sendMsg = "";
    if(deviceID==NULL){
        sendMsg = "|4|1|1|" + server + "|" + port + "|" + productID + "|" + pwd + "|\r";
    }else{
        sendMsg = "|4|1|1|" + server + "|" + port + "|" + productID + "|" + pwd + "|" + deviceID + "|\r";
    }
    serialSend(sendMsg);
    while(loop(OBLOQ_MQTT_CONNECT_OK));
}
void DFRobot_WiFi_IoT_Module_UART::HTTPBegin(const String &ip, const String &port)
{
    if (isIP(ip.c_str()))
        httpIp = ip + ":" + port;
    else
        httpIp = ip;
}
void DFRobot_WiFi_IoT_Module_UART::iotDebug(int8_t code, uint8_t* data)
{
    if(debugHandle){
        switch (code)
        {
#if IOTDEBUG
            case OBLOQ_NONE:
                break;
            case OBLOQ_MQTT_CONNECT_OK:
                debugHandle("mqtt Connection Successful", code); break;
            case OBLOQ_SUB_OK:
                debugHandle("Subscribe to the success", code); break;
            case OBLOQ_PING_OK:
                debugHandle("ping ok", code); break;
            case OBLOQ_PULISH_OK:
                debugHandle("mqtt send success", code); break;
            case OBLOQ_MQTT_DISCONNECT_OK:
                debugHandle("mqtt disconnect", code); break;
            case OBLOQ_GET_VERSION_OK:
                debugHandle("get version", code); break;
            case OBLOQ_WIFI_CONNECT_OK:
                debugHandle("wifi Connection Successful", code); break;
            case OBLOQ_HTTP_RECEIVE_OK:
                debugHandle("http normal receive", code); break;
            case OBLOQ_MQTT_RECEIVE_OK:
                break;
            case OBLOQ_SERIAL_CONNECT_FAILURE:
                break;
            case OBLOQ_MQTT_CONNECT_FAILURE:
                debugHandle("mqtt connect failure", code); break;
            case OBLOQ_WIFI_CONNECT_FAILURE:
                debugHandle("Wifi connection failure", code); break;
            case OBLOQ_SUB_FAILURE:
                debugHandle("Topic subscription failed", code); break;
            case OBLOQ_PULISH_FAILURE:
                debugHandle("Pulish failure", code); break;
#endif
#if ENABLE_DBGIOTSTATUS
            case OBLOQ_MESSAGE_FLAG:
                if (!singleHardSerialFlag || ENABLE_SOFTSERIALPRINT)
                {printSerial->print("receivedata:"); printSerial->print((char *)iotRecvData); printSerial->print(" STA:"); printSerial->println(obloqStatus);}
                break;
#endif 
            default:
                break;          
        }
    }
}

void DFRobot_WiFi_IoT_Module_UART::saveTopic(const String iotTopics[])
{
    for (uint8_t i = 0; i < SUBSCRIBE_UPPER_LIMIT; ++i)
    {
        if (iotTopics[i].length() > IOT_TOPIC_MAX_LEN) {
            if(debugHandle || OBLOQ_DEBUG){
                if (!singleHardSerialFlag || ENABLE_SOFTSERIALPRINT) {
                    printSerial->print("topic");
                    printSerial->print(i);
                    printSerial->println(" too long");
                }
            }
            continue;
        }
        if (iotTopics[i] != ""){
            mqttTopicArray.push_back(iotTopics[i]);
        }
    }
}

uint8_t DFRobot_WiFi_IoT_Module_UART::isIP(const char str[])
{
    int a, b, c, d;
    char temp[20];
    if ((sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d)) != 4)
        return 0;
    sprintf(temp, "%d.%d.%d.%d", a, b, c, d);
    if (strcmp(temp, str) != 0)
        return 0;
    if (!((a <= 255 && a >= 0) && (b <= 255 && b >= 0) && (c <= 255 && c >= 0)))
        return 0;
    else
        return 1;
}
void DFRobot_WiFi_IoT_Module_UART::serialSend(const String &msg)
{
#if defined(NRF5) || defined(NRF52833)
    if (softSerialFlag){
        _s->print(msg);
    } else {
        uBit.serial.send(msg.c_str());
    }
#else
    _s->print(msg);
#endif
}


void DFRobot_WiFi_IoT_Module_UART::init(int8_t receive, int8_t send)
{
    if(softSerialFlag) {
/***********************************任务初始化************************************/
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_AVR_MEGA2560)
        mySCoop.start();
        mySCoop.yield();
        delay(10);
#elif defined(NRF5) || defined(NRF52833)
        create_fiber(myTask1);
#elif defined(ARDUINO_ARCH_RP2040)
        xTaskCreate(myTask1, "myTask1", 256, NULL, 1, nullptr);
#endif
/*********************************************************************************/
    } else {
/***********************************串口初始化************************************/        
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)   
        Serial.begin(9600);
        _s = &Serial;
        singleHardSerialFlag = 1;
#elif defined(ARDUINO_AVR_LEONARDO)
        Serial1.begin(9600);
        _s = &Serial1;
#elif defined(ARDUINO_AVR_MEGA2560)
        if(receive == 19 && send == 18){
            Serial1.begin(9600);
            _s = &Serial1;
        }else if(receive == 17 && send == 16){
            Serial2.begin(9600);
            _s = &Serial2;
        }else if(receive == 15 && send == 14){
            Serial3.begin(9600);
            _s = &Serial3;
        }else{
            Serial1.begin(9600);
            _s = &Serial1;
        }
#elif defined(NRF5) || defined(NRF52833)
        serialInit(receive, send);
        singleHardSerialFlag = 1;
#elif defined(ARDUINO_ARCH_RP2040)
        if (receive == 1 && send == 0) {
            Serial1.setFIFOSize(256);
            Serial1.begin(9600);
            _s = &Serial1;
        } else if (receive == 9 && send == 8) {
            Serial2.setFIFOSize(256);
            Serial2.begin(9600);
            _s = &Serial2;
        }
#endif   
/***********************************任务初始化************************************/
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_AVR_MEGA2560)
        mySCoop.start();
        mySCoop.yield();
#elif defined(ARDUINO_ARCH_RP2040)
        xTaskCreate(myTask1, "myTask1", 256, NULL, 1, nullptr);
#endif
/*********************************************************************************/
    }
}

#if defined(NRF5) || defined(NRF52833)
void DFRobot_WiFi_IoT_Module_UART::serialInit(uint8_t receive, uint8_t send)
{
    receive = g_PinID[receive];
    send = g_PinID[send];
    MicroBitPin *rxpin = getMicroBitPin(receive);
    MicroBitPin *txpin = getMicroBitPin(send);
#if defined(NRF5)
    uBit.serial.redirect(txpin->name, rxpin->name);
    uBit.serial.baud((int)9600);
#elif defined(NRF52833)
    uBit.serial.redirect(*txpin, *rxpin);
    uBit.serial.setBaudrate((int)9600);
#endif

    uBit.serial.setTxBufferSize(IOT_MAX_MESSAGE+IOT_TOPIC_MAX_LEN + 10);
    uBit.serial.setRxBufferSize(IOT_MAX_MESSAGE+IOT_TOPIC_MAX_LEN + 10);
    uBit.serial.eventOn(ManagedString('\r'), MicroBitSerialMode::ASYNC);
#if defined(NRF5)
    uBit.messageBus.listen(32, 1, mqttTick);
#elif defined(NRF52833)
    uBit.messageBus.listen(DEVICE_ID_SERIAL, CODAL_SERIAL_EVT_DELIM_MATCH, mqttTick);
#endif
    uBit.serial.clearRxBuffer();
    uBit.serial.clearTxBuffer();
    delay(10);
}
#endif

uint8_t DFRobot_WiFi_IoT_Module_UART::loop(uint8_t status, int timeOut)
{
    uint32_t startingTime = millis();
    while (true)
    {
        if(status == obloqStatus)
        {
            return 0;
        }
#if defined(ARDUINO_ARCH_RP2040)
        delay(2);
#else
        delay(50);
#endif
        if((millis() - startingTime) > timeOut)
        {
            if(debugHandle || OBLOQ_DEBUG){
                if (!singleHardSerialFlag || ENABLE_SOFTSERIALPRINT) {
                    printSerial->print("time out status:");
                    printSerial->println(status);
                }
            }
            memset(iotRecvData, 0, IOT_MAX_MESSAGE+IOT_TOPIC_MAX_LEN+10);
            return 1;
        }
    }
}


void DFRobot_WiFi_IoT_Module_UART::receiveObloqData()
{
#if defined(NRF5) || defined(NRF52833)
    if(softSerialFlag) {
        while (_s->available())
        {
            int c = _s->read();
            if (c >= 0) {
                if(con > (IOT_MAX_MESSAGE+IOT_TOPIC_MAX_LEN + 10 - 1)){
                    if(debugHandle || OBLOQ_DEBUG){
                        if (!singleHardSerialFlag || ENABLE_SOFTSERIALPRINT) {
                            printSerial->println("receive message is too long");
                        }
                    }
                } else {
                    iotRecvData[con] = c;
                    con++;
                }
            }
            if (c == '\r')
            {
                iotRecvData[con-1] = 0;
                if(debugHandle || OBLOQ_DEBUG){
                    if (!singleHardSerialFlag || ENABLE_SOFTSERIALPRINT) {
                        // printSerial->println((char *)iotRecvData);
                    }
                }
                argumentParsing((char *)iotRecvData, con-1);
                con = 0;
                break;
            }  
            delay(1);
        }
    } else {
        int n = uBit.serial.getRxBufferSize();
        ManagedString s = uBit.serial.read(n, MicroBitSerialMode::ASYNC);
        n = s.length();
        if (n > (IOT_MAX_MESSAGE+IOT_TOPIC_MAX_LEN + 10 - 1)) {
            if(debugHandle || OBLOQ_DEBUG){
                if (!singleHardSerialFlag || ENABLE_SOFTSERIALPRINT) {
                    printSerial->println("receive message is too long");
                }
            }
        } else {
            strcpy((char *)iotRecvData, s.toCharArray());
            iotRecvData[n-1] = 0;
            if(debugHandle || OBLOQ_DEBUG){
                if (!singleHardSerialFlag || ENABLE_SOFTSERIALPRINT) {
                    // printSerial->println((char *)iotRecvData);
                }
            }
            argumentParsing((char *)iotRecvData, n-1);
        }
    }
#else
    while (_s->available())
    {
        int c = _s->read();
        if (c >= 0) {
            if(con > (IOT_MAX_MESSAGE+IOT_TOPIC_MAX_LEN + 10 - 1)){
                if(debugHandle || OBLOQ_DEBUG){
                    if (!singleHardSerialFlag || ENABLE_SOFTSERIALPRINT) {
                        printSerial->println("receive message is too long");
                    }
                }
            } else {
                iotRecvData[con] = c;
                con++;
            }
        }
            
        if (c == '\r')
        {
            iotRecvData[con-1] = 0;
            if(debugHandle || OBLOQ_DEBUG){
                if (!singleHardSerialFlag || ENABLE_SOFTSERIALPRINT) {
                    // printSerial->println((char *)iotRecvData);
                }
            }
            argumentParsing((char *)iotRecvData, con-1);
            con = 0;
            break;
        }
        
        delay(1);
    }
#endif
}

void DFRobot_WiFi_IoT_Module_UART::argumentParsing(char *data, uint8_t len)
{
    char type = data[1];
    char *p = data;
    switch (type)
    {
        case SYSTEMTYPE:
        {
            if (data[3] == SYSTEMPING) {
                iotDebug(OBLOQ_PING_OK);
                obloqStatus = OBLOQ_PING_OK;
            } else if (data[3] == SYSTEMVERSION) {
                memset(p+len-1, 0, 1);
                iotDebug(OBLOQ_GET_VERSION_OK);
                obloqStatus = OBLOQ_GET_VERSION_OK;
            } else if (data[3] == SYSTEMHEARTBEAT) {
                obloqStatus = OBLOQ_NONE;
            } else {
                obloqStatus = OBLOQ_NONE;
            }
            break;
        }
        case WIFITYPE:
        {
            if (data[3] == WIFIDISCONNECT) {
                obloqStatus = OBLOQ_NONE;        
            } else if (data[3] == WIFICONNECTING) {
                obloqStatus = OBLOQ_NONE;        
            } else if (data[3] == WIFICONNECTED) {
                memset(p+len-1, 0, 1);
                strcpy((char *)IPaddress, p+5);
                iotDebug(OBLOQ_WIFI_CONNECT_OK);
                obloqStatus = OBLOQ_WIFI_CONNECT_OK;  
            } else if (data[3] == WIFICONNECTFAILED) {
                iotDebug(OBLOQ_WIFI_CONNECT_FAILURE);
                obloqStatus = OBLOQ_WIFI_CONNECT_FAILURE; 
            } else {
                obloqStatus = OBLOQ_NONE;
            }
            break;
        }
        case HTTPTYPE:
        {//|3|errcode|message|
            char code[10];
            uint8_t n = 0;
            memset(code, 0, 10);
            p = p + 3;
            while(p[0] != '|' && p[0] != NULL) {
                code[n++] = p[0];
                p++;
            }
            if (strcmp(code, "200") == 0) {
                memset(data+len-1, 0, 1);
                iotDebug(OBLOQ_HTTP_RECEIVE_OK);
                obloqStatus = OBLOQ_HTTP_RECEIVE_OK;
            } else if (strcmp(code, "-1") == 0) {
                obloqStatus = OBLOQ_NONE;
            } else if (strcmp(code, "1") == 0) {
                obloqStatus = OBLOQ_NONE;
            } else {
                obloqStatus = OBLOQ_NONE;
            }
            break;
        }
        case MQTTTYPE:
        {
            if(data[3] == '1') {
                if(data[5] == '1') {
                    if(data[7] == '1') {
                        iotDebug(OBLOQ_MQTT_CONNECT_OK);
                        obloqStatus = OBLOQ_MQTT_CONNECT_OK;
                    } else if (data[7] == '2') {
                        iotDebug(OBLOQ_MQTT_CONNECT_FAILURE);
                        // serialSend("|4|1|4|\r");
                        // serialSend("|4|1|5|\r");
                        obloqStatus = OBLOQ_MQTT_CONNECT_FAILURE;
                    } else {}
                }  else if (data[5] == '2') {
                    if(data[7] == '1') {
                        iotDebug(OBLOQ_SUB_OK);
                        obloqStatus = OBLOQ_SUB_OK;
                    } else if (data[7] == '2') {
                        if(data[9] == '1') {
                            obloqStatus = OBLOQ_NONE;
                        } else if (data[9] == '2') {
                            iotDebug(OBLOQ_SUB_FAILURE);
                            obloqStatus = OBLOQ_SUB_FAILURE;
                        } else {}
                    } else {}
                }  else if (data[5] == '3') {
                    if(data[7] == '1') {
                        iotDebug(OBLOQ_PULISH_OK);
                        obloqStatus = OBLOQ_PULISH_OK;
                    } else if (data[7] == '2') {
                        iotDebug(OBLOQ_PULISH_FAILURE);
                        obloqStatus = OBLOQ_PULISH_FAILURE;
                    } else {}
                }  else if (data[5] == '4') {
                    if(data[7] == '1') {
                        iotDebug(OBLOQ_MQTT_DISCONNECT_OK);
                        obloqStatus = OBLOQ_MQTT_DISCONNECT_OK;
                    } else if (data[7] == '2') {
                        obloqStatus = OBLOQ_NONE;
                    } else {}
                }  else if (data[5] == '5') {
                //|4|1|5|topic|message|
                    char topic[IOT_TOPIC_MAX_LEN + 1];
                    uint8_t n = 0;
                    memset(topic, 0, IOT_TOPIC_MAX_LEN + 1);
                    p = p + 7;
                    while(p[0] != '|' && p[0] != NULL) {
                        if(n > IOT_TOPIC_MAX_LEN - 1){
                            if(debugHandle || OBLOQ_DEBUG){
                                if (!singleHardSerialFlag || ENABLE_SOFTSERIALPRINT) {
                                    printSerial->println("topic too long");
                                }
                            }
                            return;
                        }    
                        topic[n++] = p[0];
                        p++;
                    }
                    for (int i = 0; i < mqttTopicArray.size(); i++) {
                        if(strcmp(topic, mqttTopicArray[i].c_str()) == 0) {
                            if(mqttHandle[i]){
                                memset(data+len-1, 0, 1);
                                mqttHandle[i](++p);
                            }
                        }
                    }
                    iotDebug(OBLOQ_MQTT_RECEIVE_OK);
                    obloqStatus = OBLOQ_MQTT_RECEIVE_OK;
                } else {
                    obloqStatus = OBLOQ_NONE;
                }
            } else {}
            break;     
        }
        default:
            break;
    }
    iotDebug(OBLOQ_MESSAGE_FLAG);
}





