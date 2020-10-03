## Publish/Subscribe Tutorial

The following tutorial will cover how to do publish/subscribe messaging using
the EIS Message Bus. The outcome of this tutorial will be two Python scripts,
one of which will publish messages and the other which will subscribe to
messages.

To start, we will create a script named, `publisher.py`. To start, we will
add a few necessary imports which will be utilized later in the tutorial.

```python
import os
import time
import json
import argparse
import eis.msgbus as mb
```

The last line of the imports above is where we import the EIS Message Bus
Python binding, which for sake of clarity will be referenced as `mb` in all
of the code to follow.

Next, we will add some more boilerplate code for parsing command line
arguments.

```python
# Argument parsing
ap = argparse.ArgumentParser()
ap.add_argument('config', help='JSON configuration')
ap.add_argument('-t', '--topic', default='publish_test', help='Topic')
args = ap.parse_args()
```

The code above initializes an `argparse.ArgumentParser()` object to parse
the incoming command line arguments. The arguments for the publisher will be
one positional argument, `config`, which will be the pointer to a JSON
file configuration for the message bus, and one optional argument, `--topic`,
which will give us the topic string under which to publish messages.

It is important to note at this point that a JSON configuration is going to be
used for the publisher, however, this is not required of an application using
the Python binding. The EIS Message Bus Python API requires that a Python
`dict` object be used to configure the bus. For ease of use, JSON has been
used in this example.

Next we will load our configuration Python dictionary from the given JSON file.

```python
with open(args.config, 'r') as f:
    config = json.load(f)
```

Next we will initialize our message bus context object.

```python
print('[INFO] Initializing message bus context')
msgbus = mb.MsgbusContext(config)
```

The message bus context object is the object that will be used for initializing
all of the publisher, subscribers, services, and service requesters in your
application. However, what you are able to initialize via the context is
dependent on the configuration used during initialization.

Now, lets initailize our publisher.

```python
print(f'[INFO] Initializing publisher for topic \'{args.topic}\'')
publisher = msgbus.new_publisher(args.topic)
```

The message bus context object contains a method called, `new_publisher()`.
This method creates a new publisher object for the topic it is given. Once
this publisher object is obtained, the application can start publishing
messages (as shown below).

```python
print('[INFO] Running...')
while True:
    publisher.publish(({'hello': 123}, b'HELLO',))
    time.sleep(1)
```

The `publisher.publish()` method called above publishes the given values on the
topic given during the `new_publisher()` call. The `publish()` method can be
called one of three ways:

1. `publisher.publish({'hello': 123'})` - Publishes a JSON blob for the dictionary
2. `publisher.publish(b'HELLO')` - Publishes the arbitrary binary blob it was given
3. `publisher.publish(({'hello': 123}, b'HELLO',))` - Publishes the dictionary and blob together

The publisher object has a `close()` method which MUST be called on it prior to
the exit of your application. If this method is not called, then you application
may hang indefinitley. In order to make sure this happens, we'll wrap all of the
code in a `try...finally...` block in Python. Note that this is just the solution
for this simple application. Other applications may need to get more creative
in making sure the `close()` method is called.

When its all put together, the code looks as follows:

```python
import time
import json
import argparse
import eis.msgbus as mb

# Argument parsing
ap = argparse.ArgumentParser()
ap.add_argument('config', help='JSON configuration')
ap.add_argument('-t', '--topic', default='publish_test', help='Topic')
args = ap.parse_args()

with open(args.config, 'r') as f:
    config = json.load(f)

msgbus = None
publisher = None

try:
    print('[INFO] Initializing message bus context')
    msgbus = mb.MsgbusContext(config)

    print(f'[INFO] Initializing publisher for topic \'{args.topic}\'')
    publisher = msgbus.new_publisher(args.topic)

    print('[INFO] Running...')
    while True:
        publisher.publish(({'hello': 123}, b'HELLO',))
        time.sleep(1)
except KeyboardInterrupt:
    print('[INFO] Quitting...')
finally:
    if publisher is not None:
        publisher.close()
```

> **NOTE:** This code is provided in `python/examples/publisher.py` in the EIS
> Message Bus source code.

Now that the publisher script is finished, we will create the subscriber script
called `subscriber.py`. The script will essentially be the same as the publiser
script, except that instead of calling `new_publisher()` we will call
`new_subscriber()`.

```python
import time
import json
import argparse
import eis.msgbus as mb

# Argument parsing
ap = argparse.ArgumentParser()
ap.add_argument('config', help='JSON configuration')
ap.add_argument('-t', '--topic', default='publish_test', help='Topic')
args = ap.parse_args()

msgbus = None
subscriber = None

with open(args.config, 'r') as f:
    config = json.load(f)

try:
    print('[INFO] Initializing message bus context')
    msgbus = mb.MsgbusContext(config)

    print(f'[INFO] Initializing subscriber for topic \'{args.topic}\'')
    subscriber = msgbus.new_subscriber(args.topic)

    print('[INFO] Running...')
    while True:
        msg = subscriber.recv()
        if msg is not None:
            print(f'[INFO] RECEIVED: {msg}')
        else:
            print('[INFO] Receive interrupted')
except KeyboardInterrupt:
    print('[INFO] Quitting...')
finally:
    if subscriber is not None:
        subscriber.close()
```

> **NOTE:** This code is provided in `python/examples/subscriber.py` in the EIS
> Message Bus source code.

As can be seen in the code above, the `msgbus.new_publisher()` call has now
been replace with: `msgbus.new_subscriber()`. This call connects to the given
topic to receive publications. Depending on the underlying protocol that the
message bus is configured to use different values will be retrieved from the
configuration to accomplish the creation of the subscriber.

For instance, if you are using the ZeroMQ TCP protocol, then it will be
required that the configuration contains a key for the topic given to the
`msgbus.new_subscriber()` function call. For the purposes of this tutorial,
if the topic is the default topic (`publish_test`), then the configuration
must contain a JSON object similar to the following:

```json
{
    "publish_test": {
        "host": "127.0.0.1",
        "port": 3000
        }
    }
}
```

The other major difference between the `publisher.py` and `subscriber.py`
scripts is in the while loop at the end. In the `subscriber.py` script you'll
see that there is a call to `subscriber.recv()`. This method waits to receive
an incoming publication on the subscriber's topic. The `recv()` method has
three ways of being called.

1. `recv()`/`recv(-1)` - Blocks indefinitley until a new message is received.
2. `recv(0)` - Returns immediates if not message has been received.
3. `recv(timeout)` - Waits for the specified timeout to receive a message. `timeout`
    is assumed to be an integer that represents the time to wait in milliseconds.

To put this all together we need to first define the configuration we are going
to load for the publisher and subscriber. We will use the configuration below
saved to the file `ipc_config.json`.

```json
{
    "type": "zmq_ipc",
    "socket_dir": "/tmp"
}
```

Using this configuration will configure the message bus to use the ZeroMQ IPC
protocol and to save the Unix socket files into the `/tmp` directory on your
system.

Once this configuration is saved to the file `ipc_config.json`, start the
publisher in one Terminal window with the following command:

```sh
$ python3 publisher.py ipc_config.json
```

Next, start the subscriber in a different Terminal window with the command
shown below.

```sh
$ python3 subscriber.py ipc_config.json
```

At this point you should see the messages being published by the publisher
being printed to the console by the subscriber.
