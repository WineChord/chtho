<!--
 Copyright (c) 2021 Qizhou Guo
 
 This software is released under the MIT License.
 https://opensource.org/licenses/MIT
-->

## RAII

There are a few design points illustrating the use of RAII (Resource Acquisition Is Initialization)

* `TcpConnection` owns the connection file descriptor. 
It hold a unique pointer to a connection `Socket`. In 
the ctor of `TcpConnection`, it will create an RAII object 
of `Socket`. The file descriptor binded to that `Socket` is
close by the dtor of `Socket` (by calling `::close`). So 
when `TcpConnection` is destroyed, the dtor of `TcpConnection` 
doesn't need to do anything with the `Socket`'s file descriptor, 
because the `Socket` object itself _can_ manage it. The essence 
of RALL is resource lifetime management: you don't have to 
manually call new/delete to control resource usage. 