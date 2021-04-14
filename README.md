# chtho

A Tcp network library implemented in C++11.

## Quick Start

* Clone the code and build:

```
$ git clone https://github.com/WineChord/chtho.git
$ cd chtho
$ make
```

* Run the simple echo server and client examples:

```
$ ./build/bin/echoserver_test 5 # thread pool size (i.e. # of sub Reactors)
$ # inside another terminal 
$ ./build/bin/echoclient_test 127.0.0.1 3 # server ip and number of clients 
$ # then the message of 'hello' and 'world' will bump between 
$ # client and server infinitely 
```


## Modules

* time
  * `Timestamp`: Provide basic utilities to provide current time and its conversions.
  * `TimeZone`: Convert from Coordinated Universal Time to local time.

* logging
  * `Logger` and `LogStream`: front end of the logging system, used directly by the user.
  * `AsyncLogging`, `LogFile` and `FileUtil`: back end of the logging system, asynchronously flush the data to disk and conditionally roll the log.

* threading
  * `MutexLock`, `MutexLockGuard`, `Condition` and `CountDownLatch`: encapsulated synchronization utilities.
  * `Thread` and `ThreadPool`: thread utilities based on pthread

* timers
  * `Timer` and `TimerID`: record timestamp and callback function.
  * `TimerQueue`: add/remove timers and find expired timers using balanced binary tree.

* networking
  * `Channel`: important abstraction provided for file descriptors and its relating events and corresponding callback functions.
  * `EventLoop`: core class that demonstrates the Reactor pattern.
  * `EventLoopThread`: encapsulates the eventloop in a thread, ensures 'one loop per thread'
  * `EventLoopThreadPool`: starts a main eventloop thread acting as the main Reactor (usually used to monitor the listening socket) and a bunch of other eventloop 
 thread acting as the sub Reactors (usually used to monitor read/write events happened on the connecting sockets).
  * poller:
    `Poller`: base class providing the interface of polling, uses `Channel` to manage the events and callback functions of file descritpors.
    `Poll`: An implementation of `Poller` using `poll(2)`.
    `EPoll`: An implementation of `Poller` using `epoll(2)`, level-triggered.
  * `Buffer`: used by `TcpConnection` to allow partial read/write.
  * `TcpConnection`: manages read/write/close/error events happened on the connecting file descriptors, uses `Buffer` to read/write data.
  * `Acceptor`: created in `TcpServer`, encapsulates the `socket`, `bind`, `listen` and `accept` steps. Inside `Acceptor::listen`, it will pass the connection socket file descriptor returned by `accept` to the new connection callback function provided by `TcpServer`. `TcpServer` finds a thread from thread pool for this new connection fd and creates a `TcpConnection` object. `TcpConnection` will register the connection channel (created upon connection fd) with the poller inside the eventloop which is dispatched eariler inside `TcpServer` and then starts to handle events happened on the connection channel (through the registed callback functions on the channel).
  * `TcpServer`: encapsulates a `EventLoopThreadPool` and `Acceptor`. `Acceptor` will handle `socket`, `bind`, `listen` and `accept` steps. `TcpServer`'s main role is dispatch the new connections to threads inside eventloop thread pool. It also provides the connection callback and message callback interface to the user.
  * `Connector`: works for `TcpClient`, encapsulates the `socket` and `connect` steps. Depending on the return value of `::connect`, it will use a channel to detect whether the connection socket is available for writing. If it is, this channel for the socket is removed and the socket file descriptor is passed to new conection callback function provided by `TcpClient`. `TcpClient` will use the socket file descriptor to create a new `TcpConnection`. Then the `TcpConnection` will handle the reading/writing event on the connection file descriptor. (the whole process is a little similar to `Acceptor`)
  * `TcpClient`: encapsulates a `EventLoop` and `Connector`. Similar to `TcpServer`, it creates a new `TcpConnection` using the file descriptor provided by the `Connection`. It also provides the connection callback and message callback interface to the user. 
  * `InetAddr`: encapsulates Internet address.
  * `Socket`: encapsulates socket related information.

development history: logger/logstream -> threading -> time/timer -> eventloop/thread/threadpool/poller/channel -> buffer/tcpconnection/acceptor/tcpserver -> connector/tcpclient -> logfile/asynclogging -> ??? 

## Documentation

* [logging](./docs/logging.md)
* [timer](./docs/TimerQueue.md)
* [channel](./docs/Channel.md)
* [eventloop](./docs/EventLoop.md)
* [poller](./docs/Poller.md)
* [RAII](./docs/RAII.md)
* [smartptr](./docs/smartpointers.md)

## License

MIT License 

## Acknowledgement

This project learns _massively_ from [muduo](https://github.com/chenshuo/muduo). 