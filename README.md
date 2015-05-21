TCP Port Forward
================

This extremely simple tool merely opens a port on the local machine and forwards the connection to another machine.  It's based on [portforward.c], and the DNS resolution happens at the time of the connection.


Usage
-----

First, either compile by hand or simply run `make`:

    make

This builds an executable called `portforward`.  From here, you need to specify two settings and can specify a third.

    portforward LISTEN_PORT FORWARD_NAME [FORWARD_PORT]

* `LISTEN_PORT`: The port to listen to on the current machine.
* `FORWARD_NAME`: DNS name of destination.
* `FORWARD_PORT`: Destination port for the forwarded traffic.  Optional, defaults to the same port as `LISTEN_PORT`.


Why resolve later?
------------------

That's a great question.  Let's build a scenario to better explain why this could be a useful thing.  I am sometimes working with applications in the cloud that do not handle scaling well.  It is entirely possible that a cluster of MongoDB databases that were up yesterday are gone today, but in their place are a set of brand new ones.

These new ones were rolled on slowly and the replica set was preserved.  We also have [Consul] up and running so our DNS name of "master.mongo.service.consul" always points to the master of the replica set.  The IPs of the new machines do not match the old ones, but looking them up by name again will resolve to the correct IP addresses.

My ancient application was robust enough to reconnect in case Mongo disappeared for a bit, but it can't handle when IP addresses change.  With this proxy in place on the same machine as the application, I can run this program

    portforward 27017 master.mongo.service.consul

Now all that's left is to configure my application to use the mongo at "localhost:27017" and I'm set.

[Consul]: https://consul.io/
[portforward.c]: https://github.com/antialize/utils/blob/master/portforward.c
