DC				:= docker-compose

# build
all:
	$(DC) build

# start
up:
	$(DC) up --build

down:
	$(DC) down --volumes

clean:
	$(DC) down --rmi all --volumes --remove-orphans

rebuild:
	$(DC) build --no-cache

# into containers
.PHONY: vm
vm:
	$(DC) exec $@ /bin/bash
