<!--
 Copyright (c) 2021 Qizhou Guo
 
 This software is released under the MIT License.
 https://opensource.org/licenses/MIT
-->

## `epoll(2)`

### Level-triggered and edge-triggered

* The default behavior of an epoll event is Level-triggered, which is the same as `poll(2)`. An event can be configured as edge-triggered using `EPOLLET` flags in `epoll_ctl`.

* In ET, when a read event occurs, it only read partial data so that there is still more data available on that file descriptor. However, when calling `epoll_wait` in the next loop, it will probably hang despite the available data still present in the file input buffer, meanwhile the remote peer might be expecting a response based on the data it already sent. The semantics for this is that ET delivers events only when changes occur on the monitored file descriptor. 

* To use ET, nonblocking file descriptors should be used to avoid having a blocking read or write starve a task that is handling multiple file descriptor. Beside, it should keep on reading/writing the file descriptor until read/write return `EAGAIN`.

* Compared with ET, LT only have to read once each time a read event occurs, so it can reduce the number of times for system calls.

* When using LT, `epoll(2)` can be seen as a faster `poll(2)` which both have the same level-triggered semantics.

### `EPOLLONSHOT`

* When multiple events are generated, the caller can specify the `EPOLLONESHOT` flag, to tell `epoll(2)` to disable the associated file descriptor after the receipt of an event with `epoll_wait(2)`. When this flag is set, it is the caller's responsibility to rearm the file descriptor using `epoll_ctl(2)` with `EPOLL_CTL_MOD`.

