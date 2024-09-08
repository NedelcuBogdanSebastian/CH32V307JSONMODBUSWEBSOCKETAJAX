'''
Thread-Safe Variable Access: Used a threading.Lock (cnt_lock) to ensure that access to cnt is thread-safe.
This prevents race conditions where multiple threads might simultaneously read and write to cnt.

Exception Handling in Threads: Added a try-except block within the run function to catch and report any
exceptions that occur during message sending. This helps in diagnosing issues without crashing the program.

Graceful Exit: Added a finally block to ensure the WebSocket is properly closed even if an interrupt occurs.
'''

import websocket
import time
import threading

cnt = 0
cnt_lock = threading.Lock()  # Lock to ensure thread-safe access to `cnt`

def on_message(ws, message):
    print(f"Received message: {message}")

def on_error(ws, error):
    print(f"Encountered error: {error}")

def on_close(ws, close_status_code, close_msg):
    print("Connection closed")

def on_open(ws):
    print("Connection opened")
    def run():
        global cnt
        while True:
            print('================== WEBSOCKET REQUEST ===================')
            try:
                with cnt_lock:
                    message = f"AAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFFAAFF CLIENT MESSAGE {cnt}"
                    ws.send(message)
                    cnt += 1
                time.sleep(0.1)
            except Exception as e:
                print(f"Error in thread: {e}")
                break  # Exit loop if an exception occurs
    # Start the daemon thread for sending messages
    message_thread = threading.Thread(target=run, daemon=True)
    message_thread.start()

if __name__ == "__main__":
    ws = websocket.WebSocketApp("ws://192.168.1.10:8088/echo",
    #ws = websocket.WebSocketApp("ws://localhost:8088/echo",
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)
    ws.on_open = on_open
    try:
        ws.run_forever()
    except KeyboardInterrupt:
        print("Interrupted by user")
    finally:
        ws.close()
        print("WebSocket closed")
