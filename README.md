# A simple web server based on Muduo
## Description
The server is bulit with reactor and one loop per thread model. One I/O thread for listening incoming connections and updating timer wheel, numbers of worker threads for processing connection requests with their own epoll(LT). The server could receive GET request for static web page information (text and a picture) with http1.0/1.1. Meanwhile, a timer wheel is used to handle timeout long connections.
## Environment
- Ubuntu 20.04
- g++7.5 & CMake
- C++17
## Test Result
Run the server with 3 worker threads and enable timer wheel:
```
./build/bin/web_server 3 1
```
Run `ab` to test the performance:
```
ab -n 10000000 -k -r -c 12000 192.168.3.148:8000/
```
And the server could run normally under 12000 cocurrency
```
This is ApacheBench, Version 2.3 <$Revision: 1807734 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 192.168.3.148 (be patient)
Completed 1000000 requests
Completed 2000000 requests
Completed 3000000 requests
Completed 4000000 requests
Completed 5000000 requests
Completed 6000000 requests
Completed 7000000 requests
Completed 8000000 requests
Completed 9000000 requests
Completed 10000000 requests
Finished 10000000 requests


Server Software:        Naive
Server Hostname:        192.168.3.148
Server Port:            8000

Document Path:          /
Document Length:        200 bytes

Concurrency Level:      12000
Time taken for tests:   85.628 seconds
Complete requests:      10000000
Failed requests:        0
Keep-Alive requests:    10000000
Total transferred:      3100000000 bytes
HTML transferred:       2000000000 bytes
Requests per second:    116783.59 [#/sec] (mean)
Time per request:       102.754 [ms] (mean)
Time per request:       0.009 [ms] (mean, across all concurrent requests)
Transfer rate:          35354.41 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   6.4      0    1200
Processing:    39  102   3.5    102     140
Waiting:       39  102   3.5    102     140
Total:         39  103   7.9    102    1307

Percentage of the requests served within a certain time (ms)
  50%    102
  66%    102
  75%    103
  80%    103
  90%    103
  95%    106
  98%    115
  99%    120
 100%   1307 (longest request)
```
![image](./web_server/test_result/test.png)