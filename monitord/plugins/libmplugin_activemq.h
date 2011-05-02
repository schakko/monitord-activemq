#ifndef MPLUGINACTIVEMQ_H_
#define MPLUGINACTIVEMQ_H_

#ifdef PLUGINS
#include <typeinfo>
#include <iostream>

#ifdef WIN32
#define usleep Sleep
#include <windows.h>
#endif

#include "mplugin.h"
#include "../MonitorLogging.h"
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/util/Config.h>
#include <activemq/library/ActiveMQCPP.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>

#define ACTIVEMQ_KEY_POCSAG "pocsag"
#define ACTIVEMQ_KEY_FMS "fms"
#define ACTIVEMQ_KEY_ZVEI "zvei"
#define ACTIVEMQ_XMLNODE_BROKERURI "brokerUri"
#define ACTIVEMQ_XMLNODE_USERNAME "username"
#define ACTIVEMQ_XMLNODE_PASSWORD "password"
#define ACTIVEMQ_XMLNODE_CLIENTID "clientId"
#define ACTIVEMQ_XMLNODE_SENDTIMEOUT "sendTimeout"
#define ACTIVEMQ_XMLNODE_CLOSETIMEOUT "closeTimeout"
#define ACTIVEMQ_XMLNODE_PRODUCERWINDOWSIZE "producerWindowSize"
#define ACTIVEMQ_XMLNODE_USECOMPRESSION "useCompression"
#define ACTIVEMQ_XMLNODE_CLIENTACK "clientAck"
#define ACTIVEMQ_XMLNODE_LOGFILE "logfile"
#define ACTIVEMQ_XMLNODE_LOGLEVEL "loglevel"
#define ACTIVEMQ_XMLNODE_TOPIC "topic"
#define ACTIVEMQ_XMLATTR_TYPE "type"
#define ACTIVEMQ_XMLNODE_USETOPIC "useTopic"
#define ACTIVEMQ_XMLNODE_DELIVERYMODEPERSISTENT "deliveryModePersistent"
#define ACTIVEMQ_XMLNODE_DESTURI "destUri"


typedef struct
{
	bool bUseTopic;
	bool bDeliveryModePersistent;
	std::string destUri;
	cms::Destination *destination;
	cms::MessageProducer *producer;
} TopicInfo;

typedef std::map<std::string,TopicInfo*> Topics;
typedef std::pair<std::string,TopicInfo*> PairMapping;

class MonitorPlugInActiveMQ : public MonitorPlugIn
{
public:
	MonitorPlugInActiveMQ();
	~MonitorPlugInActiveMQ();
 	std::string m_brokerUri;
 	std::string m_username;
 	std::string m_password;
 	std::string m_clientId;
	std::string m_destUri;
 	unsigned int m_sendTimeout;
	unsigned int m_closeTimeout;
	unsigned int m_producerWindowSize;
	bool m_bUseCompression;
	bool m_bClientAck;
	bool m_bDeliveryModePersistent;
 	bool m_bConnected;
	bool m_bTopicsInitialized;

	cms::Connection* m_connection;
	cms::Session* m_session;

	TopicInfo m_genericTopic;
	Topics m_topics;

	bool initProcessing(class MonitorConfiguration* configPtr,XMLNode config);
	bool processResult(class ModuleResultBase *pRes);
	bool quitProcessing();
	void Show();

	Topics getTopics();
	void setTopics(Topics &topics);
	void shutdownActiveMQCPPLibrary();
	void initializeActiveMQCPPLibrary();
	void initializeConfiguration(XMLNode config);
	void initializeConnectionFactory(activemq::core::ActiveMQConnectionFactory *connectionFactory);
	bool initializeActiveMqConnection();
	void initializeTopics(Topics &topics);
	void parseTopics(XMLNode config, Topics &topics, TopicInfo &referenceTopic);
	void parseTopic(XMLNode config, TopicInfo &topicInfo, TopicInfo &referenceTopic);
	void initializeTopic(TopicInfo &topicInfo, TopicInfo &referenceTopic);
};


#endif
#endif /*MPLUGINACTIVEMQ_H_*/
