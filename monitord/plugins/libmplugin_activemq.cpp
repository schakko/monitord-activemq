/**
 * monitord-activemq
 * http://github.com/schakko/monitord-activemq
 * Contains *untested* prototyped code
 */
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


using namespace std;
using namespace activemq;
using namespace activemq::core;
using namespace cms;

typedef struct
{
	bool bUseTopic;
	bool bDeliveryModePersistent;
	std::string destUri;
	Destination *destination;
	MessageProducer *producer;
} TopicInfo;

typedef map<string,TopicInfo*> Topics;

class MonitorPlugInActiveMQ : public MonitorPlugIn
{
public:
 	std::string brokerUri;
 	std::string username;
 	std::string password;
 	std::string clientId;
	std::string destUri;
 	unsigned int sendTimeout;
	unsigned int closeTimeout;
	unsigned int producerWindowSize;
	bool bUseCompression;
	bool bClientAck;
	bool bDeliveryModePersistent;
 	bool bConnected;

	Connection* connection;
	Session* session;

	TopicInfo genericTopic;
	Topics topics;

	MonitorPlugInActiveMQ()
	{
		bUseCompression = false;
		bClientAck = false;
 		bConnected = false;
	}

	virtual ~MonitorPlugInActiveMQ()
	{
		Topics::iterator i ;
		TopicInfo *pTopicInfo;
		
	    	// Destroy resources.
		for (i = topics.begin(); i != topics.end(); i++)
		{
			pTopicInfo = i->second;

			try {
				if (pTopicInfo->destination != NULL) { 
					delete pTopicInfo->destination;
				}
	        	}
			catch (CMSException& e) { 
				e.printStackTrace(); 
			}
			
			pTopicInfo->destination = NULL;

			try {
				if (pTopicInfo->producer != NULL) {
					delete pTopicInfo->producer;
				}
			}
			catch (CMSException& e) { 
				e.printStackTrace(); 
			}
			
			pTopicInfo->producer = NULL;
		}

		// Close open resources.
		try {
			if (session != NULL) {
				session->close();
			}

			if (connection != NULL) {
				connection->close();
			}
		}
		catch (CMSException& e) {
			e.printStackTrace(); 
		}

		try {
			if (session != NULL) {
				delete session;
			}
		}
		catch (CMSException& e) { 
			e.printStackTrace(); 
		}

		session = NULL;

		try {
			if (connection != NULL) {
				delete connection;
			}
		} 
		catch (CMSException& e) { 
			e.printStackTrace(); 
		}

		connection = NULL;
	}

	virtual void Show()
	{
		FILE_LOG(logINFO) << "MonitorActiveMQPlugin successfully loaded" ;
	}

	virtual bool processResult(class ModuleResultBase *pRes)
	{
		FILE_LOG(logDEBUG) << "apache mq: processing Result...";

		if (bConnected == false) {
			return false;
		}

		// TODO Produce TextMessage
		return true;
/*
		int pingcounter=0 ;
		while (mysql_ping(&m_mysql) && pingcounter<100)
		{
			usleep(100) ;
			pingcounter++ ;
			FILE_LOG(logINFO) << "mysql connection lost ... trying reconnect"  ;
		}

		if (mysql_ping(&m_mysql))
		{
			FILE_LOG(logERROR) << " unable to reconnect to mysql database" ;
			return false ;
		}

		if (pingcounter>0)
		{
			FILE_LOG(logINFO) << "mysql: connection re-established"  ;
		}


		if ((*pRes)["typ"]=="fms")
		{
			std::string insertString ;
			insertString = createInsertString(pRes,fmsMapping,fmsTable);
			mysql_query(&m_mysql,insertString.c_str()) ;
		} else if ((*pRes)["typ"]=="pocsag")
		{
			std::string insertString ;
			insertString = createInsertString(pRes,pocsagMapping,pocsagTable);
			mysql_query(&m_mysql,insertString.c_str()) ;
		}else if ((*pRes)["typ"]=="zvei")
		{
			std::string insertString;
			insertString = createInsertString(pRes,zveiMapping,zveiTable);
			mysql_query(&m_mysql,insertString.c_str()) ;
		}


		return true ;
*/	}

