SHELL=/bin/bash
project = diploma
account ?= poka_net
dir ?= ${project}
endpoint ?= "https://testnet.wax.pink.gg"
docker = docker run -it -v $(CURDIR):/wax -v home:/root winwinsoftware/wax-dev:latest
docker_nt = docker run -i -v $(CURDIR):/wax -v home:/root winwinsoftware/wax-dev:latest

collection ?= hz
token ?= qwe

ifeq ($(OS),Windows_NT)
    MY_OS := Windows
else
    MY_OS := $(strip $(shell uname))
endif

all: build deploy

build:
	$(docker) bash -c "mkdir -p /root/build && cd /root/build && cmake -DDIPLOMA_TOKEN=$(token) -DDIPLOMA_COLLECTION=$(collection) /wax && make VERBOSE=1 && rm -rf /wax/$(project) || true && cp -Rap /root/build /wax/$(project)"
clean:
	$(docker) bash -c "rm -rf /root/build"
	rm -rf $(project)
create:
	$(docker) bash -c 'if [ ! -f /root/wallet ]; then cleos wallet create -f /root/wallet; else echo "wallet alread created"; exit -1; fi'
addkey:
	$(docker) bash -c 'cleos wallet unlock < /root/wallet; cleos wallet import'
deploy:
	$(docker) bash -c "cleos wallet unlock < /root/wallet; cleos -u $(endpoint) set contract $(account) /wax/$(project)/ -p $(account)@active"
deploy-mainnet:
	$(docker) bash -c "cleos wallet unlock < /root/wallet; cleos -u $(endpoint) set contract $(account) /wax/$(project)/ -p $(account)@active"
addcode:
	$(docker) bash -c "cleos wallet unlock < /root/wallet; cleos -u $(endpoint) set account permission $(account) active --add-code"
console:
	$(docker) bash
account:
	$(docker) cleos -u $(endpoint) get account $(account) -j
action:
	$(docker_nt) bash <<< 'cleos wallet unlock < /root/wallet; cleos -u $(endpoint) push action $(target) $(action) $(data) -p $(account)@active'
table:
	$(docker_nt) bash <<< 'cleos -u $(endpoint) get table $(target) $(scope) $(table) $(args)'