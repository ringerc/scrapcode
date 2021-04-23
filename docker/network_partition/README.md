A demo of docker networking between containers with ability to isolate
communication between the nodes while not granting excessive priviliges.

Uses user defined locally scoped bridge network.

## Run the demo

Run with

    docker-compose up

You'll see chatter between the nodes on stderr. Now you can mess with their
networking externally using the methods below.

## Detach and attach containers

To break connectivity:

    docker disconnect unstable network_partition_client_1

to resume connectivity

    docker connect unstable network_partition_client_1

This will make the containers nonroutable. If there are existing TCP
connections they won't break straight away, they'll keep retrying until their
timeouts are hit, and they'll buffer sent data for retry.

## Messing with connectivity using iptables

You can use iptables within each container. Because each container has its own
network namespace, this won't mess up the host's iptables rules.

Within either container, it's possible to do things like:

    # blackhole from other container
    iptables -A INPUT -s client -j DROP

    # delete blackhole
    iptables -D INPUT 1

    # send TCP RST for other container
    iptables -A INPUT -s client -j DROP

    # restore connectivity
    iptables -D INPUT 1

You can run this with `docker exec` e.g.

    docker exec network_partition_client_1 iptables -A INPUT -s server -j DROP

## Docker-compose not required

Example uses docker compose, but it's simple to do with plain docker using the
`docker network create` command and the `--network` and `--cap-add NET_ADMIN`
flags to `docker run`.

e.g.

    docker build -t net-demo .

    docker network create unstable

    docker run \
        --detach -n network_partition_client_1 \
        --entrypoint /usr/local/bin/client \
        --network unstable \
        net-demo

    docker run \
        --detach -n network_partition_server_1 \
        --entrypoint /usr/local/bin/server \
        --network unstable \
        net-demo

## Note on docker and network namespaces

Docker uses network namespaces, but `ip netns` won't see them. Docker tracks it
in `/var/run/docker/netns` but `ip netns` tracks them in `/var/run/netns`.

You can hack around this with 

    sudo rmdir /var/run/netns
    sudo ln -s /var/run/docker/netns /var/run/netns

