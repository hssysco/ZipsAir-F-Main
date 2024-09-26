#ifndef __MQTT_H__
#define __MQTT_H__

typedef struct MqttInfo {
	void *mqttHdl;

	char *ClientId;
	char *Topic;
	const char *Addr;
	int Port;
	char *Username;
	char *Password;

	unsigned char Connected;
	int StatusPeriod;
	int Id;
	bool ExistCerts;

	void (*fnMsgCallback)(void *);
	
}MqttInfoT;

void InitMqtt( MqttInfoT *pMqttInfo);

#endif /* __MQTT_H__ */