	virtual bool initProcessing(class MonitorConfiguration* configPtr, XMLNode config)
	{
		activemq::library::ActiveMQCPP::initializeLibrary();

		// read default configuration for topics
		readTopic(config, genericTopic, genericTopic);

		brokerUri 	= getNodeText(config, "brokerUri", "tcp://127.0.0.1:61616");
		username 	= getNodeText(config, "username", "");
		password 	= getNodeText(config, "password", "");
		clientId 	= getNodeText(config,"clientId", "");

		sendTimeout 	= getNodeInt(config, "sendTimeout", 0);
		closeTimeout 	= getNodeInt(config, "closeTimeout", 0);
		producerWindowSize = getNodeInt(config, "producerWindowSize", 0);
		bUseCompression = getNodeBool(config, "useCompression", false);
		bClientAck 	= getNodeBool(config, "clientAck", false);
		
		std::string logFile	= getNodeText(config, "logfile", "screen");
		std::string logLevel	= getNodeText(config, "loglevel", "INFO");

		#ifdef WIN32
		if (!(logFile == "screen")) {
			FILE* pFile = fopen(logFile.c_str(), "a");
			Output2FILE::Stream() = pFile;

		}

		FILELog::ReportingLevel() = FILELog::FromString(logLevel);
		FILE_LOG(logINFO) << "logging started";
		#endif

		auto_ptr<ActiveMQConnectionFactory> connectionFactory(new ActiveMQConnectionFactory());

		// create a connection
		try {
			connectionFactory->setUsername(username);
			connectionFactory->setPassword(password);
			connectionFactory->setClientId(clientId);
			connectionFactory->setBrokerURI(brokerUri);
			connectionFactory->setUseCompression(bUseCompression);
			connectionFactory->setSendTimeout(sendTimeout);
			connectionFactory->setCloseTimeout(closeTimeout);
			connectionFactory->setProducerWindowSize(producerWindowSize);

		        connection = connectionFactory->createConnection();
	        	connection->start();

			if (bClientAck) {
                		session = connection->createSession(Session::CLIENT_ACKNOWLEDGE);
			} 
			else {
				session = connection->createSession(Session::AUTO_ACKNOWLEDGE);
			}

			bConnected = true;
		} 
		catch (CMSException& e) {
			FILE_LOG(logERROR) << "Could not connect to messaging queue \"" << brokerUri << "\" with username=\"" << username << "\"";
			return false;
		}

		parseTopics(config);

		Topics::iterator i;
		TopicInfo *pTopicInfo;
		
		// create new producers
		for (i = topics.begin(); i != topics.end(); i++)
		{
			pTopicInfo = i->second;
			
			if (pTopicInfo->bUseTopic) {
				pTopicInfo->destination = session->createTopic(pTopicInfo->destUri);
			} 
			else {
				pTopicInfo->destination = session->createQueue(pTopicInfo->destUri);
			}

			pTopicInfo->producer = session->createProducer(pTopicInfo->destination);

			if (pTopicInfo->bDeliveryModePersistent) {
				pTopicInfo->producer->setDeliveryMode(DeliveryMode::PERSISTENT);
			} 
			else {
				pTopicInfo->producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
			}
		}
	}

	virtual bool quitProcessing() 
	{
		return true;
	}

	void parseTopics(XMLNode config)
	{
		XMLNode topicNode;
		Topics::iterator i;
		TopicInfo pTopicInfo;
		
		// alle Topics initalisieren
		for (i=topics.begin(); i != topics.end(); i++)
		{
			pTopicInfo = static_cast<TopicInfo>(*(i->second));
			initalizeTopic(pTopicInfo, genericTopic);
		}

		int nTopic=config.nChildNode("topic");
		
		for (int num=0; num<nTopic ; ++num)
		{
			if (!((topicNode = config.getChildNode("topic", num))).isEmpty()) {
				std::string typ = topicNode.getAttribute("type") ;

				pTopicInfo = static_cast<TopicInfo>(*((topics.find(typ))->second));

				if (topics.count(typ) == 1) {
					readTopic(topicNode, pTopicInfo, genericTopic) ;
				}
			}
		}
	}

	void initalizeTopic(TopicInfo &topicInfo, TopicInfo &referenceTopic)
	{
		if (&referenceTopic != NULL) {
			topicInfo.bUseTopic = referenceTopic.bUseTopic;
			topicInfo.bDeliveryModePersistent = referenceTopic.bDeliveryModePersistent;
			topicInfo.destUri = referenceTopic.destUri;
		}
	}

	void readTopic(XMLNode &config, TopicInfo &topicInfo, TopicInfo &referenceTopic)
	{
		if (&topicInfo == NULL) {
			initalizeTopic(topicInfo, referenceTopic);
		}

		if (config.nChildNode("useTopic") >= 1) {
			topicInfo.bUseTopic = getNodeBool(config, "useTopic", false);
		}

		if (config.nChildNode("deliveryModePersistent") >= 1) {
			topicInfo.bDeliveryModePersistent = getNodeBool(config, "deliveryModePersistent", false);
		}

		if (config.nChildNode("destUri") >= 1) {
			topicInfo.destUri = getNodeText(config, "destUri", "monitord");
		}
	}
};

class MonitorPlugInActiveMQFactory : public MonitorPlugInFactory
{
public:
	MonitorPlugInActiveMQFactory()
	{
	}

	~MonitorPlugInActiveMQFactory()
	{
	}

	virtual MonitorPlugIn * CreatePlugIn()
	{
		return new MonitorPlugInActiveMQ();
	}
};

DLL_EXPORT void * factory0( void )
{
	return new MonitorPlugInActiveMQFactory;
}
