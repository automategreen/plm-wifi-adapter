WiFi Adapter for an Insteon PLM
===============
> A Spark Core WiFi adapter for an Insteon PLM
***

With the release of Insteon's newest Hub, Insteon removed the Hub's PLM interface.  This adapter adds secure access over WiFi to an Insteon PLM.

The adapter is a quick demo of what an Insteon compatible open hub could do.  We are currently working on an Open Source Hub with native support for the Insteon RF.

## Insteon Over the Spark Cloud

To improve on the standard Insteon Hub, we've used the awesome [Spark OS](https://www.spark.io/) to extend the PLM interface security to the cloud.  No unencrypted open information here. All messaging pass securely to and from the Spark OS.

### Insteon Message

All Insteon messages are sent as HEX strings.  The HEX strings allow for all Insteon messages to fit within the Spark OS packet restriction.  It also minimizes the logic for the adapter.

Because the HEX is cryptic and requires an in-depth knowledge of the Insteon HEX commands, we're adding support for the adapter to our Node.js module, [home-controller](https://github.com/automategreen/home-controller).

### Sending an Insteon Command

All the Insteon commands can be sent using HEX strings.

For example, to turn an Insteon switch on, you send a direct command to turn on the device.

```
$ spark call 0123456789ABCDEFGHI insteon 0262AABBCC0F117F
8
```

- Direct command: 0262
- Device ID: AABBCC
- Flags: 0F
- Action (turn on): 11
- Level (50%): 7F


The light turns on to 50%, and the call returns the number of bytes sent to the PLM. In this case `8`. 


### Receiving an Insteon Event

Now you can control your device, but you want more.  You want notifications in real time of from you devices.  No problem.  You can use the Spark OS subscribe to receive Insteon messages from your device.

Here's and example of getting notified of motion detected by a motion sensor.

```
$ spark subscribe insteon 0123456789ABCDEFGHI
Subscribing to "insteon" from 0123456789ABCDEFGHI's stream
Listening to: /v1/devices/0123456789ABCDEFGHI/events/insteon
{"name":"insteon","data":"0250aabbcc000001cf1101","ttl":"60","published_at":"2015-01-12T21:16:48.361Z","coreid":"0123456789ABCDEFGHI"}
```

The data field contains the Insteon message.

- Received command: 0250
- Device ID: AABBCC
- Group: 000001
- Flags: cf
- Action (motion detected): 11

### Insteon Request/Response

If you want to query a device for information, you combine the `call` and `subscribe` together. 

First, make sure you're subscribed.

```
spark subscribe insteon 0123456789ABCDEFGHI
Subscribing to "insteon" from 0123456789ABCDEFGHI's stream
Listening to: /v1/devices/0123456789ABCDEFGHI/events/insteon
```

Then, send the request. For example, get the light level.

```
$ spark call 0123456789ABCDEFGHI insteon 0262aabbcc0f1900
2
```

You'll then receive the Insteon response from the subscription.

```
{"name":"insteon","data":"0250aabbccffffff2f01ff","ttl":"60","published_at":"2015-01-12T21:16:48.361Z","coreid":"0123456789ABCDEFGHI"}
```

Light is on at 100%.  

If your want an easier way, you can use [home-controller](https://github.com/automategreen/home-controller).

## Native PLM Interface

The original Hub had an unsecured PLM interface, which allowed for the same communication on the LAN.  We can duplicate the interface on the Spark Core. By default, this interface is disabled (for security).  To enable it, place a simple Spark OS call.

```
spark call 55ff71066678505519401367 config plm=true
```

This call enables the PLM TCP interface (port 9761).

You can now use the adapter with legacy software solution (including our [home-controller](https://github.com/automategreen/home-controller) Node.js package).

## Feedback & Support

Please let us know if you like this concept. We're looking for feedback on creating an open Hub.  Send us an email, [support@automategreen.com](mailto:support@automategreen.com), or a tweet, [@AutomateGreen](https://twitter.com/AutomateGreen).
