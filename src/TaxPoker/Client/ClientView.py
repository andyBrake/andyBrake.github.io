from distutils.cmd import Command
import string
import tkinter as tk
from Common import *
from ClientController import *
from ClientModule import *

class PlayerInfo:
    def __init__(self, name:str, id:int, total_bet:int):
        self.player = Player(name, id, total_bet)
        self.strVar:tk.StringVar = None


class ClientView:
    def __init__(self, controller, width=1200, height=700):
        self.controller = controller
        self.isUpdate = False
        # 定义窗口
        self.width = width
        self.height = height
        self.window = tk.Tk()
        self.window.title('PokerClient')

        self.sysInfo:tk.StringVar = None

        self.private_card_info:tk.StringVar = None
        self.private_cards:list[Card] = []
        for i in range(0,2):
            self.private_cards.append(Card(CardType.cCARD_TYPE_INV, 0))

        self.public_card_info:tk.StringVar = None
        self.public_cards:list[Card] = []
        self.public_card_cnt = 0
        for i in range(0,5):
            self.public_cards.append(Card(CardType.cCARD_TYPE_INV, 0))

        self.bonus_info:tk.StringVar = None
        self.bonus:int = 0
        self.gameStage = 0

        self.curPlayerId:int = -1
        self.playerInfo:list[PlayerInfo] = []
        for i in range(0,8):
            player = PlayerInfo(name="Player {}".format(i), id=i, total_bet=gPlayerInitBet)
            self.playerInfo.append(player)

    # Launch the window and loop, the function will not return
    def launch_window(self):
        print("Launch Window")
        self._set_window_size()
        
        self._add_player()
    
        self._add_public_card()
        
        self._add_bonus()

        self._add_action_button()

        self._add_sysinfo()

        self.controller.viewReady = True
        
        self.window.mainloop()

    def enable_check_button(self):
        self.check_button['state'] = 'normal'
        return

    def enable_call_button(self):
        self.call_button['state'] = 'normal'
        return
    
    def enable_raise_button(self):
        self.raise_button['state'] = 'normal'
        return

    def enable_action_button(self):
        self.fold_button['state'] = 'normal'
        self.allin_button['state'] = 'normal'

    def update_all_player_info(self, id:int, desc:str):
        playerInfoFormat = "{desc}"
        print("Update Player %u to Desc %s"%(id, desc))
        if 'None' not in desc:
            self.playerInfo[id].player.name = desc.split('(')[0].strip('(').strip()
            tt = desc.split('(')[1].strip().strip(')').strip('(')
            tt = tt.strip(')').strip('(')
            self.playerInfo[id].player.total_bet = int(tt)
            self._update_player_info(id, action=0, bet=0, str=' NA ')
        else:
            self.playerInfo[id].strVar.set(playerInfoFormat.format(desc=desc))
        return

    def update_cards(self, id:int, cardCnt:int, t0:int, v0:int, t1:int, v1:int, t2:int, v2:int):
        # Public Card Update
        if id < 0:
            self.public_cards[self.public_card_cnt] = Card(t0, v0)
            self.public_card_cnt = self.public_card_cnt + 1

            if cardCnt == 1:
                self._update_public_card(self.public_cards)
                return

            self.public_cards[self.public_card_cnt] = Card(t1, v1)
            self.public_card_cnt = self.public_card_cnt + 1
            if cardCnt == 2:
                self._update_public_card(self.public_cards)
                return
            
            self.public_cards[self.public_card_cnt] = Card(t2, v2)
            self.public_card_cnt = self.public_card_cnt + 1
            self._update_public_card(self.public_cards)
            return
        else:
            self.private_cards[0] = Card(t0, v0)
            self.private_cards[1] = Card(t1, v1)

            self.private_card_info.set(value="My Card:\n{} {}".format(
                    self.private_cards[0].str, self.private_cards[1].str))
        return

    def update_bonus(self, stage:int, bonus:int):
        self.gameStage = stage
        self.bonus = bonus
        self._update_bonus(self.gameStage, self.bonus)
        pass

    def update_sysinfo(self, info:str):
        self.sysInfo.set(value="SysInfo : \n{}".format(info))

    def checkUpdateFlag(self):
        if self.isUpdate:
            self.isUpdate = False
            return True
        return False

    ###############################################################################
    ## Private Method
    ###############################################################################

    def _general_callback(self):
        self.isUpdate = True
        self.fold_button['state'] = 'disabled'
        self.check_button['state'] = 'disabled'
        self.call_button['state'] = 'disabled'
        self.raise_button['state'] = 'disabled'
        self.allin_button['state'] = 'disabled'

        self.window.update()

    def _set_window_size(self):
        # 窗口居中，获取屏幕尺寸以计算布局参数，使窗口居屏幕中央
        screenwidth = self.window.winfo_screenwidth()
        screenheight = self.window.winfo_screenheight()
        #print("W %d-%d H %d-%d"%((screenwidth-self.width)/2, screenwidth, (screenheight-self.height)/2, screenheight))
        size_geo = '%dx%d+%d+%d' % (self.width, self.height, (screenwidth-self.width)/2, (screenheight-self.height)/2)
        self.window.geometry(size_geo)

    def _add_player(self):
        cntPerRow = 4 # 每行显示Player的个数
        pixWidth = 300
        pixHeight = 200

        for i in range(0,8):
            info = tk.StringVar()
            info.set("Init")
            self.playerInfo[i].strVar = info

            label = tk.Label(self.window, textvariable=info, bg="#7CCD7C",
                                # 设置标签内容区大小
                                width=15,height=4,
                                # 设置填充区距离、边框宽度和其样式（凹陷式）
                                padx=10, pady=10, borderwidth=10, relief="sunken")
            if i < cntPerRow:
                x = i * pixWidth
            else:
                x = (i-cntPerRow) * pixWidth
                
            y = 0 if (i < cntPerRow) else pixHeight  
            
            label.place(x=x, y=y)
            # ?? play info str
            #self.player_label.append(label)
        return

    def _add_public_card(self):
        # Private Card 
        self.private_card_info = tk.StringVar()
        self.private_card_info.set(value = " NA-NA")
        label_card = tk.Label(self.window, textvariable = self.private_card_info, bg="#70CDFF",
                                # 设置标签内容区大小
                                width=35,height=2,
                                # 设置填充区距离、边框宽度和其样式（凹陷式）
                                padx=12, pady=12, borderwidth=5, relief="sunken")
        label_card.place(x=320, y=320)

        # Public Card
        self.public_card_info = tk.StringVar()
        self._update_public_card(self.public_cards)

        label_card = tk.Label(self.window, textvariable = self.public_card_info, bg="#70CDFF",
                                # 设置标签内容区大小
                                width=35,height=2,
                                # 设置填充区距离、边框宽度和其样式（凹陷式）
                                padx=12, pady=12, borderwidth=5, relief="sunken")
        label_card.place(x=320, y=120)

    def _update_public_card(self, card):
        pcFormat = "Public Card: {card0} {card1} {card2} {card3} {card4}"
        value = pcFormat.format(card0=card[0].str, card1=card[1].str, 
                    card2=card[2].str, card3=card[3].str, card4=card[4].str)

        self.public_card_info.set(value = value)

    def _add_bonus(self):
        self.bonus_info = tk.StringVar()
        self._update_bonus(self.gameStage, self.bonus)

        label_bonus = tk.Label(self.window, textvariable = self.bonus_info, bg="#7CCDFF",
                                # 设置标签内容区大小
                                width=10,height=2,
                                # 设置填充区距离、边框宽度和其样式（凹陷式）
                                padx=12, pady=12, borderwidth=5, relief="sunken")
        label_bonus.place(x=700, y=120)

    def _update_bonus(self, stage:int, bonus:int):
        stageDesc = ['Pay Blind', 'Pre-Flop', 'Post-Flop', 'Turn', 'River', 'Final']
        bonusFormat = "Stage: {s}\nPot: {b}"

        self.bonus_info.set(value=bonusFormat.format(s=stageDesc[stage], b=bonus))

    def _add_action_button(self):
        window = self.window
        action_button_gap = 150

        self.fold_button = tk.Button(window, text="Fold", command=self._fold_cb, state="disabled")
        self.fold_button.place(x=0, y=450)
        
        self.check_button = tk.Button(window, text="Check", command=self._check_cb, state="disabled")
        self.check_button.place(x=action_button_gap, y=450)
        
        self.call_button = tk.Button(window, text="Call", command=self._call_cb, state="disabled")
        self.call_button.place(x=2*action_button_gap, y=450)
        
        self.raise_button = tk.Button(window, text="Raise", command=self._raise_cb, state="disabled")
        self.raise_button.place(x=3*action_button_gap, y=450)
        
        self.allin_button = tk.Button(window, text="Allin", command=self._allin_cb, state="disabled")
        self.allin_button.place(x=4*action_button_gap, y=450)
        return

    def _add_sysinfo(self):
        self.sysInfo = tk.StringVar()
        self.sysInfo.set(value="None")

        label_sysinfo = tk.Label(self.window, textvariable = self.sysInfo, bg="#7CCDFF",
                                # 设置标签内容区大小
                                width=35,height=2,
                                # 设置填充区距离、边框宽度和其样式（凹陷式）
                                padx=12, pady=12, borderwidth=5, relief="sunken")
        label_sysinfo.place(x=700, y=320)

    def _update_player_info(self, player_id, action:int, bet:int, str:string):
        playerInfoFormat = "Player:%s\nTotal Bet:%d\nAction:%s\nBet:%u"

        playerInfo = self.playerInfo[player_id]
        player_name = playerInfo.player.name
        player_strVar = playerInfo.strVar
        player_strVar.set(playerInfoFormat%(player_name, playerInfo.player.total_bet, 
                            str, playerInfo.player.pay_bet))

    def _fold_cb(self):
        self.controller.player.isFold = True
        self._update_player_info(player_id=self.controller.player.id, 
                    action= PlayerAction.cPLAYER_FOLD, bet=0, str="Fold")
        self._general_callback()

    def _check_cb(self):
        self.controller.player.pay_bet = 0 
        self._update_player_info(player_id=self.controller.player.id, 
                    action= PlayerAction.cPLAYER_CHECK, bet=0, str="Check")
        self._general_callback()

    def _call_cb(self):
         # TODO : the request maybe None
        self.controller.player.pay_bet = self.controller.request.bet
        self._update_player_info(player_id=self.controller.player.id, 
                    action=PlayerAction.cPLAYER_CALL, bet=0, str="Call")
        self._general_callback()
    
    def _raise_cb(self):
        print("Sorry, don't support Raise so far")
        return
        self.controller.player.pay_bet = 10 # TODO : to read the request bet to set the pay bet value
        self._update_player_info(player_id=self.controller.player.id, 
                    action=PlayerAction.cPLAYER_RAISE, bet=0, str="Raise")
        self._general_callback()
    
    def _allin_cb(self):
        print("Sorry, don't support Allin so far")
        return
        self._update_player_info(player_id=self.controller.player.id, 
                    action=PlayerAction.cPLAYER_ALLIN, bet=0, str="All in")
        self._general_callback()

if __name__ == '__main__':
    gui = ClientView()

    gui.launch_window()
    
