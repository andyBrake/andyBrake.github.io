#TCP Client客户端 封装成类
import socket,threading,logging,datetime
import Common  

from enum import IntEnum, unique
from ClientView import *
from ClientModule import *
from time import sleep


DATEFMT="%H:%M:%S"
FORMAT = "[%(asctime)s]\t [%(threadName)s,%(thread)d] %(message)s"
logging.basicConfig(level=logging.INFO,format=FORMAT,datefmt=DATEFMT)


@unique
class ClientState(IntEnum):
    cINIT = 0 # initial statue
    cCONNECTED = 1 # To try to connect with Server, assign a Player ID

    cWAIT_PLAYER_ID = 2 # Sync the status to 1 ready
    cRESP_PLAYER_ID = 3 # acquire and response player action

    cWAIT_START_STATUS = 4
    cRESP_START_STATUS = 5

    cPLAYING_SYNC = 6
    

class ClientController:
    def __init__(self, p:Player, ip='127.0.0.1',port=10002):
        self.sock = socket.socket()
        self.addr = (ip,port)
        self.state = ClientState.cINIT
        self.player = p
        self.syncView:bool = False
        self.viewReady:bool = False
        self.request = None
        self.view = ClientView(controller=self)
        self.event = threading.Event()


    def start(self):
        try:
            self.sock.connect(self.addr)
            self.state = ClientState.cCONNECTED # already connect
            # 准备接收数据，recv是阻塞的，启动新的线程
            threading.Thread(target=self._recv,name='recv').start()
            # Init the window
            self.view.launch_window()
            sleep(1)
        except Exception as e:
            print(f"连接服务器失败: {e}")
            # 在GUI中显示错误信息
            if hasattr(self, 'view') and self.view:
                self.view.update_sysinfo(f"连接服务器失败: {e}")

 
    # 接收Server消息的线程
    def _recv(self):
        print("Start the Client Rcv Thread...")
        
        self._response2server()
        # then in the loop, the first reqeust should be the ID assign

        while not self.event.is_set():
            try:
                data = self.sock.recv(1024) #阻塞
            except Exception as e:
                if not self.event.is_set():
                    print("!!!!!Rcv Abnormal!!!! |%s|"%data)
                    logging.info(e) #有任何异常保证退出
                break

            if (len(data.strip()) > 0):
                #msg = "{:%H:%M:%S} IP:{} Msg:{}\n".format(datetime.datetime.now(),*self.addr, data.decode().strip())
                #print(msg)
                #logging.info("{}".format(data.decode()))
                print("\t\tRcv Server Msg:%s"%data)
                try:
                    self.request = Common.Request(data.decode())
                    self._process(self.request)
                    self._response2server()
                except Exception as e:
                    print(f"处理消息时出错: {e}")
 
    # generate the response to Server
    def _process(self, request:Common.Request):
        # waiting the connect msg to confirm player ID
        if self.state == ClientState.cWAIT_PLAYER_ID: 
            if request.type == Common.MsgType.cMSG_ASSIGN_ID:
                self.player.id = request.id
                self.state = ClientState.cRESP_PLAYER_ID
                self.syncView = False
                print("Get the Player ID %u from Server"%request.id)
                # 更新界面显示玩家ID
                if hasattr(self, 'view') and self.view:
                    self.view.update_sysinfo(f"已连接服务器，玩家ID: {request.id}")
            else:
                print("Invalid state or msg!")
                return
        elif self.state == ClientState.cWAIT_START_STATUS:
            if request.type == Common.MsgType.cMSG_SYNC_STATUS:
                print("Get the Start Status %u from Server"%request.status)
                self.syncView = True
                self.state = ClientState.cRESP_START_STATUS
            else:
                print("Invalid state or msg!")
                return
        elif self.state == ClientState.cPLAYING_SYNC:
            print("Client Receive Acquire, in Playing state\n")
            request.display()
            self.syncView = True

            # decide which option button is available
            self.enableViewOptionButton(request)

            self._playerAction(request)

        
        if self.syncView:
            self.notifyViewUpdate()
            self.syncView = False
        return

    def _response2server(self):
        # In connect state, need to confirm with Server
        if self.state == ClientState.cCONNECTED:
            rsp = Common.Response()
            rsp.setConnectType("Fa")
            self.send(rsp.toString())
            self.state =ClientState.cWAIT_PLAYER_ID
        # Rcv the Player ID from Server, send the Ready to Server
        elif self.state == ClientState.cRESP_PLAYER_ID: 
            rsp = Common.Response()
            rsp.setStatusType(self.player.id, 0)
            self.send(rsp.toString())
            self.state = ClientState.cWAIT_START_STATUS
        # Confirm start 
        elif self.state == ClientState.cRESP_START_STATUS:
            rsp = Common.Response()
            rsp.setStatusType(self.player.id, 1)
            self.state = ClientState.cPLAYING_SYNC
            self.send(rsp.toString())
        # Playing 
        elif self.state == ClientState.cPLAYING_SYNC:
            if self.request.type == Common.MsgType.cMSG_ADJUST_BET:
                rsp = self._rsp4AdjustBet()
                self.send(rsp.toString())
                print("\nResp Msg:")
                print(rsp.toString())
                print("\n")
                return

            if self.request.type == Common.MsgType.cMSG_SEND_CARD:
                rsp = Common.Response()
                rsp.setSendCardType(id=self.request.id)
                print("\nResp Msg:")
                print(rsp.toString())
                print("\n")
                self.send(rsp.toString())
                return

            rsp = self._FillAction()
            self.send(rsp.toString())
            print("\nResp Msg:")
            print(rsp.toString())
            print("\n")
        else:
            print("Should not here")
            # exit(1)  # 不要直接退出，而是给出提示
            if hasattr(self, 'view') and self.view:
                self.view.update_sysinfo("出现未知错误，请重新连接")
        return

    # This request must be a cMSG_ACQ_ACTION type, and include these below information
    #{
    #    Type : 2
    #    Player ID: 1
    #    Option:0         # 0 Blind, 1 Bet
    #    Bet:5            # 表示最低需要支付的筹码值
    #    Behind: 7        # 表示在你之后，还有多少位Player决策。例如Blind消息，Behind则为0，因为只需要一个Player支付盲注
    #    Bonus: 100       # 表示当前底池总共有多少价值
    #    Status: GS       # 当前游戏步骤  
    #}
    ####################################################################################
    def _playerAction(self, request:Common.Request):
        # 调整total bet，比如win或者buy in触发
        if request.type == Common.MsgType.cMSG_ADJUST_BET:
            self.player.total_bet = self.player.total_bet + request.adjust
            print("Player adjust bet %d to %d"%(request.adjust, self.player.total_bet))
            # 更新界面显示
            if hasattr(self, 'view') and self.view:
                self.view.update_sysinfo(f"筹码调整: {request.adjust}, 当前筹码: {self.player.total_bet}")
            return

        if request.type != Common.MsgType.cMSG_ACQ_ACTION:
            print("Invalid type! %u"%request.type)
            return
        # Acquire Blind bet, must pay
        if request.option == 0:
            self.player.isBlind = True
            self.player.pay_bet = request.bet
            # 更新界面显示
            if hasattr(self, 'view') and self.view:
                self.view.update_sysinfo(f"支付盲注: {request.bet}")
        # Acquire normal bet
        else:
            # Waiting the user action through View
            while True:
                if self.view.checkUpdateFlag():
                    break
                print("\t\t Waiting User Action")
                sleep(1)
            #self.player.pay_bet = request.bet
            print("Player require bet %u"%request.bet)
            print("Player isFold %s"%self.player.isFold)
            #request.behind:int = 
            #request.bonus:int = 
        pass
    
    def enableViewOptionButton(self, request:Common.Request):
        if self.request.type != Common.MsgType.cMSG_ACQ_ACTION:
            return
        # Blind, don't need user action
        if request.option == 0:
            return
        # Can Check    
        if request.bet == 0:
            self.view.enable_check_button()
            self.view.enable_raise_button()
        else:
            self.view.enable_call_button()
            self.view.enable_raise_button()
        self.view.enable_action_button()
        return

    def notifyViewUpdate(self):
        if self.syncView is False:
            return
        # Wait the Window Loop process start
        while self.viewReady == False:
            sleep(1)

        if self.request.type == Common.MsgType.cMSG_SYNC_STATUS:
            print("To Update view player info")
            self.view.update_sysinfo("玩家准备就绪")
            for id in range(len(self.request.desc)):
                self.view.update_all_player_info(id, self.request.desc[id])
        
        if self.request.type == Common.MsgType.cMSG_ACQ_ACTION:
            self.view.update_sysinfo("当前下注额: {}".format(self.request.bet))
            self.view.update_bonus(self.request.status, self.request.bonus)
        
        if self.request.type == Common.MsgType.cMSG_SEND_CARD:
            print("  To update card info, id %d, cnt %u"%(self.request.id, self.request.cardCnt))
            self.view.update_sysinfo("发牌: {}张".format(self.request.cardCnt))
            self.view.update_cards(self.request.id, self.request.cardCnt, 
                self.request.card0type, self.request.card0value,
                self.request.card1type, self.request.card1value,
                self.request.card2type, self.request.card2value)
        return

    def _FillAction(self):
        action = Common.PlayerAction.cPLAYER_INV
        ## TODO : to get the user action from View
        # self.player.pay_bet
        # isFold
        # isAllin

        if self.player.isFold:
            print("  This Player Fold.")
            action = Common.PlayerAction.cPLAYER_FOLD
        elif self.player.isAllin:
            action = Common.PlayerAction.cPLAYER_ALLIN
        else: # check, call or raise depend on the pay bet
            if self.player.pay_bet == 0:
                print("  This Player Check.")
                action = Common.PlayerAction.cPLAYER_CHECK
            elif self.player.pay_bet == self.request.bet:
                print("  This Player Call.")
                action = Common.PlayerAction.cPLAYER_CALL
            else:
                print("  This Player Raise.")
                action = Common.PlayerAction.cPLAYER_RAISE
        print("Acquire Bet %u, then pay bet %u "%(self.request.bet, self.player.pay_bet))
        self.player.total_bet = self.player.total_bet - self.player.pay_bet

        rsp = Common.Response()            
        rsp.setActionType(self.player.id, action=action, 
                            bet=self.player.pay_bet, total_bet=self.player.total_bet)
        
        return rsp
    
    def _rsp4AdjustBet(self):
        rsp = Common.Response()
        rsp.type = Common.MsgType.cMSG_ADJUST_BET
        
        return rsp

    def send(self,msg:str):
        try:
            data = "{}\n".format(msg.strip()).encode()
            #print("Send Msg:%s"%data)
            self.sock.send(data)
        except Exception as e:
            print(f"发送消息失败: {e}")
            if hasattr(self, 'view') and self.view:
                self.view.update_sysinfo(f"发送消息失败: {e}")

    def stop(self):
        #logging.info("{} broken".format(self.addr))
        self.event.set()
        try:
            self.sock.close()
        except:
            pass

        self.event.wait(3)
        
        logging.info("Client Close")
 
 
def main():
    #e = threading.Event()
    player = Player(name = "Fa", id=-1, total_bet = 500)
    cc = ClientController(p=player)
    cc.start()

    # 移除自动退出逻辑，让玩家可以持续游戏
    # loop = 20
    # while True:
    #     #msg = input(">> ")
    #     loop = loop - 1
    #     sleep(1)
    #     #if msg.strip() == 'exit':
    #     if loop == 0:
    #         #cc.send(msg)
    #         cc.stop()
    #         break

    #     #cc.send(msg)
    pass
  
if __name__ == '__main__':
    main()