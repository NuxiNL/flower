# flow·er: a label-based networking daemon

## Introduction

On most operating systems that are in use today, programs are allowed to
create connections to other systems on the network rather liberally. As
this is often a bad thing, systems provide additional kernel subsystems
that allow you to restrict this, called _firewalls_. The downside of
firewalls are that they will always remain _bolted on to the system_.
There is no way a regular, unprivileged processes can programmatically
(and portably) place restrictions on subprocesses they are about to
spawn.

Programming interfaces such as the Berkeley sockets API and
`getaddrinfo()` are also strongly coupled against IPv4, IPv6, TCP, UDP
and DNS. Operating systems provide little to no features to experiment
with [custom transport protocols](https://cr.yp.to/tcpip/minimalt-20130522.pdf)
and [domain-specific name services](https://pdos.csail.mit.edu/6.824/papers/borg.pdf).

With Flower, we're trying to move the responsibility of setting up
network connections into a separate daemon, called the Switchboard.
Processes can register _servers_ on the Switchboard, to which _clients_
can connect. When establishing a connection, the Switchboard creates a
socket pair and uses [Unix file descriptor passing](https://keithp.com/blogs/fd-passing/) to
hand out socket endpoints to both processes. Special processes, called
_ingresses_ and _egresses_, can push existing socket file descriptors
(e.g., IPv4, IPv6) into the Switchboard.

## Identification through labels

The Switchboard identifies its clients and servers by a set of string
key-value pairs (_labels_). Clients and servers are allowed to establish
a connection if there are no labels for which the keys match, but the
values contradict. Once connected, both parties obtain a copy of the
union of both sets of labels. This allows clients to pass connection
metadata (network addresses, hostnames, local usernames) to servers and
vice versa.

An interesting aspect of the Switchboard is that these labels also act
as a security mechanism. Handles to the Switchboard have a set of labels
attached to it that can only be extended over time. Every time a new
label is attached, the size of the space in which it can establish
connections is reduced. The Switchboard's security model is
capability-based.

## Example usage

A Switchboard process can be started, simply by running:

    flower_switchboard /var/run/flower

Flower ships with a utility similar to `nc(1)`, called `flower_cat`,
that allows you to easily start clients and servers. A simple one-shot
server can be started by running:

    flower_cat -l \
        /var/run/flower \
        '{"program": "demo", "server_name": "My server v0.1"}'

A client can be started similarly:

    flower_cat \
        /var/run/flower \
        '{"program": "demo", "client_name": "My client v0.1"}'

This will establish a connection, having labels:

    {
        "client_name": "My client v0.1",
        "program": "demo",
        "server_name": "My server v0.1"
    }

Of course, it makes far more sense to communicate with the Switchboard
programmatically, as opposed to using `flower_cat`. The Switchboard
makes use of [ARPC](https://github.com/NuxiNL/arpc). Its protocol is
specified in a [.proto file](flower/protocol/switchboard.proto).

## Motivation

Flower has been developed for use with
[CloudABI](https://nuxi.nl/cloudabi/), a POSIX-like runtime environment
that is strongly sandboxed. CloudABI doesn't allow programs to open
arbitrary network connections (i.e., there is no `bind()` and
`connect()`). A system like Flower is thus needed to grant processes
access to the network in a sensible way.
