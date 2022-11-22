from enum import IntEnum, unique

gPlayerInitBet = 500

@unique
class CardType(IntEnum):
    cCARD_SPADE = 0 # 黑桃    \u2664
    cCARD_HEART = 1 # 红桃    \u2661
    cCARD_CLUB  = 2 # 梅花    \u2663
    cCARD_DIAMOND = 3 # 方片    \u2662
    cCARD_TYPE_INV = 4


@unique
class MsgType(IntEnum):
    cMSG_ASSIGN_ID = 0 # To try to connect with Server, assign a Player ID
    cMSG_SYNC_STATUS = 1 # Sync the status to 1 ready
    cMSG_ACQ_ACTION = 2 # acquire and response player action
    cMSG_ADJUST_BET =3  # adjust the player total bet
    cMSG_SEND_CARD  = 4 # Server send the Card info to Client
    cMSG_INV        = 10

@unique
class PlayerAction(IntEnum):
    cPLAYER_FOLD  = 0
    cPLAYER_CHECK = 1
    cPLAYER_CALL  = 2
    cPLAYER_RAISE = 3
    cPLAYER_ALLIN = 4
    cPLAYER_INV   = 10

##############################################################################
#     The Game Stage
#
##############################################################################
@unique
class GameStatus(IntEnum):
    GS_Init = 0           #下盲注
    GS_preFlop  = 1        #翻牌前
    GS_postFlop = 2       #翻牌后
    GS_Turn  = 3           #转牌后
    GS_River = 4          #河牌后
    GS_Final =5             #最后比大小


