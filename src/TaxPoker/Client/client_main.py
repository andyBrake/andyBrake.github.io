


#TCP Client客户端 封装成类
import socket,threading,logging,datetime
import Common
import ClientPlayer
from time import sleep


DATEFMT="%H:%M:%S"
FORMAT = "[%(asctime)s]\t [%(threadName)s,%(thread)d] %(message)s"
logging.basicConfig(level=logging.INFO,format=FORMAT,datefmt=DATEFMT)
 

class Client:
    def __init__(self,ip='127.0.0.1',port=10002):
        self.sock = socket.socket()
        self.addr = (ip,port)
        self.state = 0
        self.player = ClientPlayer.Player("Fa")
        self.event = threading.Event()
 
 
    def start(self):
        self.sock.connect(self.addr)
        self.state = 1 # already connect
        # 准备接收数据，recv是阻塞的，启动新的线程
        threading.Thread(target=self._recv,name='recv').start()
        sleep(1)
 
 
    def _recv(self):
        print("Start the Client Rcv Thread...")
        
        self._response2server()
        # then in the loop, the first reqeust should be the ID assign

        while not self.event.is_set():
            try:
                data = self.sock.recv(1024) #阻塞
            except Exception as e:
                logging.info(e) #有任何异常保证退出
                break

            if (len(data.strip()) > 0):
                #msg = "{:%H:%M:%S} IP:{} Msg:{}\n".format(datetime.datetime.now(),*self.addr, data.decode().strip())
                #print(msg)
                #logging.info("{}".format(data.decode()))
                print("\t\tRcv Server Msg:%s"%data)
                request = Common.Request(data.decode())
                self._process(request)
                self._response2server()
 
    # generate the response to Server
    def _process(self, request:Common.Request):
        # waiting the connect msg to confirm player ID
        if self.state == 2:
            if request.type == 0:
                self.player.id = request.id
                self.state = 3
                print("Get the Player ID %u from Server"%request.id)
            else:
                print("Invalid state or msg!")
                return
        elif self.state == 4:
            if request.type == 1:
                print("Get the Start Status %u from Server"%request.status)
                self.state = 5
            else:
                print("Invalid state or msg!")
                return
        elif self.state == 6:
            print("Client in Playing state\n")
        return

    def _response2server(self):
        # In connect state, need to confirm with Server
        if self.state == 1:
            rsp = Common.Response()
            rsp.setConnectType("Fa")
            self.send(rsp.toString())
            self.state =2
        # Rcv the Player ID from Server, send the Ready to Server
        elif self.state == 3: 
            rsp = Common.Response()
            rsp.setStatusType(self.player.id, 0)
            self.send(rsp.toString())
            self.state = 4
        # Confirm start 
        elif self.state == 5:
            rsp = Common.Response()
            rsp.setStatusType(self.player.id, 1)
            self.state = 6
            self.send(rsp.toString())
        # Playing 
        elif self.state == 6:
            self.send("Client Playing Response")
            pass
        else:
            print("Should not here")
        return

    def send(self,msg:str):
        data = "{}\n".format(msg.strip()).encode()
        #print("Send Msg:%s"%data)
        self.sock.send(data)
 
    def stop(self):
        #logging.info("{} broken".format(self.addr))
        self.event.set()
        self.sock.close()
 
        self.event.wait(3)
        
        logging.info("Client Close")
 
 
def main():
    #e = threading.Event()
    cc = Client()
    cc.start()
 
    loop = 10
    while True:
        #msg = input(">> ")
        loop = loop - 1
        sleep(1)
        #if msg.strip() == 'exit':
        if loop == 0:
            #cc.send(msg)
            cc.stop()
            break

        #cc.send(msg)
    pass
  
if __name__ == '__main__':
    main()