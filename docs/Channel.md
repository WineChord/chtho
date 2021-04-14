<!--
 Copyright (c) 2021 Qizhou Guo
 
 This software is released under the MIT License.
 https://opensource.org/licenses/MIT
-->

## `Channel` class

A `Channel` class provide encapsulation upon:

* a file descriptor
* events to be monitored
* callback function for read/write/close/error events

So it can be considered as the _event_ object for an eventloop. 
Each `Channel` object belongs only to one eventloop throughout 
its lifetime. Each eventloop can have multiple `Channel` as 
_events_ to monitor.


When certain event happens, it will run the callback function 
which is registered in the `Channel`. 

