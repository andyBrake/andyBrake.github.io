import string
import tkinter as tk


action_check = 1
action_call = 2
action_raise = 3
action_fold = 4
action_allin = 5

player_info_format = "Player:%s\nRemind Bet:%d\nAction:%s\nBet:%u"

class PlayerInfo:
    def __init__(self, id:int, name:str, remindBet:int = 400):
        self.id = id
        self.name = name
        self.action:str = "Option"
        self.bet = 0
        self.remindBet = remindBet
        self.strVar:tk.StringVar = None


class ClientGui:
    def __init__(self, id, width=1200, height=700):
        print("----")
        self.width = width
        self.height = height
        self.curPlayerId = id
        self.playerInfo:list[PlayerInfo] = []
        # 定义窗口
        self.window = tk.Tk()
        self.window.title('Poker')

        self.player_label:list = []
        self.public_card_info = None
        self.bonus_info = None

        for i in range(0,8):
            player = PlayerInfo(id=i, name="P{}".format(i))
            self.playerInfo.append(player)

    # set all player info, got from Server
    def set_player_info(self, allPlayer): 
        #for player in allPlayer: #player is a list: [id, name, remindBet]
        #    self.playerInfo.append(PlayerInfo(id=player[0], name=player[1], remindBet = player[2]))
        pass

    def launch_window(self):
        print("Launch Window")
        self._set_window_size()
        
        self._add_player()
    
        self._add_public_card()
        
        self._add_bonus()

        self._add_action_button()
        
        # 点击执行按钮
        button = tk.Button(self.window, text="执行", command=self._general_callback)
        button.place(x=0, y=650)

        self.window.mainloop()

    def _general_callback(self):
        self.window.update()

    def _set_window_size(self):
        # 窗口居中，获取屏幕尺寸以计算布局参数，使窗口居屏幕中央
        screenwidth = self.window.winfo_screenwidth()
        screenheight = self.window.winfo_screenheight()
        print("W %d-%d H %d-%d"%((screenwidth-self.width)/2, screenwidth, (screenheight-self.height)/2, screenheight))
        size_geo = '%dx%d+%d+%d' % (self.width, self.height, (screenwidth-self.width)/2, (screenheight-self.height)/2)
        self.window.geometry(size_geo)

    def _add_player(self):
        cntPerRow = 4 # 每行显示Player的个数
        pixWidth = 300
        pixHeight = 200
        window = self.window

        for i in range(0,8):
            info = tk.StringVar()
            info.set("Init")
            self.playerInfo[i].strVar = info

            label = tk.Label(window, textvariable=info, bg="#7CCD7C",
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
            self.player_label.append(label)
        return

    # pubic card
    def _add_public_card(self):
        window = self.window
        self.public_card_info = tk.StringVar()
        self.public_card_info.set(value= "\u2664 A   \u2661 K   \u2663 Q   \u2662 J   \u2663 10")

        label_card = tk.Label(window, textvariable = self.public_card_info, bg="#70CDFF",
                                # 设置标签内容区大小
                                width=35,height=2,
                                # 设置填充区距离、边框宽度和其样式（凹陷式）
                                padx=12, pady=12, borderwidth=5, relief="sunken")
        label_card.place(x=320, y=120)

    # bonus label
    def _add_bonus(self):
        window = self.window
        self.bonus_info = tk.StringVar()
        self.bonus_info.set(value=" 0 \u2664\u2661\u2663\u2662")

        label_bonus = tk.Label(window, textvariable = self.bonus_info, bg="#7CCDFF",
                                # 设置标签内容区大小
                                width=10,height=2,
                                # 设置填充区距离、边框宽度和其样式（凹陷式）
                                padx=12, pady=12, borderwidth=5, relief="sunken")
        label_bonus.place(x=700, y=120)

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
        player = self.playerInfo[player_id]
        player_name = player.name
        player_strVar = player.strVar
        player_strVar.set(player_info_format%(player_name, player.remindBet, str, player.bet))

    def _fold_cb(self):
        self._update_player_info(4, action_fold, 0, "Fold")
        self._general_callback()

    def _check_cb(self):
        self._update_player_info(4, action_check, 0, "Check")
        self._general_callback()

    def _call_cb(self):
        self._update_player_info(4, action_call, 0, "Call")
        self._general_callback()
    
    def _raise_cb(self):
        self._update_player_info(4, action_raise, 0, "Raise")
        self._general_callback()
    
    def _allin_cb(self):
        self._update_player_info(4, action_allin, 0, "All in")
        self._general_callback()

if __name__ == '__main__':
    print("Hello")
    gui = ClientGui(id=0)

    gui.launch_window()
    
