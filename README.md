[![Build Status](https://travis-ci.org/finaldie/skull.svg?branch=master)](https://travis-ci.org/finaldie/skull)

skull
=====

Fast to start-up, easy to maintain, high productivity serving framework.

## How to Build
Use Ubuntu14.04 as an example.

### Install dependencies
```console
apt-get install valgrind libyaml-dev python-yaml libprotobuf-c0 libprotobuf-c0-dev protobuf-c-compiler

git clone git@github.com:finaldie/skull.git
cd skull
git submodule update --init --recursive
make dep
```

### Build
```console
make -j4
```

### Install Skull and its related Scripts
```console
make install
```

## A Quick Demo
After you installed skull into your system, you can run the following steps to
create your skull project.

### Create a skull project
```shell
skull create project
cd project
skull workflow -add # then input $concurrent, $idl_name, $port
skull module -add # then input $module_name, $workflow_index and $language
skull build
skull deploy
skull start
```

**notes:** By default, the new module with a example code which is used for echo-back message

### Play with skull
```console
telnet localhost 7758
Trying ::1...
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
hello skull
hello skull
have fun :)
have fun :)

```

## How to Create a Service
The following is the example of adding a service **s1**, and then add a api **get** for it.
```console
bash $> skull service -add
service name? s1
which language the service belongs to? (c) c
data mode? (rw-pr | rw-pw) rw-pr
notice: the common/c folder has already exist, ignore it
service [s1] added successfully
bash $> cd src/services/s1
bash $> skull service --api-add
service api name: get
service api access_mode: (read|write) read
s1-get_req added
s1-get_resp added
service api get added successfully
```
After that, you can use service apis which defined in `skull/service.h` from a module to communicate a service :)

## Share your Service and Import from Others
Service is designed for sharing, each service is built for one single purpose or solve specific problem, share to others which help people to build the project easier than ever.


## How to Check Metrics
Currently, the [AdminModule][1] is a builtin module in skull, just connect to port `7759`, then you will see them.
```console
final@ubuntu: ~>telnet 0 7759
Trying 0.0.0.0...
Connected to 0.
Escape character is '^]'.
help
commands help:
 - help
 - show
show
2015:12:25_22:58:40 to 2015:12:25_22:59:03
skull.core.g.global.response: 2.000000
skull.core.g.global.timer_complete: 22.000000
skull.core.g.global.connection_create: 1.000000
skull.core.g.global.entity_create: 26.000000
skull.core.g.global.latency: 780.000000
skull.core.g.global.entity_destroy: 23.000000
skull.core.g.global.uptime: 22.000000
skull.core.g.global.timer_emit: 24.000000
skull.core.g.global.request: 3.000000
skull.core.t.worker.timer_complete.master0: 22.000000
skull.core.t.worker.entity_create.master0: 26.000000
skull.core.t.worker.entity_destroy.master0: 22.000000
skull.core.t.worker.timer_emit.master0: 24.000000
skull.core.t.worker.latency.worker0: 780.000000
skull.core.t.worker.accept.worker0: 1.000000
skull.core.t.worker.entity_destroy.worker0: 1.000000
skull.core.t.worker.response.worker0: 2.000000
skull.core.t.worker.request.worker0: 3.000000
skull.user.s.module.response: 1.000000
skull.user.s.module.request: 1.000000
```

## Frequent Questions

[1]: https://github.com/finaldie/skull-admin-c
