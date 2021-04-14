<!--
 Copyright (c) 2021 Qizhou Guo
 
 This software is released under the MIT License.
 https://opensource.org/licenses/MIT
-->

## Front End

* Core class: `Logger` and `LogStream`
* Usage:
  * `LOG_LEVEL << message;`, `LEVEL` can be `ERR`, `TRACE`, `DEBUG`...

Take `LOG_INFO` as an example, it is actually a macro. The macro will 
create a temporary `Logger` object and call its `stream()` member function.
In the ctor of `Logger`, it will get current file name, line, pid, tid and 
timestamp. This will be formed as a string which be put at the beginning of 
the log. Because the filename where each `Logger` object is created is known 
at compile time, so the `SourceFile` use templates to get the length of the 
filename at compile time. The message will be passed to `LogStream::operator<<`,
which is overloaded on many different types. Here I tried to use some SFINAE 
to enable template overloading only on some certain cases. All the message 
put into stream is placed in buffer_ inside `LogStream`. So when do these 
logs are printed out or saved file? The answer is the dtor. Because the 
`Logger` object created by the macro is a temporary object. So immediately 
after processing all the stream, it will call its dtor. Inside dtor, it will 
call the output function on the buffer_ of `LogStream`. The output function 
can be customized, when its implementation is to write the buffer to file, 
then the log is written to file. The default output function is writing to 
stdout. The writes may be buffered by the operating system, so when you want 
to see some urgent imformation, you will want to flush the data directly. 
For `Level::FATAL`, the flush function is explicitly called. The flush function 
can also be customized. The default flush function is `fflush`.


## Back End

* Core class: `AsyncLogging` and `LogFile`
* Usage: 
  * Create a new `AsyncLogging` global object `glog`
  * Define function `output` to call `glog.append` 
  * Set the output function of logging front end using `Logger::setOutput`

As we have seen in front end, the `output` function will be called in the 
dtor of `Logger` object. Then `AsyncLogging::append` gets called. Inside 
`AsyncLogging::append`, if the current buffer's available space is sufficient 
to hold the string message, append the message to current buffer. Otherwise 
the current buffer will be pushed into the buffer list waiting to be flushed 
to disk and the next buffer pointer is swapped as current buffer. (If the next 
buffer pointer is nullptr, this means both of them have been full. In this 
case, we will allocate a new buffer. However, this is rare.) This is called 
as the 'double buffering' technique. Its advantage is that the front end only 
has to put the data into the buffer and doesn't need to wait for the disk IO 
operation. It also avoids waking up the back end logging thread each time a 
log is generated. 

As for the back end logging thread, another two buffers are created. Inside 
the critical section, the condition variable will wait until timeout or 
bufs_ is not empty (which means there are buffers that can be written to 
disk). Then immediately the current buffer pointer is pushed into bufs_, 
and the newly create buffer pointer 1 to moved to current buffer pointer.
bufs_ is swaped with bufs so that the writing process only have to depend 
on bufs and bufs_ is available for use in `AsyncLogging::append`. This 
reduces the size of critical section. Then the new buffer pointer 2 is also 
moved to next buffer pointer (is it is not nullptr). Out of the critical 
section, bufs are written to file through `LogFile::append` (which will 
flush the file every 3 secs and roll the file if the file is larger than 
the roll size or it reaches the next period). Finally, bufs will only 
reserve 2 buffers as before and the two new buffers will regain their 
pointers from bufs (these has been written). As we can see, we actually 
use 4 buffers instead of 2. 