from distutils.cmd import Command
import string
import tkinter as tk
from tkinter import ttk, messagebox
from Common import *
from ClientController import *
from ClientModule import *

class PlayerInfo:
    def __init__(self, name:str, id:int, total_bet:int):
        self.player = Player(name, id, total_bet)
        self.strVar:tk.StringVar = None


class ClientView:
    def __init__(self, controller, width=1200, height=800):
        self.controller = controller
        self.isUpdate = False
        # 定义窗口
        self.width = width
        self.height = height
        self.window = tk.Tk()
        self.window.title('德州扑克客户端 - PokerClient')
        self.window.configure(bg='#f0f0f0')

        self.sysInfo:tk.StringVar = tk.StringVar()
        self.sysInfo.set("等待连接服务器...")

        self.private_card_info:tk.StringVar = tk.StringVar()
        self.private_card_info.set("我的手牌: 等待发牌...")
        
        self.private_cards:list[Card] = []
        for i in range(0,2):
            self.private_cards.append(Card(CardType.cCARD_TYPE_INV, 0))

        self.public_card_info:tk.StringVar = tk.StringVar()
        self.public_cards:list[Card] = []
        self.public_card_cnt = 0
        for i in range(0,5):
            self.public_cards.append(Card(CardType.cCARD_TYPE_INV, 0))

        self.bonus_info:tk.StringVar = tk.StringVar()
        self.bonus:int = 0
        self.gameStage = 0

        self.curPlayerId:int = -1
        self.playerInfo:list[PlayerInfo] = []
        for i in range(0,8):
            player = PlayerInfo(name="Player {}".format(i), id=i, total_bet=gPlayerInitBet)
            self.playerInfo.append(player)
            
        # Raise 输入框
        self.raise_amount = tk.StringVar()
        self.raise_entry = None

    # Launch the window and loop, the function will not return
    def launch_window(self):
        print("Launch Window")
        self._set_window_size()
        
        self._add_title()
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
        if self.raise_entry:
            self.raise_entry['state'] = 'normal'
        return

    def enable_action_button(self):
        self.fold_button['state'] = 'normal'
        self.allin_button['state'] = 'normal'
        return

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

            self.private_card_info.set(value="我的手牌:\n{} {}".format(
                    self.private_cards[0].str, self.private_cards[1].str))
        return

    def update_bonus(self, stage:int, bonus:int):
        self.gameStage = stage
        self.bonus = bonus
        self._update_bonus(self.gameStage, self.bonus)
        pass

    def update_sysinfo(self, info:str):
        if self.sysInfo:
            self.sysInfo.set(value="系统信息 : \n{}".format(info))

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
        if self.raise_entry:
            self.raise_entry['state'] = 'disabled'

        self.window.update()

    def _set_window_size(self):
        # 窗口居中，获取屏幕尺寸以计算布局参数，使窗口居屏幕中央
        screenwidth = self.window.winfo_screenwidth()
        screenheight = self.window.winfo_screenheight()
        #print("W %d-%d H %d-%d"%((screenwidth-self.width)/2, screenwidth, (screenheight-self.height)/2, screenheight))
        size_geo = '%dx%d+%d+%d' % (self.width, self.height, (screenwidth-self.width)/2, (screenheight-self.height)/2)
        self.window.geometry(size_geo)

    def _add_title(self):
        # 添加标题
        title_frame = tk.Frame(self.window, bg='#2c3e50', height=60)
        title_frame.pack(fill=tk.X)
        title_frame.pack_propagate(False)
        
        title_label = tk.Label(title_frame, text="德州扑克游戏", font=("Arial", 20, "bold"), 
                              fg='white', bg='#2c3e50')
        title_label.pack(expand=True)

    def _add_player(self):
        # 玩家信息区域
        player_frame = tk.Frame(self.window, bg='#ecf0f1')
        player_frame.pack(fill=tk.X, padx=10, pady=5)
        
        # 上排玩家 (0-3)
        top_row = tk.Frame(player_frame, bg='#ecf0f1')
        top_row.pack(fill=tk.X)
        
        # 下排玩家 (4-7)
        bottom_row = tk.Frame(player_frame, bg='#ecf0f1')
        bottom_row.pack(fill=tk.X, pady=5)
        
        cntPerRow = 4 # 每行显示Player的个数
        pixWidth = 280
        pixHeight = 120

        for i in range(0,8):
            info = tk.StringVar()
            info.set("等待玩家加入...")
            self.playerInfo[i].strVar = info

            # 创建玩家信息框架
            player_container = tk.Frame(top_row if i < cntPerRow else bottom_row, 
                                       bg='#2ecc71', relief='raised', bd=2)  # 绿色背景
            player_container.pack(side=tk.LEFT, padx=5, pady=5)
            
            # 玩家信息标签 (使用更深的绿色)
            label = tk.Label(player_container, textvariable=info, bg="#27ae60",
                                font=("Arial", 10, "bold"),
                                width=25, height=5,
                                padx=8, pady=8, borderwidth=2, relief="groove",
                                fg="white")  # 白色文字以便在绿背景上清晰显示
            
            label.pack()
            
        return

    def _add_public_card(self):
        # 公共牌区域框架
        card_frame = tk.Frame(self.window, bg='#f0f0f0')
        card_frame.pack(fill=tk.X, padx=10, pady=10)
        
        # 标题
        title_label = tk.Label(card_frame, text="公共牌区", font=("Arial", 14, "bold"), 
                              bg='#f0f0f0')
        title_label.pack()
        
        # 私人牌显示
        private_label = tk.Label(card_frame, textvariable = self.private_card_info, 
                                bg="#3498db", font=("Arial", 12, "bold"),  # 蓝色背景显示私人牌
                                width=40, height=3,
                                padx=10, pady=10, borderwidth=3, relief="ridge",
                                fg="white")
        private_label.pack(pady=5)
        
        # 公共牌显示
        self._update_public_card(self.public_cards)

        public_label = tk.Label(card_frame, textvariable = self.public_card_info, 
                               bg="#3498db", font=("Arial", 12, "bold"),  # 蓝色背景显示公共牌
                               width=50, height=3,
                               padx=10, pady=10, borderwidth=3, relief="ridge",
                               fg="white")
        public_label.pack(pady=5)

    def _update_public_card(self, card):
        pcFormat = "公共牌: {card0} {card1} {card2} {card3} {card4}"
        value = pcFormat.format(card0=card[0].str, card1=card[1].str, 
                    card2=card[2].str, card3=card[3].str, card4=card[4].str)

        self.public_card_info.set(value = value)

    def _add_bonus(self):
        # 底池信息框架
        bonus_frame = tk.Frame(self.window, bg='#f0f0f0')
        bonus_frame.pack(fill=tk.X, padx=10, pady=5)
        
        # 标题
        title_label = tk.Label(bonus_frame, text="底池信息", font=("Arial", 14, "bold"), 
                              bg='#f0f0f0')
        title_label.pack()
        
        self._update_bonus(self.gameStage, self.bonus)

        label_bonus = tk.Label(bonus_frame, textvariable = self.bonus_info, 
                              bg="#f1c40f", font=("Arial", 12, "bold"),  # 黄色背景显示底池
                              width=30, height=3,
                              padx=10, pady=10, borderwidth=3, relief="ridge",
                              fg="black")  # 黑色文字在黄背景上更清晰
        label_bonus.pack(pady=5)

    def _update_bonus(self, stage:int, bonus:int):
        stageDesc = ['支付盲注', '翻牌前', '翻牌后', '转牌', '河牌', '摊牌']
        bonusFormat = "阶段: {s}\n底池: {b}"
        
        # 如果是初始状态，显示等待
        if stage == 0 and bonus == 0:
            self.bonus_info.set(value="等待游戏开始...")
        else:
            self.bonus_info.set(value=bonusFormat.format(s=stageDesc[stage], b=bonus))

    def _add_action_button(self):
        # 操作按钮区域
        action_frame = tk.Frame(self.window, bg='#f0f0f0')
        action_frame.pack(fill=tk.X, padx=10, pady=10)
        
        # 标题
        title_label = tk.Label(action_frame, text="游戏操作", font=("Arial", 14, "bold"), 
                              bg='#f0f0f0')
        title_label.pack()
        
        # 按钮容器
        button_container = tk.Frame(action_frame, bg='#f0f0f0')
        button_container.pack(pady=10)
        
        # 按钮样式
        button_style = {"font": ("Arial", 12, "bold"), "width": 10, "height": 2}
        
        self.fold_button = tk.Button(button_container, text="弃牌\nFold", 
                                    command=self._fold_cb, state="disabled",
                                    bg="#e74c3c", fg="white", **button_style)
        self.fold_button.grid(row=0, column=0, padx=5)
        
        self.check_button = tk.Button(button_container, text="过牌\nCheck", 
                                     command=self._check_cb, state="disabled",
                                     bg="#9b59b6", fg="white", **button_style)  # 粉色保持不变
        self.check_button.grid(row=0, column=1, padx=5)
        
        self.call_button = tk.Button(button_container, text="跟注\nCall", 
                                    command=self._call_cb, state="disabled",
                                    bg="#9b59b6", fg="white", **button_style)  # 粉色保持不变
        self.call_button.grid(row=0, column=2, padx=5)
        
        # Raise 区域
        raise_frame = tk.Frame(button_container, bg='#f0f0f0')
        raise_frame.grid(row=0, column=3, padx=5)
        
        self.raise_button = tk.Button(raise_frame, text="加注\nRaise", 
                                     command=self._raise_cb, state="disabled",
                                     bg="#9b59b6", fg="white", **button_style)  # 粉色保持不变
        self.raise_button.pack()
        
        # Raise 输入框
        entry_frame = tk.Frame(raise_frame, bg='#f0f0f0')
        entry_frame.pack()
        
        tk.Label(entry_frame, text="金额:", bg='#f0f0f0').pack(side=tk.LEFT)
        self.raise_entry = tk.Entry(entry_frame, textvariable=self.raise_amount, 
                                   width=8, state="disabled")
        self.raise_entry.pack(side=tk.LEFT)
        
        self.allin_button = tk.Button(button_container, text="全押\nAll-in", 
                                     command=self._allin_cb, state="disabled",
                                     bg="#9b59b6", fg="white", **button_style)  # 粉色保持不变
        self.allin_button.grid(row=0, column=4, padx=5)
        return

    def _add_sysinfo(self):
        # 系统信息区域
        sysinfo_frame = tk.Frame(self.window, bg='#f0f0f0')
        sysinfo_frame.pack(fill=tk.BOTH, padx=10, pady=5, expand=True)
        
        # 标题
        title_label = tk.Label(sysinfo_frame, text="系统信息", font=("Arial", 14, "bold"), 
                              bg='#f0f0f0')
        title_label.pack()
        
        # 使用Text控件显示多行信息
        sysinfo_text = tk.Label(sysinfo_frame, textvariable=self.sysInfo,
                               bg="#3498db", font=("Arial", 11, "bold"),  # 蓝色背景显示系统信息
                               width=80, height=4,
                               padx=10, pady=10, borderwidth=3, relief="ridge",
                               justify=tk.LEFT,
                               fg="white")
        sysinfo_text.pack(pady=5)

    def _update_player_info(self, player_id, action:int, bet:int, str:string):
        playerInfoFormat = "玩家: {name}\n筹码: {total_bet}\n操作: {action}\n下注: {pay_bet}"
        
        playerInfo = self.playerInfo[player_id]
        player_name = playerInfo.player.name
        player_strVar = playerInfo.strVar
        player_strVar.set(playerInfoFormat.format(name=player_name, 
                                                total_bet=playerInfo.player.total_bet,
                                                action=str, 
                                                pay_bet=playerInfo.player.pay_bet))

    def _fold_cb(self):
        self.controller.player.isFold = True
        self.controller.player.pay_bet = 0
        self._update_player_info(player_id=self.controller.player.id, 
                    action= PlayerAction.cPLAYER_FOLD, bet=0, str="弃牌")
        self._general_callback()

    def _check_cb(self):
        self.controller.player.pay_bet = 0 
        self._update_player_info(player_id=self.controller.player.id, 
                    action= PlayerAction.cPLAYER_CHECK, bet=0, str="过牌")
        self._general_callback()

    def _call_cb(self):
         # TODO : the request maybe None
        if self.controller.request:
            self.controller.player.pay_bet = self.controller.request.bet
            self._update_player_info(player_id=self.controller.player.id, 
                        action=PlayerAction.cPLAYER_CALL, bet=self.controller.request.bet, str="跟注")
        else:
            self.controller.player.pay_bet = 0
            self._update_player_info(player_id=self.controller.player.id, 
                        action=PlayerAction.cPLAYER_CALL, bet=0, str="跟注")
        self._general_callback()
    
    def _raise_cb(self):
        # 获取Raise金额
        try:
            amount_str = self.raise_amount.get()
            if not amount_str:
                messagebox.showwarning("输入错误", "请输入加注金额")
                return
                
            amount = int(amount_str)
            if amount <= 0:
                messagebox.showwarning("输入错误", "加注金额必须大于0")
                return
            if amount > self.controller.player.total_bet:
                messagebox.showwarning("筹码不足", "您的筹码不足以支付该金额")
                return
                
            self.controller.player.pay_bet = amount
            self._update_player_info(player_id=self.controller.player.id, 
                        action=PlayerAction.cPLAYER_RAISE, bet=amount, str="加注")
            self._general_callback()
        except ValueError:
            messagebox.showwarning("输入错误", "请输入有效的数字")
            return

    def _allin_cb(self):
        # 全押操作
        self.controller.player.pay_bet = self.controller.player.total_bet
        self.controller.player.isAllin = True
        self._update_player_info(player_id=self.controller.player.id, 
                    action=PlayerAction.cPLAYER_ALLIN, bet=self.controller.player.pay_bet, str="全押")
        self._general_callback()

if __name__ == '__main__':
    # 创建一个测试用的控制器模拟
    class MockController:
        def __init__(self):
            self.player = Player("测试玩家", 0, 1000)
            self.request = None
            
    gui = ClientView(MockController())
    gui.launch_window()