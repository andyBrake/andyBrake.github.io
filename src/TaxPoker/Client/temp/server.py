#!python

import threading,logging,time,random,datetime,socket

DATEFMT="%H:%M:%S"
FORMAT = "[%(asctime)s]\t [%(threadName)s,%(thread)d] %(message)s"
logging.basicConfig(level=logging.INFO,format=FORMAT,datefmt=DATEFMT)
 
class ChatServer:
    def __init__(self,ip='127.0.0.1',port=9999):
        self.addr = (ip,port)
        self.sock = socket.socket()
        self.event = threading.Event()
 
        self.clients = {}
 
    def show_client(self):
        while not self.event.is_set():
            if len(self.clients) > 0:
                logging.info(self.clients)
                self.event.wait(3)
 
 
    def start(self):
        self.sock.bind(self.addr)
        self.sock.listen()
        #start another thread
        threading.Thread(target=self._accept,name='accept',daemon=True).start()
        #threading.Thread(target=self.show_client,name='show_client',daemon=True).start()
 
 
    def stop(self):
        for c in self.clients.values():
            c.close()
        self.sock.close()
        self.event.wait(3)
        self.event.set()
 
    def _accept(self):
        while not self.event.is_set():
            conn,client = self.sock.accept() # block here
            f = conn.makefile(mode='rw')
            self.clients[client] = f
 
            logging.info("{}-{}".format(conn,client))
            print(client)

            # recv is block operation, each connect will start another thread
            threading.Thread(target=self._recv, args=(f, client), name='recv',daemon=True).start()
 
    # receive the client msg
    def _recv(self, f, client):
        while not self.event.is_set():
            try:
                data = f.readline()
                print("Receive msg : [%s]"%data)
            except Exception:
                data = 'quit'
            finally:
                if data is None or data == '':
                    data = 'quit'
                    print("Invalid Msg, should close now!!!!\n")
                    
                msg = data.strip()
                # client send msg to quit
                if msg == 'quit':
                    f.close()
                    self.clients.pop(client)
 
                    logging.info('{} quit'.format(client))
                    break
            
            #print("Receive msg : [%s]"%data)
            #msg = "{:%Y/%m/%d %H:%M:%S} {}:{}\n{}\n".format(datetime.datetime.now(),*client,data)
            #msg = data
            #print(msg)
            #logging.info(msg)
 
            #for c in self.clients.values():
                #print("All Client Value %s"%c)
                #c.writelines(msg)
                #c.flush()
 
 
 


def showthreads(e : threading.Event ):
    while not e.wait(3):
        logging.info(threading.enumerate())
 


if __name__ == '__main__': 
    cs = ChatServer()
    print('!!!!!!!!!!!')
    cs.start()
    print('~~~~~~~~~~~~~~~~~~~~')
    
    e = threading.Event()
    #threading.Thread(target=showthreads,name='showthreads',args=(e,)).start()
    
    while not e.wait(1):
        cmd = input('>>> ').strip()
        if cmd == 'quit':
            cs.stop()
            e.wait(3)
            break