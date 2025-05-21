# kasalua
kasalua provides high level lua binding to the Kasa interface library with minimal overhead

# Build
```
git clone https://github.com/TheRootED24/kasalua.git
cd kasalua
cmake .
make
```

# Usage
```
Lua 5.1.5  Copyright (C) 1994-2012 Lua.org, PUC-Rio (double int32)
> kasa = require "kasa"

-- load the avaiable commands
> cmds = require "kasa_commands"

-- init kasa (initializes semaphore)
> kasa:init()

-- targeting a single device, to do a full scan of all devices use .255 ie kasa:scan("10.0.10.255", 1000, 0) the 1000 represents the timeout in Ms and third parameter (is_query) is redundant, always use 0
> devs = kasa:scan("10.0.10.174", 1000, 0)
BROADCASTING ON IP: 10.0.10.174
1	table	0x557382468010
2	number	1              --<--- found 1 device
3	table	0x5573824682d0
1	table	0x557382468010
1	table	0x557382468010

> print(#devs)
1
> print(devs[1].ip)

10.0.10.174

> print(devs[1].sysinfo)

{"system":{"get_sysinfo":{"sw_ver":"1.0.8 Build 230804 Rel.172306","hw_ver":"2.0","model":"KP303(US)","deviceId":"0000000000000000000000000000000000000000","oemId":"00000000000000000000000000000000","hwId":"00000000000000000000000000000000","rssi":-48,"latitude_i":-1679051393,"longitude_i":-1679051393,"alias":"TP-LINK_Power Strip_0A7D","status":"new","obd_src":"tplink","mic_type":"IOT.SMARTPLUGSWITCH","feature":"TIM","mac":"00:00:00:00:00:00","updating":0,"led_off":0,"children":[{"id":"00","state":1,"alias":"EXHAUST FAN","on_time":26786,"next_action":{"type":1,"schd_sec":82800,"action":0}},{"id":"01","state":1,"alias":"Light1","on_time":33986,"next_action":{"type":1,"schd_sec":82800,"action":0}},{"id":"02","state":1,"alias":"HUMIDIFIER","on_time":26787,"next_action":{"type":1,"schd_sec":82800,"action":0}}],"child_num":3,"ntc_state":0,"err_code":0}}}

-- send a cmd to the device, we'll just re-query the sysinfo for the demo
resp = kasa:send(devs[1].ip, cmds.system_sysinfo)
> print(resp)

{"system":{"get_sysinfo":{"sw_ver":"1.0.8 Build 230804 Rel.172306","hw_ver":"2.0","model":"KP303(US)","deviceId":"0000000000000000000000000000000000000000","oemId":"00000000000000000000000000000000","hwId":"00000000000000000000000000000000","rssi":-53,"latitude_i":-1679051393,"longitude_i":-1679051393,"alias":"TP-LINK_Power Strip_0A7D","status":"new","obd_src":"tplink","mic_type":"IOT.SMARTPLUGSWITCH","feature":"TIM","mac":"00:00:00:00:00:00","updating":0,"led_off":0,"children":[{"id":"00","state":1,"alias":"EXHAUST FAN","on_time":26786,"next_action":{"type":1,"schd_sec":82800,"action":0}},{"id":"01","state":1,"alias":"Light1","on_time":33986,"next_action":{"type":1,"schd_sec":82800,"action":0}},{"id":"02","state":1,"alias":"HUMIDIFIER","on_time":26787,"next_action":{"type":1,"schd_sec":82800,"action":0}}],"child_num":3,"ntc_state":0,"err_code":0}}}
```

See kasa_commands.lua for a for list of available cmds.

# Supported Models
all bulbs
all light strips
all plugs
all strips
