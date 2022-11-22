

class Player:
    def __init__(self, name:str, id:int, total_bet:int):
        self.id:int = id
        self.name:str = name

        self.pay_bet:int = 0           # 准备支付的本次bet
        self.total_bet:int = total_bet # 还剩余的总bet
        self.isBlind:bool = False # 是否是Blind 玩家
        self.isAllin:bool = False
        self.isFold :bool = False
        self.action:str = "Option"