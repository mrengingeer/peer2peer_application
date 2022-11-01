# peer2peer_application
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

