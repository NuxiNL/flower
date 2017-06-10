flower: a label-based network backplane service
===============================================

Flower is a service discovery service. It connects clients to servers based on
their labels, which are called 'connection constraints'. For example, an
application may subscribe as an LDAP server and request incoming connections;
then, any other application searching for LDAP servers will cause both
applications to receive a socketpair to each other.


