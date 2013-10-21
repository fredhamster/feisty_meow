
What are Octopi...



CROMP protocol

CROMP stands for Compact Remote Object Model Platform. CROMP is a communication protocol implemented in C++. Its goal is to provide a comfortable object model for shipping data structures as requests over the network and receiving replies back. While we use CROMP to implement a Remote Procedure Call (RPC) interface, it is also suited to asynchronous data exchanges driven by client or server.

The basic unit of data in CROMP is an infoton. This is a C++ object that serves as a base class for all packets of data sent over the network. It's a term that bundles together the concepts of information and photons to get a smallest information unit in CROMP.

There is also a coordinating object called the Octopus that processes infotons and can produce infotons as results. This object is the core service provider for any CROMP based system, but it mainly handles routing infotons to the proper servicing object.

In CROMP, the objects that actually do the work are called tentacles. Each tentacle forms one arm of the octopus in a metaphorical sense; the tentacle is reponsible for handling one particular type of infoton and also may need to generate reply infotons to requests that have been made. An octopus can have an arbitrary number of tentacles attached to it for handling data (not just eight).

The CROMP subsystem wraps an octopus object and gives it a network presence. The CROMP layer defines network packing rules that make up the protocol. It also provides a transport mechanism based on sockets for infotons to be sent and received. This supports a CROMP service residing at a particular port on a particular host and processing infotons sent to it over the network.


