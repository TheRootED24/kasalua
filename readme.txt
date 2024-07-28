compile: cd src && make

basic lua usage example:

#!/usr/bin/lua

--load the module
local kasa = require ("kasa");
--init the semaphore
kasa:init();

-- scan devices
local devs = kasa:scan(arg[1] or "192.168.1.255", arg[2] or 1000, arg[3] or 0);

-- print out the scan results
print(string.format("Found: %d Kasa Smart Devices", #devs));

  for i,_ in ipairs(devs) do
    print("\nDevice IP: "..devs[i].ip)
    print("\n********************** Device SysInfo **********************\n")  
    print(devs[i].sysinfo.."\n");
  end

  -- the device json can now be processed using java sript
  -- or parsed into a lua object using the lua "jsonmg" module as shown below

  --[[ jsonmg basic example]] --
  -- using the first device from the kasa scan (devs[1]) 

  jsonmg = require ("jsonmg");
  -- parse the first device found into a simple lua object
  local dev = jsonmg:parse(devs[1].sysinfo);
  -- trim down to sysinfo object
  dev.sysinfo = dev.system.get_sysinfo;
  -- add the ip
  dev.ip = devs[1].ip;
  -- print some properties
  print(string.format("IP: %s\n", dev.ip));
  print(string.format("Type: %s\n", dev.sysinfo.mic_type));
  print(string.format("Model: %s\n", dev.sysinfo.model));
  print(string.format("Alias: %s\n", dev.sysinfo.alias));
  print(string.format("Children: %d\n", dev.sysinfo.child_num));

  -- stringify the lua object into a valid json obj/arr
  local json = jsonmg:stringify(dev);
  -- print it 
  print(json);
