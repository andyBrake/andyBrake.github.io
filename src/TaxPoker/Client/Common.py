from enum import IntEnum, unique



@unique
class MsgType(IntEnum):
    cMSG_ASSIGN_ID = 0 # To try to connect with Server, assign a Player ID
    cMSG_SYNC_STATUS = 1 # Sync the status to 1 ready
    cMSG_ACQ_ACTION = 2 # acquire and response player action
    cMSG_INV        = 10

@unique
class PlayerAction(IntEnum):
    cPLAYER_FOLD  = 0
    cPLAYER_CHECK = 1
    cPLAYER_CALL  = 2
    cPLAYER_RAISE = 3
    cPLAYER_ALLIN = 4
    cPLAYER_INV   = 10


#####################################################################
# The Request message came from Server, send to Client
# Format is "Option:%s;\nRequireBet:%u;\nBehind:%u;\n"
#### The first kind request 
#{
#    Type : 0
#    Player Name: Fa
#    Player ID: 1
#}
#### The second kind request 
#{
#    Type : 1
#    Player ID: 1
#    Status:1         # Status 分类：0 表示Ready， 1 表示 Start
#}
#### The third kind request 
#{
#    Type : 2
#    Player ID: 1
#    Option:0         # 0 Blind, 1 Bet
#    Bet:5            # 表示最低需要支付的筹码值
#    Behind: 7        # 表示在你之后，还有多少位Player决策。例如Blind消息，Behind则为0，因为只需要一个Player支付盲注
#    Bonus: 100       # 表示当前底池总共有多少价值
#}
#####################################################################
class Request:
    def __init__(self, info:str):
        req = info.strip('\n').strip().split(';')
        # confirm the type firstly
        item = req[0].strip('\n').strip().split(':')
        if 'Type' == item[0]:
            self.type = int(item[1])
        else:
            print("Invalid Request Info parameter")

        if self.type == MsgType.cMSG_ASSIGN_ID :
            self.name:str = req[1].strip('\n').strip().split(':')[1]
            self.id:int = int(req[2].strip('\n').strip().split(':')[1])
        elif self.type == MsgType.cMSG_SYNC_STATUS:
            self.id = int(req[1].strip('\n').strip().split(':')[1])
            self.status:int = int(req[2].strip('\n').strip().split(':')[1])
        elif self.type == MsgType.cMSG_ACQ_ACTION:
            self.id:int = int(req[1].strip('\n').strip().split(':')[1])
            self.option:int = int(req[2].strip('\n').strip().split(':')[1])
            self.bet:int = int(req[3].strip('\n').strip().split(':')[1])
            self.behind:int = int(req[4].strip('\n').strip().split(':')[1])
            self.bonus:int = int(req[5].strip('\n').strip().split(':')[1])
        else:
            pass
        

    def display(self):
        print("Request type:%u"%(self.type))
        if (MsgType.cMSG_ASSIGN_ID == self.type):
            print("Name:%s, ID:%u"%(self.name, self.id))
        elif (MsgType.cMSG_SYNC_STATUS == self.type):
            print("ID: %u, Status:%u"%(self.id, self.status))
        elif (MsgType.cMSG_ACQ_ACTION == self.type):
            print("ID:%u, Option:%u, Bet:%u, Behind:%u, Bonus:%u"
                %(self.id, self.option, self.bet, self.behind, self.bonus))




#####################################################################
# The Response message came from Client, send to Server
#### The first kind response
#{
#    Type : 0
#    Player Name: Fa
#}
#### The second kind response 
#{
#    Type : 1         # type 分类:  0 表示connect消息，即完成链接之后发送Player Name的消息
#                     #            1 表示Status消息
#                     #            2 表示Option/Action消息
#    Player ID: 1
#    Status:0         # Status 分类：0 表示Ready， 1 表示 Start
#}
#### The third kind response  
#{
#    Type : 2
#    Player ID: 1
#    Action:1         # 0 Fold, 1 Check (此时Bet一定为0), 2 Raise（此时Bet一定比require要求的大）, 
#                     # 3 AllIn（此时Bet不一定比require要求的大，但是Player的剩余筹码归0） 
#    Bet: 10          # Player选择支付的筹码，可能比require要求的大，即表示raise了
#}
#####################################################################
class Response:
    def __init__(self):
        self.type:int = MsgType.cMSG_INV
        self.name:str = None
        self.PlayerId:int = 0
        self.status:int = 0
        self.action:int = 0
        self.bet:int = 0

    def setConnectType(self, playerName:str):
        self.type = MsgType.cMSG_ASSIGN_ID
        self.name = playerName
        return

    def setStatusType(self, id:int, status:int):
        self.type = MsgType.cMSG_SYNC_STATUS
        self.PlayerId = id
        self.status = status
        return

    def setActionType(self, id:int, action:int, bet:int):
        self.type = MsgType.cMSG_ACQ_ACTION
        self.PlayerId = id
        self.action = action
        self.bet = bet
        return

    def toString(self) -> str:
        type0format = "Type : {type};\nPlayer Name: {name}\n"
        type1format = "Type : {type};\nPlayer ID: {id};\nStatus:{status}\n"
        type2format = "Type : {type};\nPlayer ID: {id};\nAction:{action};\nBet:{bet}\n"
        message:str = None

        if self.type == MsgType.cMSG_ASSIGN_ID:
            message = type0format.format(type=0, name=self.name)
        elif self.type == MsgType.cMSG_SYNC_STATUS:
            message = type1format.format(type=1, id=self.PlayerId, status=self.status)
        elif self.type == MsgType.cMSG_ACQ_ACTION:
            message = type2format.format(type=2, id=self.PlayerId, action=self.action, bet=self.bet)
        else:
            message = "Invalid Type!!!!"
            print("To String get invalid type %u"%self.type)
        return message