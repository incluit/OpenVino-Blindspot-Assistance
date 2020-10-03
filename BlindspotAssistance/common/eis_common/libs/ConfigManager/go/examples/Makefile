HOST = localhost
PORT = 2379
CLIENT_CERTS = /etc/ssl/etcd
CA_CERTS = /etc/ssl/ca
value = ""

build:
	go build main.go

configmgrclient: build
	./main -certFile $(CLIENT_CERTS)/etcd-client.pem \
	-privateFile $(CLIENT_CERTS)/etcd-client-key.pem \
	-trustFile $(CA_CERTS)/ca.pem \
	-key $(key) \
	-action $(action) \
	-val $(value)


configmgrclient_insecure: build
	./main -certFile "" -privateFile "" -trustFile "" \
	-key $(key) \
	-action $(action) \
	-val $(value)
clean:
	rm -rf main
