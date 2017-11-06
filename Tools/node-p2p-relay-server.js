var server = require("dgram").createSocket("udp4");
var active_relays = [];

server.on("message", function (msg, rinfo)
{
	if (msg.length < 1) return;
	var command = msg.readUInt8(0);
	//console.log("Command: " + command + " - From " + rinfo.address + ":" + rinfo.port);
	if (command == 100 && msg.length >= 3) //command 100 = new connection
	{
		var open_connections = msg.readUInt8(1) | (msg.readUInt8(2) << 8);
		var key = msg.slice(3).toString('binary');
		var active_relay = active_relays[key];
		if (open_connections <= 0)
		{
			console.log("Invalid connection command - Key: " + key + " - From " + rinfo.address + ":" + rinfo.port + " - open_connections: " + open_connections);
		}
		else if (active_relay === undefined)
		{
			active_relays[key] = { host: rinfo, connections_available: open_connections };
			console.log("New relay host - Key: " + key + " - From " + rinfo.address + ":" + rinfo.port + " - connections_available: " + open_connections);

			var response = new Buffer(1 + msg.length - 3);
			response.writeUInt8(200, 0); //command 200 = become host
			msg.copy(response,       1, 3);
			server.send(response, 0, response.length, rinfo.port, rinfo.address, function(err, bytes)
				{ console.log("   SENT: CommandBecomeHost to " + rinfo.address + ":" + rinfo.port + " (ERR: " + err + ")"); });
		}
		else
		{
			var connections_available = --active_relays[key].connections_available;
			console.log("Got relay client - Key: " + key + " - From " + rinfo.address + ":" + rinfo.port + " - connections_available: " + connections_available);

			var host = active_relay.host, ip = host.address.split(".");
			var response = new Buffer(7 + msg.length - 3);
			response.writeUInt8(201, 0); //command 201 = become guest
			response.writeUInt8(ip[0]*1,        1);
			response.writeUInt8(ip[1]*1,        2);
			response.writeUInt8(ip[2]*1,        3);
			response.writeUInt8(ip[3]*1,        4);
			response.writeUInt8(host.port&0xFF, 5);
			response.writeUInt8(host.port>>8,   6);
			msg.copy(response,                  7, 3);
			server.send(response, 0, response.length, rinfo.port, rinfo.address, function(err, bytes)
				{ console.log("   SENT: CommandBecomeGuest to " + rinfo.address + ":" + rinfo.port + " (ERR: " + err + ")"); });

			if (connections_available == 0)
			{
				console.log("   Session filled up");
				delete active_relays[key];
			}
		}
	}
	else if (command == 101) //command 101 = remove active relay
	{
		var key = msg.slice(1).toString('binary');
		var active_relay = active_relays[key];
		if (active_relay !== undefined && active_relay.host.address  == rinfo.address && active_relay.host.port  == rinfo.port)
		{
			console.log("Remove relay host: Key: " + key + " - From " + rinfo.address + ":" + rinfo.port);
			delete active_relays[key];
		}
		else
		{
			console.log("Invalid remove relay command - Key: " + key + " - From " + rinfo.address + ":" + rinfo.port + " - open_connections: " + open_connections);
		}
	}
});

server.on("error", function (err)
{
	console.log("server error:\n" + err.stack);
	server.close();
});

server.on("listening", function ()
{
	var address = server.address();
	console.log("server listening " + address.address + ":" + address.port);
});

server.bind(41234);
