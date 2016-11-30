# Architecture




Tracking only occurs while a client is connected to one of the nodes.

When a client connects, the clients mac address is broadcasted among the mesh nodes to be tracked.

When a promiscuous mode packet is received, if the MAC address is one that should be tracked, it is stored

When a client disconnects, the nodes remove the MAC from the clients to track

Each node periodically sends out the RSSI of any node that has been seen within the last 5(?) seconds
