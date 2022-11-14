

class Player:
    def __init__(self, name:str):
        self.id = 0
        self.name = name

        self.pay_bet = 0 # 准备支付的本次bet
        self.total_bet = 500 # 还剩余的总bet
        self.isBlind = False # 是否是Blind 玩家
        pass