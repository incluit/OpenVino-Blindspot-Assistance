package util

import (
	configmgr "ConfigManager"
	"log"
	"os"
	"strconv"
	"strings"

	"github.com/golang/glog"
)

const (
	maxClients     = 200
	maxSubscribers = 200
	maxEndPoints   = 100
)

// GetMessageBusConfig - constrcuts config object based on topic type(pub/sub),
// message bus type(tcp/ipc) and dev/prod mode
func GetMessageBusConfig(topic string, topicType string, devMode bool, cfgMgrConfig map[string]string) map[string]interface{} {
	var subTopics []string
	var topicConfigList []string
	appName := os.Getenv("AppName")
	msgbusHwm, err := strconv.ParseInt(os.Getenv("ZMQ_RECV_HWM"), 10, 64)
	if err != nil {
		glog.Infof("ZMQ_RECV_HWM not found. Using default value")
		msgbusHwm = -1
	}
	cfgMgrCli := configmgr.Init("etcd", cfgMgrConfig)
	if cfgMgrCli == nil {
		glog.Errorf("Config Manager initialization failed...")
	}

	topic = strings.TrimSpace(topic)
	if strings.ToLower(topicType) == "sub" {
		subTopics = strings.Split(topic, "/")
		topic = subTopics[1]
	}

	if topicType == "server" {
		topicConfig := os.Getenv("Server")
		if len(topicConfig) == 0 {
			glog.Errorf("Please set the Server config to start the server")
			return nil
		} else {
			topicConfigList = strings.Split(topicConfig, ",")
		}
	} else {
		topicConfig := os.Getenv(topic+"_cfg")
		if len(topicConfig) == 0 {
			glog.Errorf("Please set the %s_cfg to start the Publisher/Subscriber", topic)
			return nil
		} else {
			topicConfigList = strings.Split(topicConfig, ",")
	        }
	}
	var messageBusConfig map[string]interface{}
	topicConfigList[0] = strings.TrimSpace(topicConfigList[0])
	topicConfigList[1] = strings.TrimSpace(topicConfigList[1])
	messageBusConfig = map[string]interface{}{
		"type": topicConfigList[0],
	}
	if msgbusHwm != -1 {
		messageBusConfig["zmq_recv_hwm"] = msgbusHwm
	}
	if topicConfigList[0] == "zmq_tcp" {
		address := strings.Split(topicConfigList[1], ":")
		hostname := address[0]
		port, err := strconv.ParseInt(address[1], 10, 64)
		if err != nil {
			glog.Errorf("string to int64 converstion Error: %v", err)
		}
		hostConfig := map[string]interface{}{
			"host": hostname,
			"port": port,
		}
		if strings.ToLower(topicType) == "pub" {
			messageBusConfig["zmq_tcp_publish"] = hostConfig
			if !devMode {
				var allowedClients []interface{}
				subscribers := strings.Split(os.Getenv("Clients"), ",")
				if len(subscribers) > maxSubscribers {
					glog.Errorf("Exceeded Max Subscribers %d", len(subscribers))
				} else {
					for _, subscriber := range subscribers {
						subscriber = strings.TrimSpace(subscriber)
						clientPublicKey, err := cfgMgrCli.GetConfig("/Publickeys/" + subscriber)
						if err != nil {
							glog.Errorf("ConfigManager couldn't get Subscriber's Public Key %v", err)
						}

						if clientPublicKey != "" {
							allowedClients = append(allowedClients, clientPublicKey)
						}
					}
				}
				serverSecretKey, err := cfgMgrCli.GetConfig("/" + appName + "/private_key")
				if err != nil {
					log.Fatal(err)
				}
				messageBusConfig["allowed_clients"] = allowedClients
				hostConfig["server_secret_key"] = serverSecretKey
			}
		} else if strings.ToLower(topicType) == "sub" {
			messageBusConfig[topic] = hostConfig
			if !devMode {
				subTopics[0] = strings.TrimSpace(subTopics[0])
				serverPublicKey, err := cfgMgrCli.GetConfig("/Publickeys/" + subTopics[0])
				if err != nil || serverPublicKey == "" {
					glog.Errorf("ConfigManager couldn't get Publisher's Public Key")
				}
				clientSecretKey, err := cfgMgrCli.GetConfig("/" + appName + "/private_key")
				if err != nil {
					glog.Errorf("ConfigManager couldn't get Subscriber's Private Key %v", err)
				}
				clientPublicKey, err := cfgMgrCli.GetConfig("/Publickeys/" + appName)
				if err != nil {
					glog.Errorf("ConfigManager couldn't get Subscriber's Public Key %v", err)
				}
				hostConfig["server_public_key"] = serverPublicKey
				hostConfig["client_secret_key"] = clientSecretKey
				hostConfig["client_public_key"] = clientPublicKey
			}
		} else if strings.ToLower(topicType) == "server" {
			messageBusConfig[topic] = hostConfig
			if !devMode {
				var allowedClients []interface{}
				clients := strings.Split(os.Getenv("Clients"), ",")
				if len(clients) > maxClients {
					glog.Errorf("Exceeded Max Clients %d", len(clients))
				} else {
					for _, client := range clients {
						client = strings.TrimSpace(client)
						clientPublicKey, err := cfgMgrCli.GetConfig("/Publickeys/" + client)
						if err != nil {
							glog.Errorf("ConfigManager couldn't get Client's Public Key %v", err)
						}
						if clientPublicKey != "" {
							allowedClients = append(allowedClients, clientPublicKey)
						}
					}
				}
				serverSecretKey, err := cfgMgrCli.GetConfig("/" + appName + "/private_key")
				if err != nil {
					log.Fatal(err)
				}
				messageBusConfig["allowed_clients"] = allowedClients
				hostConfig["server_secret_key"] = serverSecretKey
			}
		} else if strings.ToLower(topicType) == "client" {
			messageBusConfig[topic] = hostConfig
			if !devMode {
				endPoints := strings.Split(os.Getenv("RequestEP"), ",")
				if len(endPoints) > maxEndPoints {
					glog.Errorf("Exceeded End points limit %d", len(endPoints))
				} else {
					for _, endpoint := range endPoints {
						if topic == endpoint {
							clientPublicKey, err := cfgMgrCli.GetConfig("/Publickeys/" + appName)
							if err != nil {
								glog.Errorf("ConfigManager couldn't get Client's Public Key %v", err)
							}

							clientSecretKey, err := cfgMgrCli.GetConfig("/" + appName + "/private_key")
							if err != nil {
								log.Fatal(err)
							}

							serverPublicKey, err := cfgMgrCli.GetConfig("/Publickeys/" + topic)
							if err != nil || serverPublicKey == "" {
								glog.Errorf("ConfigManager couldn't get Server's Public Key %v", err)
							}

							hostConfig["server_public_key"] = serverPublicKey
							hostConfig["client_secret_key"] = clientSecretKey
							hostConfig["client_public_key"] = clientPublicKey
							break
						}
					}
				}
			}
		} else {
			panic("Unsupported Topic Type!!!")
		}
	} else if topicConfigList[0] == "zmq_ipc" {
		messageBusConfig["socket_dir"] = topicConfigList[1]
	} else {
		panic("Unsupported MessageBus Type!!!")
	}
	return messageBusConfig
}

//GetTopics - returns list of topics based on topic type
func GetTopics(topicType string) []string {
	var topics []string
	if strings.ToLower(topicType) == "pub" {
		topics = strings.Split(os.Getenv("PubTopics"), ",")
	} else {
		topics = strings.Split(os.Getenv("SubTopics"), ",")
	}
	return topics
}
