HOST = localhost
PORT = 2379
CLIENT_CERTS = /etc/ssl/etcd
CA_CERTS = /etc/ssl/ca
value = ""

configmgrclient:
	python3.6 main.py -certFile $(CLIENT_CERTS)/etcd-client.pem \
	-privateFile $(CLIENT_CERTS)/etcd-client-key.pem \
	-trustFile $(CA_CERTS)/ca.pem \
	-key $(key) \
	-action $(action) \
	-value $(value)

configmgrclient_insecure:
	python3.6 main.py -certFile "" -privateFile "" -trustFile "" \
	-key $(key) \
	-action $(action) \
	-val $(value)
