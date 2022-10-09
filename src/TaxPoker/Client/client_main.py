


#TCP Client客户端 封装成类
import socket,threading,logging,datetime
from time import sleep
DATEFMT="%H:%M:%S"
FORMAT = "[%(asctime)s]\t [%(threadName)s,%(thread)d] %(message)s"
logging.basicConfig(level=logging.INFO,format=FORMAT,datefmt=DATEFMT)
 
 
class ChatClient:
    def __init__(self,ip='127.0.0.1',port=10002):
        self.sock = socket.socket()
        self.addr = (ip,port)
 
        self.event = threading.Event()
 
 
    def start(self):
        print("To connect")
        self.sock.connect(self.addr)

        # 准备接收数据，recv是阻塞的，启动新的线程
        threading.Thread(target=self._recv,name='recv').start()
        sleep(1)
 
 
    def _recv(self):
        print("To receive  :")
        self.send("Player Name: Fa")

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
                print("Msg:%s"%data)
                #self._analysisRequest(data.decode())
                self.send("Fold")
 
    def _analysisRequest(self, message:str):
        # Option : Blind, Check, Bet
        msgFormat = "Option:%s;\nRequireBet:%u;\nBehind:%u;\n"

        req = message.split(';')
        print(req)

        for each in req:
            item = each.split(':')
            print("Item name : %s, Item value : %s"%(item[0], item[1]))

    def send(self,msg:str):
        data = "{}\n".format(msg.strip()).encode()
        print("Send Msg:%s"%data)
        self.sock.send(data)
 
    def stop(self):
        #logging.info("{} broken".format(self.addr))
        self.event.set()
        self.sock.close()
 
        self.event.wait(3)
        
        logging.info("Client Close")
 
 
def main():
    #e = threading.Event()
    cc = ChatClient()
    cc.start()
 
    while True:
        msg = input(">> ")

        if msg.strip() == 'exit':
            cc.send(msg)
            sleep(1)
            cc.stop()
            break

        cc.send(msg)
 
    pass
  
if __name__ == '__main__':
    main()