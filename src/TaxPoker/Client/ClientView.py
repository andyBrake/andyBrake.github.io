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
        # 定义窗口
        self.width = width
        self.height = height
        self.window = tk.Tk()
        self.window.title('PokerClient')

        #self.player_label:list = []  # ? Player display info
        self.public_card_info:tk.StringVar = None
        self.public_cards:list[Card] = []

        self.bonus_info:tk.StringVar = None
        self.bonus:int = 0

        self.curPlayerId:int = -1
        self.playerInfo:list[PlayerInfo] = []
        for i in range(0,8):
            player = PlayerInfo(name="Player {}".format(i), id=i, total_bet=gPlayerInitBet)
            self.playerInfo.append(player)

    # set all player info, got from Server
    def set_player_info(self, id:int, name:str): 
        playerInfoFormat = "-{n}-{i}-"
        self.curPlayerId = id
        self.playerInfo[id].player.name = name

        if self.playerInfo[id].player.id != id:
            print("Assign ID %d doesn't match ID %d"%(id, self.playerInfo[id].player.id))
        self.playerInfo[id].strVar.set(playerInfoFormat.format(n=name, i=id))
        return

    def launch_window(self):
        print("Launch Window")
        self._set_window_size()
        
        self._add_player()
    
        self._add_public_card()
        
        self._add_bonus()

        self._add_action_button()

        self.controller.viewReady = True
        
        self.window.mainloop()

    def _general_callback(self):
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
        self.public_card_info = tk.StringVar()
        self._update_public_card()

        label_card = tk.Label(self.window, textvariable = self.public_card_info, bg="#70CDFF",
                                # 设置标签内容区大小
                                width=35,height=2,
                                # 设置填充区距离、边框宽度和其样式（凹陷式）
                                padx=12, pady=12, borderwidth=5, relief="sunken")
        label_card.place(x=320, y=120)

    def _update_public_card(self):
        pcFormat = "\u2664 {card0}   \u2661 {card1}   \u2663 {card2}   \u2662 {card3}   \u2663 {card4}"
        self.public_card_info.set(value= "\u2664 A   \u2661 K   \u2663 Q   \u2662 J   \u2663 10")

    def _add_bonus(self):
        self.bonus_info = tk.StringVar()
        self._update_bonus()

        label_bonus = tk.Label(self.window, textvariable = self.bonus_info, bg="#7CCDFF",
                                # 设置标签内容区大小
                                width=10,height=2,
                                # 设置填充区距离、边框宽度和其样式（凹陷式）
                                padx=12, pady=12, borderwidth=5, relief="sunken")
        label_bonus.place(x=700, y=120)

    def _update_bonus(self):
        bonusFormat = " {b} \u2664\u2661\u2663\u2662"
        self.bonus_info.set(value=bonusFormat.format(b=self.bonus))

    def _add_action_button(self):
        window = self.window
        action_button_gap = 150

        fold_button = tk.Button(window, text="Fold", command=self._fold_cb)
        fold_button.place(x=0, y=450)
        
        check_button = tk.Button(window, text="Check", command=self._check_cb)
        check_button.place(x=action_button_gap, y=450)
        
        call_button = tk.Button(window, text="Call", command=self._call_cb)
        call_button.place(x=2*action_button_gap, y=450)
        
        raise_button = tk.Button(window, text="Raise", command=self._raise_cb)
        raise_button.place(x=3*action_button_gap, y=450)
        
        allin_button = tk.Button(window, text="Allin", command=self._allin_cb)
        allin_button.place(x=4*action_button_gap, y=450)
        return

    def _update_player_info(self, player_id, action:int, bet:int, str:string):
        playerInfoFormat = "Player:%s\nTotal Bet:%d\nAction:%s\nBet:%u"

        playerInfo = self.playerInfo[player_id]
        player_name = playerInfo.player.name
        player_strVar = playerInfo.strVar
        player_strVar.set(playerInfoFormat%(player_name, playerInfo.player.total_bet, 
                            str, playerInfo.player.pay_bet))

    def _fold_cb(self):
        self._update_player_info(4, PlayerAction.cPLAYER_FOLD, 0, "Fold")
        self._general_callback()

    def _check_cb(self):
        self._update_player_info(4, PlayerAction.cPLAYER_CHECK, 0, "Check")
        self._general_callback()

    def _call_cb(self):
        self._update_player_info(4, PlayerAction.cPLAYER_CALL, 0, "Call")
        self._general_callback()
    
    def _raise_cb(self):
        self._update_player_info(4, PlayerAction.cPLAYER_RAISE, 0, "Raise")
        self._general_callback()
    
    def _allin_cb(self):
        self._update_player_info(4, PlayerAction.cPLAYER_ALLIN, 0, "All in")
        self._general_callback()

if __name__ == '__main__':
    gui = ClientView()

    gui.launch_window()
    