class Card():
    def __init__(self, t:CardType, v:int):
        self.t = t
        self.v = v
        # for the Card Type
        if CardType.cCARD_SPADE == t:
            type = '\u2664'
        elif CardType.cCARD_HEART == t:
            type = '\u2661'
        elif CardType.cCARD_CLUB == t:
            type = '\u2663'
        elif CardType.cCARD_DIAMOND == t:    
            type = '\u2662'
        else:
            type = ' '
        # for the Card value
        if v >= 2 and v <= 10:
            value = str(v)
        elif v == 11: # J
            value = 'J'
        elif v == 12 : # Q
            value = 'Q'
        elif v == 13 : # K
            value = 'K'
        elif v == 14: # A
            value = 'A'
        else:
            value = ' '
        
        self.str = "{type}{value}".format(type=type, value=value)
        print("Card %s"%self.str)

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
#    Count : x        # x 表示本局游戏有多少玩家,后续会跟上x个玩家的信息
#    P0    : Description0
#    P1    : Description1
#    P2    : Description2
#    P3    : Description3
#    P4    : Description4
#    P5    : Description5
#    P6    : Description6
#    P7    : Description7
#}
#### The third kind request 
#{
#    Type : 2
#    Player ID: 1
#    Option:0         # 0 Blind, 1 Bet
#    Bet:5            # 表示最低需要支付的筹码值
#    Behind: 7        # 表示在你之后，还有多少位Player决策。例如Blind消息，Behind则为0，因为只需要一个Player支付盲注
#    Bonus: 100       # 表示当前底池总共有多少价值
#    Status : 1       # 表示当前进展到一局游戏的哪一步了
#    Count : x        # x 表示本局游戏有多少玩家,后续会跟上x个玩家的信息
#    P0    : Description0
#    P1    : Description1
#    P2    : Description2
#    P3    : Description3
#    P4    : Description4
#    P5    : Description5
#    P6    : Description6
#    P7    : Description7
#}
#### The fourth kind request 
#{
#    Type : 3
#    Player ID: 1
#    Adjust: -5        
#} 
#####################################################################
class Request:
    def __init__(self, info:str):
        self.desc:list[str] = []

        req = info.strip('\n').strip().split(';')
        # confirm the type firstly
        item = req[0].strip('\n').strip().split(':')
        if 'Type' == item[0]:
            self.type = int(item[1])
        else:
            print("Invalid Request Info parameter")
        # Assign Player ID
        if self.type == MsgType.cMSG_ASSIGN_ID :
            self.name:str = req[1].strip('\n').strip().split(':')[1]
            self.id:int = int(req[2].strip('\n').strip().split(':')[1])
        # Sync Status 0 or 1
        elif self.type == MsgType.cMSG_SYNC_STATUS:
            self.id = int(req[1].strip('\n').strip().split(':')[1])
            self.status:int = int(req[2].strip('\n').strip().split(':')[1])
            self.totalPlayerCnt:int = int(req[3].strip('\n').strip().split(':')[1])

            for i in range(0, 8):
                print(" Try to get player %u"%i)
                print(req[4 + i].strip('\n').strip().split(':')[1])
                self.desc.append(req[4 + i].strip('\n').strip().split(':')[1])
            
        # Acquire Player Action
        elif self.type == MsgType.cMSG_ACQ_ACTION:
            self.id:int = int(req[1].strip('\n').strip().split(':')[1])
            self.option:int = int(req[2].strip('\n').strip().split(':')[1])
            self.bet:int = int(req[3].strip('\n').strip().split(':')[1])
            self.behind:int = int(req[4].strip('\n').strip().split(':')[1])
            self.bonus:int = int(req[5].strip('\n').strip().split(':')[1])
            self.status:int = int(req[6].strip('\n').strip().split(':')[1])
            self.totalPlayerCnt : int(req[7].strip('\n').strip().split(':')[1])
            
            for i in range(0, 8):
                print(" Try to get player %u"%i)
                print(req[7 + i].strip('\n').strip().split(':')[1])
                self.desc.append(req[4 + i].strip('\n').strip().split(':')[1])
        # Adjust Player total bet
        elif self.type == MsgType.cMSG_ADJUST_BET:
            self.id:int = int(req[1].strip('\n').strip().split(':')[1])
            self.adjust:int = int(req[2].strip('\n').strip().split(':')[1])
        # Receive the Card info    
        elif self.type == MsgType.cMSG_SEND_CARD:
            self.id:int = int(req[1].strip('\n').strip().split(':')[1])
            self.cardCnt:int = int(req[2].strip('\n').strip().split(':')[1])
            self.card0type:int  = int(req[3].strip('\n').strip().split(':')[1])
            self.card0value:int = int(req[4].strip('\n').strip().split(':')[1])
            self.card1type:int  = int(req[5].strip('\n').strip().split(':')[1])
            self.card1value:int = int(req[6].strip('\n').strip().split(':')[1])
            self.card2type:int  = int(req[7].strip('\n').strip().split(':')[1])
            self.card2value:int = int(req[8].strip('\n').strip().split(':')[1])
        else:
            print("Invalid Request Type"%self.type)
        

    def display(self):
        print("\tRequest type:%u"%(self.type))
        if (MsgType.cMSG_ASSIGN_ID == self.type):
            print("\tName:%s, ID:%u"%(self.name, self.id))
        elif (MsgType.cMSG_SYNC_STATUS == self.type):
            print("\tID: %u, Status:%u"%(self.id, self.status))
        elif (MsgType.cMSG_ACQ_ACTION == self.type):
            print("\tID:%u, Option:%u, Bet:%u, Behind:%u, Bonus:%u, Status:%u"
                %(self.id, self.option, self.bet, self.behind, self.bonus, self.status))
        elif self.type == MsgType.cMSG_ADJUST_BET:
            print("\tID: %u, Adjust Bet:%u"%(self.id, self.adjust))
        elif self.type == MsgType.cMSG_SEND_CARD:
            print("\tID: %d, Card Cnt: %u, Cards: %d-%d, %d-%d, %d-%d"%(self.id, self.cardCnt,
                self.card0type, self.card0value,
                self.card1type, self.card1value,
                self.card2type, self.card2value))



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
#    Total Bet : 400  # Player 剩余的总Bet数量，需要扣除本次action之后的值
#}
#### The fourth kind response  
#{
#    Type : 3
#    Player ID: 1
#    Adjust: 0          # Player选择支付的筹码，可能比require要求的大，即表示raise了
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
        self.adjust:int = 0

    def setConnectType(self, playerName:str):
        self.type = MsgType.cMSG_ASSIGN_ID
        self.name = playerName
        return

    def setStatusType(self, id:int, status:int):
        self.type = MsgType.cMSG_SYNC_STATUS
        self.PlayerId = id
        self.status = status
        return

    def setActionType(self, id:int, action:int, bet:int, total_bet:int):
        self.type = MsgType.cMSG_ACQ_ACTION
        self.PlayerId = id
        self.action = action
        self.bet = bet
        self.total_bet = total_bet
        return

    def setSendCardType(self, id:int):
        self.type = MsgType.cMSG_SEND_CARD
        self.id:int = id
        self.result = 0

    def toString(self) -> str:
        type0format = "Type : {type};\nPlayer Name: {name}\n"
        type1format = "Type : {type};\nPlayer ID: {id};\nStatus:{status}\n"
        type2format = "Type : {type};\nPlayer ID: {id};\nAction:{action};\nBet:{bet};\nTotal Bet:{totalBet}\n"
        type3format = "Type : {type};\nPlayer ID: {id};\nAdjust:{adjust}\n"
        message:str = None

        if self.type == MsgType.cMSG_ASSIGN_ID:
            message = type0format.format(type=0, name=self.name)
        elif self.type == MsgType.cMSG_SYNC_STATUS:
            message = type1format.format(type=1, id=self.PlayerId, status=self.status)
        elif self.type == MsgType.cMSG_ACQ_ACTION:
            message = type2format.format(type=2, id=self.PlayerId, action=self.action, bet=self.bet, totalBet = self.total_bet)
        elif self.type == MsgType.cMSG_ADJUST_BET:
            message = type3format.format(type=3, id=self.PlayerId, adjust=self.adjust)
        elif self.type == MsgType.cMSG_SEND_CARD:
            message = type1format.format(type=4, id=self.id, status=self.result) # Server doesn't care about this resp
        else:
            message = "Invalid Type!!!!"
            print("To String get invalid type %u"%self.type)
        return message