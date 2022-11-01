# peer2peer_application
Access to the internet has reshaped the world in a very short amount of time. From
sending and receiving emails to navigation and entertainment, the internet touches every aspect
of how people live, work and socialize. Many useful and entertaining applications have been
developed, however this would not be possible without networking protocols. These protocols
are a set of rules which determine the method of data transmission between different devices on
the same network. The application developer designs an application architecture depending on
the application structure over various end systems. There are predominantly two types of
application architectures: client-server architecture and Peer to Peer (P2P) architecture.

In client-server architecture, such as a Web application, there exists a host, a server,
which manages most of the resources and services requests from many other hosts, ie, clients. On
the other hand, in a Peer to Peer architecture, applications allow direct communication between
pairs of periodically connected hosts. These hosts are called peers, or devices such as laptops,
desktops, which are controlled by the users. Peer to Peer applications can be beneficial when
distributing large files from a server to multiple hosts. With a client-server architecture, the
server must transfer a copy of the file to each client. However, with P2P, the file distribution puts
less burden on the server, as each peer can directly communicate with another and transfer parts
of the file it has received.

This project involves the development of a Peer to Peer application consisting of an index
server and three peers. These peers are able to send and receive content among themselves with
the assistance of the index server. The peers can be both clients and servers, meaning that a peer
can both serve the content and also request to download the content. UDP sockets are used to
allow communication between the index server and peers, while TCP sockets are used to
download any available content.

The use of UDP and TCP sockets are part of a technique to connect nodes (devices) on a
network called Socket Programming. Transmission Control Protocol, known as TCP, is a method
that requires a connection that is established using a three way handshake between a source and
destination to transfer data. This allows for information to be continuously streamed across a
network that can be ensured to be accurate upon receiving. User Datagram Protocol, also known
as UDP, differs from TCP in that it does not need a connection in order to send information. This
is done by sending packets of information at a time.

The implementation of this project consists of a total of 7 sockets in the network. Sockets
s2 (used for searching), s3,s5 are UDP sockets that allow different peers to communicate with the
index server. Sockets s1(used to download) and s4 are TCP sockets. First, Peer 1 creates a TCP
socket to register its content to the index server, establishing itself as a content server. Peer 2
contacts the index server to find the address of the content server before creating a TCP
connection to download the content from the content server. Once this download is complete,
Peer 2 registers itself as a content server of its downloaded content. Peer 3 follows the same
procedure as Peer 2, and the index server responds with the address of either Peer 1 or Peer 2 to
download the content. Finally, Peer 3 is able to download the content from Peer 2.
The Protocol Data Unit (PDU) consists of a type and data, where the type field is one byte and
the data field has a maximum size of 100, except for the C-type PDU which has a size of the size
of the content, as it carries the content. The PDU type and its function, along with direction are
listed below in Table 1. There are eight PDU types which allow data transfer between the hosts.

