


#TCP Client客户端 封装成类
import socket,threading,logging,datetime
DATEFMT="%H:%M:%S"
FORMAT = "[%(asctime)s]\t [%(threadName)s,%(thread)d] %(message)s"
logging.basicConfig(level=logging.INFO,format=FORMAT,datefmt=DATEFMT)
 
 
class ChatClient:
    def __init__(self,ip='127.0.0.1',port=9999):
        self.sock = socket.socket()
        self.addr = (ip,port)
 
        self.event = threading.Event()
        self.start()
 
 
    def start(self):
        self.sock.connect(self.addr)
 
        # 准备接收数据，recv是阻塞的，启动新的线程
        threading.Thread(target=self._recv,name='recv').start()
 
 
    def _recv(self):
        while not self.event.is_set():
            try:
                data = self.sock.recv(1024) #阻塞
 
            except Exception as e:
                logging.info(e) #有任何异常保证退出
                break
            msg = "{:%H:%M:%S} {}:{}\n{}\n".format(datetime.datetime.now(),*self.addr,data.decode().strip())
            print(type(msg),msg)
            logging.info("{}".format(data.decode()))
 
    def send(self,msg:str):
        data = "{}\n".format(msg.strip()).encode()
        self.sock.send(data)
 
    def stop(self):
        logging.info("{} broken".format(self.addr))
        self.sock.close()
 
        self.event.wait(3)
        self.event.set()
        logging.info("byebye")
 
 
def main():
    #e = threading.Event()
    cc = ChatClient()
 
    while True:
        msg = input(">>> ")
        if msg.strip() == 'quit':
            cc.stop()
            break
        cc.send(msg)
 
  
if __name__ == '__main__':
    main()