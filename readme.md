# mcrx - Multicast receive

A simple tool to check if data is received on a multicast address.


# Build 


```
$ cd mcrx
$ mkdir build
$ cd build 
$ cmake ..
$ make
```

# Usage


Pass multicast address as argument to `mcrx`. Multiple multicast on same ip with a different port can be 
received by separating ports with a `:`.

Example: receive data on 224.4.4.4:4444 and 224.4.4.4:5555

```
$ mcrx 224.4.4.4:4444:5555
Joined 224.4.4.4
Joined 224.4.4.4
224.4.4.4:4444              250.79 KBytes
224.4.4.4:5555              1.5484 MBytes

```

