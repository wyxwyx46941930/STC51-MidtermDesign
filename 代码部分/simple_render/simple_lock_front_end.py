import time
from http.server import BaseHTTPRequestHandler, HTTPServer

import logging
import serial
import threading

HOST_NAME = 'localhost'
PORT_NUMBER = 9000

# STATE STRING 
STATE_PASSWORD_RETTING_SUCCESSFULLY = "密码重置成功"
STATE_PASSWORD_VALID = "密码正确，大门打开"
STATE_PASSWORD_INVALID = "密码错误"
STATE_WAITING_INPUT = '等待大门密码输入。。。'


# setup baudrate and other basic serial port infomation
BAUDRATE = 9600
TIMEOUT = 0.5
PORT = 'COM3'
SERIAL = serial.Serial(PORT, BAUDRATE, timeout=TIMEOUT)

# share variable, will be altered by the receiver thread.
state = STATE_WAITING_INPUT

class MyHandler(BaseHTTPRequestHandler):
    def do_HEAD(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

    def do_GET(self):
        if(self.path == "/"):
            try:
                content = open("./index.html", "r").read()
                response = self.handle_http(200, content)
            except FileNotFoundError:
                content = "file not found"
            self.wfile.write(response)
        elif(self.path == "/poll"):
            # write password file            
            self.wfile.write(self.handle_http(200, state))
        elif(self.path == "/bulma.css"):
            # write password file
            content = open("./bulma.css", "r").read()        
            self.wfile.write(self.handle_http(200, content, "text/css"))

    def do_POST(self):
        # write date to from
        # return data input
        content_len = int(self.headers.get('content-length', 0))
        new_password = self.rfile.read(content_len)
        new_password = bytearray(new_password)[-8:]
        # enter password restting mode
        SERIAL.write(b'B')
        print("post")
        assert len(new_password) == 8
        SERIAL.write(bytes(map(lambda x : x + 19, new_password)))
        # print("post")
        # self.wfile.write(self.handle_http(200, resetting_state))
        
    # return a page with administration
    def handle_http(self, status_code, content, type = 'text/html'):
        self.send_response(status_code)
        self.send_header('Content-type', type)
        self.end_headers()
        return bytes(content, 'UTF-8')

# running a backend thread to read from
def receiver(ser):
    # setup logger
    logger = logging.getLogger('simple_lock')
    logger.setLevel(logging.INFO)
    fh = logging.FileHandler('record.log', encoding='utf-8')
    fh.setLevel(logging.INFO)
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    fh.setFormatter(formatter)
    logger.addHandler(fh)

    buffer = ''
    global state
    # state = STATE_PASSWORD_INVALID
    while 1:
        s = ser.read(1)
        if s:
            c = s.decode('utf-8')
            # Notice: when restarting SCM, the first char is char(0).
            # In Windows, if the first char of string is char(0),
            #   the string won't be printed.
            if ord(c) != 0:
                # print(c)
                buffer += c
        else:
            if buffer:
                print("getting feed back from outer part of lock")
                print(buffer)
                if buffer == 'r':
                    state = STATE_PASSWORD_RETTING_SUCCESSFULLY                     
                elif buffer == 'f':
                    state = STATE_PASSWORD_INVALID
                elif buffer == 't':
                    state = STATE_PASSWORD_VALID
                logger.info(state)
                time.sleep(5)
                state = STATE_WAITING_INPUT
            buffer = ''

# 如果收到信息，则打印
thread_recv = threading.Thread(target=receiver, args=(SERIAL,))
thread_recv.setDaemon(True)
thread_recv.start()

if __name__ == '__main__':
    server_class = HTTPServer
    httpd = server_class((HOST_NAME, PORT_NUMBER), MyHandler)
    print(time.asctime(), 'Server Starts - %s:%s' % (HOST_NAME, PORT_NUMBER))
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()
    print(time.asctime(), 'Server Stops - %s:%s' % (HOST_NAME, PORT_NUMBER))