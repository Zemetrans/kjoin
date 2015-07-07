# kjoin

## github SSH getting started
https://help.github.com/articles/generating-ssh-keys/

## cloning the repository
git clone --recursive git@github.com:jenisov/kjoin.git ~/kjoin

## AllJoin Tiny Core
https://allseenalliance.org/developers/learn/core/thin-core

## building and testing samples

Assume we are using Ubuntu LTS 14.04 x86 64bit

### install necessary packages

`$> sudo apt-get install build-essential libgtk2.0-dev libssl-dev xsltproc libxml2-dev libcap-dev`

`$> sudo apt-get install python scons`

### build tiny core library and samples

```
$> cd ~/kjoin/ajtcl

$> scons WS=off
```

binaries for samples are located at `~/kjoin/ajtcl/samples/basic`


### build alljoin standard core library and samples

```
$> cd ~/kjoin/alljoin

$> scons BINDINGS=cpp WS=off BT=off ICE=off SERVICES="about,notification,controlpanel,config,onboarding,sample_apps"
```

### run alljoin service

```
$> export LD_LIBRARY_PATH=~/kjoin/alljoyn/build/linux/x86_64/debug/dist/cpp/lib

$> kjoin/alljoyn/build/linux/x86_64/debug/dist/cpp/bin/alljoyn-daemon
AllJoyn Message Bus Daemon version: v0.00.01
Copyright AllSeen Alliance.

Build: AllJoyn Library v0.00.01 (Built Tue Jul 07 17:59:57 UTC 2015 by dea - Git: alljoyn.git branch: 'master' tag: 'v15.04x-rc2' (+199 changes) commit ref: 578d67de81fb67c3924386acc333cee8d7bb558b)
Setting up transport for address: tcp:iface=*,port=9955
Setting up transport for address: udp:iface=*,port=9955
Setting up transport for address: unix:abstract=alljoyn
```

### in another terminal run tiny core basic sevice

```
$> ~/kjoin/ajtcl/samples/basic/basic_service 
000.000 aj_target_nvram.c:86 _AJ_LoadNVFromFile(): LoadNVFromFile() failed. status=AJ_ERR_FAILURE
Reminder: Object not yet added to the ObjectList, do not forget to call RegisterObjects
<node name="/sample">
<interface name="org.alljoyn.Bus.sample">
  <method name="Dummy">
    <arg name="foo" type="i" direction="in"/>
  </method>
  <method name="cat">
    <arg name="inStr1" type="s" direction="in"/>
    <arg name="inStr2" type="s" direction="in"/>
    <arg name="outStr" type="s" direction="out"/>
  </method>
</interface>
</node>

```

### in third terminal run tiny core basic client

```
$ ~/kjoin/ajtcl/samples/basic/basic_client
Reminder: Object not yet added to the ObjectList, do not forget to call RegisterObjects
<node name="/sample">
<interface name="org.alljoyn.Bus.sample">
  <method name="Dummy">
    <arg name="foo" type="i" direction="in"/>
  </method>
  <method name="Dummy2">
    <arg name="fee" type="i" direction="in"/>
  </method>
  <method name="cat">
    <arg name="inStr1" type="s" direction="in"/>
    <arg name="inStr2" type="s" direction="in"/>
    <arg name="outStr" type="s" direction="out"/>
  </method>
</interface>
</node>
'org.alljoyn.Bus.sample.cat' (path='/sample') returned 'Hello World!'.
Basic client exiting with status 0.
```

