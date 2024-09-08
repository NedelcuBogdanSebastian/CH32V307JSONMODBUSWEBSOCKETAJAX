# CH32V307JSONMODBUSWEBSOCKETAJAX

    This implementation has multiple methods over ethernet, for sending and receiving data.

    It implements WEBSOCKETS, JSON, AJAX, and MODBUS TCP.

    (The AJAX is only LIGHT IMPLEMENTED, only sends an answer packet that obeys CORS rules, NO DATA INTERPRETARION :D )

    To test you have 2 methods, by using the Python scripts or by webpages and RMMS
    utility.

    1. Using the Python scripts, you need python-3.8.10-amd64 and install the following
       libraries:
           * for      get_JSON.py script -> pip install requests         (json and time libraries are part of the Python standard library)

           * for get_WEBSOCKET.py script -> pip install websocket-client (time and threading are part of Python's standard library)

           * for get_MODBUSTCP.py script -> pip install pymodbus         (binascii, struct, pickle, os, time, socket, subprocess, and threading
                                                                          are part of Python's standard library)

    2. Using RMMS and webpages clients:
           * use RMMS as Modbus TCP client, to connect to 192.168.1.10, port 502

           * use the WEBSOCKETClient.html to test data transfer using websockets. After connection, the JS client from the
             web page sends a string once at one second. Also you can send a data string with CRC16 at the end,
             and the CH32V307 server will send it back and the web page will check the data CRC16.
             Use one of the following to compute CRC-16 (Modbus) variant:
                 - https://www.lammertbies.nl/comm/info/crc-calculation
		         - https://www.tahapaksu.com/crc/
		     Attention! For example when inserting the A32AF54FBBC677CDFEEA, the CRC16 (Modbus) result is: 0xA70
		                you need to append a 4 byte CRC16 => 0A70, not only the 0xA70!
		                So the resulted string is A32AF54FBBC677CDFEEA0A70.
		                If you send this data in the send field, and hit the SEND button, the message will go to server,
		                the server will send back the message, and the JS websocket client will check the CRC16 and the
		                CRC Status will become => CRC Status: CRC-OK

		   * to test the JSON you only need to call 192.168.1.10/json.html in Chrome, Firefox or Midori

		   * to test the AJAX you use the AJAXClient.html and there you can send some data and see the received string from the server


     The AJAXServer.py and the app.py are some extra work. Fell free to test! :)
